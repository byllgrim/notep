# See LICENSE file for copyright and license details.

GTKINC = `pkg-config --cflags gtk+-3.0`
GTKLIB = `pkg-config --libs gtk+-3.0`

CFLAGS = -Os -Wall -Wextra -std=c99 -pedantic -lpthread ${GTKINC}
LDFLAGS = ${GTKLIB}

PREFIX = /usr/local

compile: config.h
	@echo CC = ${CC}
	@echo CFLAGS = ${CFLAGS}
	@echo LDFLAGS = ${LDFLAGS}
	${CC} ${CFLAGS} -o notep notep.c ${LDFLAGS}

config.h:
	cp config.def.h config.h

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f notep ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/notep

format:
	clang-format -i --style="file:./style.clang-format" ./notep.c

clean:
	rm -f notep
