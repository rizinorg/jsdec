CFLAGS+=-g -O3 -std=c99 -Wall -fPIC -I./duktape
CFLAGS_PKG=$(shell pkg-config --cflags rz_util rz_io rz_cons rz_core)

LDFLAGS+=-lm
LDFLAGS_PKG=$(shell pkg-config --libs rz_util rz_io rz_cons rz_core)

LIBEXT=$(shell rizin -H LIBEXT)
PLUGDIR=$(shell rizin -H RZ_USER_PLUGINS)

V=@
ECHO=echo
RM=rm -f
CP=cp -f
MKDIR=mkdir -p

SRCS=duktape/duktape.c duktape/duk_console.c core_pdd.c
OBJS=$(SRCS:.c=.o)
BIN=core_pdd.$(LIBEXT)

all: clean build install

build: $(BIN)

$(BIN): $(OBJS)
	$(V)$(ECHO) "[CC] $@"
	$(V)$(CC) $(LDFLAGS) $(LDFLAGS_PKG) -shared $^ -o $@

%.o: %.c
	$(V)$(ECHO) "[CC] $@"
	$(V)$(CC) $(CFLAGS) $(CFLAGS_PKG) -c $< -o $@

$(PLUGDIR):
	$(V)$(MKDIR) $@

install: uninstall $(PLUGDIR) $(BIN)
	$(V)$(ECHO) "[INSTALL]" $(PLUGDIR)/$(BIN)
	$(V)$(RM) $(PLUGDIR)/$(BIN)
	$(V)$(CP) $(BIN) $(PLUGDIR)

uninstall:
	$(V)$(RM) $(PLUGDIR)/$(BIN)

clean:
	$(V)$(RM) $(BIN) $(OBJS) || sleep 0

testbin:
	$(V)$(CC) $(CFLAGS) -DUSE_RZCONS=0 -o jsdec-test duktape/duktape.c duktape/duk_console.c jsdec-test.c $(LDFLAGS)

.PHONY: clean all install uninstall testbin build
