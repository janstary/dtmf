# (c) 2018 Jan Stary <hans@stare.cz>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

VERSION = 1.0
TARBALL = dtmf-$(VERSION).tar.gz

PREFIX	= $(HOME)
BINDIR	= $(PREFIX)/bin/
MANDIR	= $(PREFIX)/man/

CC	= cc
CFLAGS	= -Wall -pedantic

BINS =	dtmf
MAN1 =	dtmf.1
SRCS =	dtmf.c

DISTFILES = LICENSE Makefile $(MAN1) $(SRCS)	

all: $(BINS) $(MAN1)

dtmf: dtmf.c
	$(CC) $(CFLAGS) -o dtmf dtmf.c -lm

.PHONY: clean install uninstall lint test

clean:
	rm -f $(TARBALL) $(BINS) $(OBJS)
	rm -rf *.dSYM *.core *~ .*~
	rm -rf dtmf-$(VERSION)

install: all
	install -d $(BINDIR)      && install -m 0755 $(BINS) $(BINDIR)
	install -d $(MANDIR)/man1 && install -m 0644 $(MAN1) $(MANDIR)/man1

uninstall:
	cd $(BINDIR)      && rm $(BINS)
	cd $(MANDIR)/man1 && rm $(MAN1)

lint: $(MAN1)
	mandoc -Tlint -Wstyle $(MAN1)

SIN=sin 697 sin 1209
SYM=123A456B789C\*0\#D
FMT=-r 8000 -b 8 -e unsigned

test: $(BINS)
	echo $(SYM) | ./dtmf | ./dtmf -d
	echo $(SYM) | ./dtmf | play -c 1 $(FMT) -t raw -
	sox $(FMT) -n $(FMT) -t raw - synth 1 $(SIN) remix - gain -3 | ./dtmf -d

dist: $(TARBALL)

$(TARBALL): $(DISTFILES)
	rm -rf .dist
	mkdir -p .dist/dtmf-$(VERSION)/
	install -m 0644 $(DISTFILES) .dist/dtmf-$(VERSION)/
	( cd .dist && tar czf ../$@ dtmf-$(VERSION) )
	rm -rf .dist/

distcheck: dist
	rm -rf dtmf-$(VERSION) && tar xzf $(TARBALL)
	( cd dtmf-$(VERSION) && make )

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<
