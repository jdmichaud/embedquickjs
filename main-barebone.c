#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "quickjs/quickjs.h"

#define PROMPT "> "
#define MAX_BUF_SIZE ((1 << 16) - 1)

int jsnprintf(char *buf, size_t size, JSContext *context, JSValue value) {
  size_t ret = 0;
  if (JS_IsBool(value)) {
    int b = JS_ToBool(context, value);
    ret = snprintf(buf, size, "%s", b == 1 ? "true" : "false");
  } else if (JS_IsNumber(value)) {
    int64_t i;
    JS_ToInt64(context, &i, value);
    ret = snprintf(buf, size, "%li", i);
  } else if (JS_IsUndefined(value) || JS_IsUninitialized(value)) {
    ret = snprintf(buf, size, "undefined");
  } else if (JS_IsNull(value)) {
    ret = snprintf(buf, size, "null");
  } else if (JS_IsNumber(value)) {
    int64_t i;
    JS_ToInt64(context, &i, value);
    ret = snprintf(buf, size, "%li", i);
  } else if (JS_IsString(value)) {
    const char *s = JS_ToCString(context, value);
    ret = snprintf(buf, size, "%s", s);
  } else if (JS_IsException(value)) {
    JSValue exception = JS_GetException(context);
    JSValue name = JS_GetPropertyStr(context, exception, "name");
    if (!JS_IsUndefined(name)) {
      const char *s = JS_ToCString(context, name);
      ret = snprintf(buf, size, "[exception] %s: ", s);
    }
    JSValue message = JS_GetPropertyStr(context, exception, "message");
    if (!JS_IsUndefined(message)) {
      const char *s = JS_ToCString(context, message);
      ret += snprintf(&buf[ret], size - ret, "%s", s);
    }
    JS_FreeValue(context, message);
    JS_FreeValue(context, name);
    JS_FreeValue(context, exception);
  } else if (JS_IsArray(context, value)) {

  // } else if (JS_IsObject(value)) {
  //   JSPropertyEnum *tab;
  //   uint32_t len;
  //   int res = JS_GetOwnPropertyNames(context, &tab, &len, value, 0);
  //   if (res != 0) return res;
  //   for (int i = 0; i < len; ++i) {
  //     if (tab[i].is_enumerable) {
  //       JSPropertyDescriptor desc;
  //       res = JS_GetOwnProperty(context, &desc, value, tab[i].atom);
  //       if (res != 0) return res;
  //     }
  //     JS_FreeAtom(context, tab[i].atom);
  //   }
  //   js_free(context, tab);
  //   ret = 0; // TODO
  } else {
    return -1;
  }

  return ret;
}

int add(int lhs, int rhs) {
  return lhs + rhs;
}

// Wrapper function for add
JSValue j_add(JSContext *context, JSValueConst this_val, int argc, JSValueConst *argv) {
  int64_t lhs;
  JS_ToInt64(context, &lhs, argv[0]);
  int64_t rhs;
  JS_ToInt64(context, &rhs, argv[1]);
  return JS_NewInt64(context, add(lhs, rhs));
}

int main(int argc, char **argv) {
  JSRuntime *runtime = JS_NewRuntime();
  JSContext *context = JS_NewContext(runtime);

  // Let's plug add to the global object
  JSValue global_obj = JS_GetGlobalObject(context);
  JS_SetPropertyStr(context, global_obj, "add",
                    JS_NewCFunction(context, j_add, "add", 0));

  printf("ctrl-D to quit\n");
  char *input = NULL;
  while (1) {
    printf("%s", PROMPT);
    scanf("%m[^\n]", &input);
    if (input == NULL) { // EOF
      break;
    }
    getchar(); // consume the new line character
    JSValue value = JS_Eval(context, input, strnlen(input, MAX_BUF_SIZE), "<eval>", 0);
    free(input);
    char result[MAX_BUF_SIZE];
    if (jsnprintf(result, MAX_BUF_SIZE, context, value) > 0) {
      printf("%s\n", result);
    } else {
      printf("Unknown result type: %li\n", value.tag);
    }
    JS_FreeValue(context, value);
  }

  JS_FreeValue(context, global_obj);
  JS_FreeContext(context);
  JS_FreeRuntime(runtime);
  printf("OK\n");
  return 0;
}
