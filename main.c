#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"

extern const uint8_t qjsc_repl[];
extern const uint32_t qjsc_repl_size;

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
  js_std_init_handlers(runtime);

  JSContext *context = JS_NewContext(runtime);
  js_init_module_std(context, "std");
  js_init_module_os(context, "os");

  JS_SetModuleLoaderFunc(runtime, NULL, js_module_loader, NULL);

  // Let's plug add to the global object
  JSValue global_obj = JS_GetGlobalObject(context);
  JS_SetPropertyStr(context, global_obj, "add",
                    JS_NewCFunction(context, j_add, "add", 0));

  char *_argv[0];
  js_std_add_helpers(context, 0, _argv);
  js_std_eval_binary(context, qjsc_repl, qjsc_repl_size, 0);
  js_std_loop(context);

  JS_FreeValue(context, global_obj);
  js_std_free_handlers(runtime);
  JS_FreeContext(context);
  JS_FreeRuntime(runtime);
  return 0;
}
