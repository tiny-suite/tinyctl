CC = gcc
BIN = tinyctl
SRC = tinyctl.c registry.c run.c commands.c tui.c
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

ROOT = $(abspath $(CURDIR)/..)
CFLAGS ?= -O2
CFLAGS += -std=c11 -Wall -Wextra -D_DEFAULT_SOURCE -DTINYSUITE_ROOT=\"$(ROOT)\"

all: $(BIN)

$(BIN): $(SRC) tinyctl.h
	$(CC) $(CFLAGS) -o $@ $(SRC)

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all install uninstall clean
