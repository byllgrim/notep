# See LICENSE file for copyright and license details.


GTK_INC     = `pkg-config --cflags-only-I gtk+-3.0`
GTK_CFLAGS  = ${GTK_INC} `pkg-config --cflags-only-other gtk+-3.0`
GTK_LDFLAGS = `pkg-config --libs gtk+-3.0`

OPT = -Os
#OPT = -g -O0
CFLAGS = ${OPT} -Wall -Wextra -std=c99 -pedantic -lpthread ${GTK_CFLAGS}
LDFLAGS = ${GTK_LDFLAGS}

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

lint:
	clang-tidy ./notep.c --fix --config-file='./lint.clang-tidy' -- ${GTK_INC}

leaks:
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./notep

clean:
	rm -f notep
