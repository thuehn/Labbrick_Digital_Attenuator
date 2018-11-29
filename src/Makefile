.EXPORT_ALL_VARIABLES:

DESTDIR=
PREFIX=/usr/local
MANDIR=/usr/share/man/man7/
CFLAGS=-g -O2 -Wall -Wno-unused-variable
LDFLAGS=-lm -lpthread -lusb -lrt

CC=gcc
LD=gcc

RM=rm -f

DEPS=LDAhid.h

INSTALL_DIR=install -m 0755 -d
INSTALL_BIN=install -m 0755

ZIP= gzip

attenuator: LDAhid.o control.o input.o
	$(LD) $(LDFLAGS) -o attenuator_lab_brick LDAhid.o control.o input.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o '$@' '$<'

.PHONY: attenuator
attenuator_lab_brick: attenuator_lab_brick

.PHONY: all
all: attenuator

.PHONY: clean
clean:
	$(RM) -- input.o control.o LDAhid.o attenuator_lab_brick

.PHONY: install
install:
	$(INSTALL_BIN) -- attenuator_lab_brick.7 $(MANDIR)
	$(INSTALL_DIR) -- '$(DEST_DIR)$(PREFIX)/bin'
	$(INSTALL_BIN) -- attenuator_lab_brick '$(DESTDIR)$(PREFIX)/bin/'
	$(ZIP) $(MANDIR)attenuator_lab_brick.7

.PHONY: uninstall
uninstall:
	$(RM) -- '$(DESTDIR)$(PREFIX)/bin/attenuator_lab_brick'
	$(RM) -- '$(MANDIR)attenuator_lab_brick.7.gz'

