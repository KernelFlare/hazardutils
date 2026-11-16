.POSIX:

CC      = cc
CFLAGS  = -O2 -g -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin

TARGET  = rservd
SRC     = rservd.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)

clean:
	rm -f $(TARGET)

.PHONY: all install clean
