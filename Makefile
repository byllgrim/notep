# See LICENSE file for copyright and license details.
GTKINC = `pkg-config --cflags gtk+-3.0`
GTKLIB = `pkg-config --libs gtk+-3.0`

CFLAGS = -Os -Wall -Wextra ${GTKINC}
LDFLAGS = ${GTKLIB}

PREFIX = /usr/local

#TODO fix this makefile
all: config.h
	cc ${CFLAGS} -o notep notep.c ${LDFLAGS}

config.h:
	cp config.def.h config.h

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f notep ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/notep

clean:
	rm -f notep
