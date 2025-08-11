CC := gcc

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif

CFLAGS := $(CFLAGS) -Wl,-E -Wall -Werror -fPIC

INCLUDE :=
LIBS := -llua -lmysqlclient -lm -ldl

ifdef LUADIR
	INCLUDE := -I$(LUADIR)/src $(INCLUDE)
	LIBS := -L$(LUADIR)/src $(LIBS)
endif

TARGET := luamysql luamysql.so

.PHONY: all clean

all: $(TARGET)

luamysql: luamysql.o host.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

luamysql.so: luamysql.o
	$(CC) $(CFLAGS) $^ -shared -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
