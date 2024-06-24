# taking bits out of pacariodems to get the building blocks of the code.
EXECUTABLES=htblkz htblk

CC=gcc
CFLAGS=-g -Wall
DBGCFLAGS=-g -Wall -DDBG

PANGINCS=-I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -pthread -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1
PANGLIBS=-lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lm -lcairo

# this version pretty much like the pacairodems0 one.
htblkz: htblkz.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

# here I stat varying some things.
htblkz2: htblkz2.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

.PHONY: clean

clean:
	rm -f ${EXECUTABLES}
