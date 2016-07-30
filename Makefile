GTKINC = `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
GTKLIB = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0`

CFLAGS = -Os ${GTKINC}
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
