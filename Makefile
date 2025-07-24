CC := gcc

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif

CFLAGS := $(CFLAGS) -Wl,-E -Wall -Werror -fPIC

ifndef LUADIR
    $(error environment variable 'LUADIR' is not set)
endif

INCLUDE := -I$(LUADIR)/src
LIBS := -L$(LUADIR)/src -llua -lmysqlclient -lm -ldl

TARGET := luamysql luamysql.so

.PHONY: all clean deps

all: deps $(TARGET)

luamysql: luamysql.o host.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

luamysql.so: luamysql.o
	$(CC) $(CFLAGS) $^ -shared -o $@ $(LIBS)

deps:
	$(MAKE) posix MYCFLAGS="-fPIC -DLUA_USE_DLOPEN -Wl,-E" MYLIBS="-ldl" -C $(LUADIR)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
