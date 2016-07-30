GTKINC = `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
GTKLIB = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0`

CFLAGS = -Os ${GTKINC}
LDFLAGS = ${GTKLIB}

PREFIX = /usr/local

all:
	cc ${CFLAGS} -o notep notep.c ${LDFLAGS}

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f notep ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/notep
