TARGET = embedqjs
LIB_QUICKJS = quickjs/libquickjs.a
CC = gcc
AR = ar
SANITIZE_FLAGS = -fsanitize=undefined                           \
  -fsanitize=pointer-overflow -fsanitize=float-divide-by-zero   \
  -fsanitize=alignment -fsanitize=bounds-strict                 \
  -fsanitize=signed-integer-overflow -fsanitize=leak            \
  -fsanitize=address -fsanitize=integer-divide-by-zero          \
  -fsanitize=null
CFLAGS = -std=c99 -ggdb3 -Wall -fstack-protector-strong          \
	-D_POSIX_C_SOURCE=200809L -DSUPPORT_FILEFORMAT_BMP            \
	$(SANITIZE_FLAGS)
LDFLAGS = $(SANITIZE_FLAGS) -Lquickjs
LIBS = -lquickjs -lm

.PHONY: default all clean

default: $(TARGET)
all: default
re: clean all

HEADERS = $(wildcard *.h)

SRCS = main.c
OBJS = $(patsubst %.c,%.o,$(SRCS))

%: %.c
	$(CC) $(CFLAGS)  -o $@ $<

$(OBJS): quickjs

$(TARGET): $(LIB_QUICKJS) $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

quickjs:
	git clone https://github.com/bellard/quickjs

$(LIB_QUICKJS): quickjs
	-(cd quickjs && $(MAKE))

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -fr quickjs
