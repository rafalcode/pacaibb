# taking bits out of pacariodems to get the building blocks of the code.
EXECUTABLES=htblkz htblk

CC=gcc
CFLAGS=-g -Wall
DBGCFLAGS=-g -Wall -DDBG

PANGINCS=-I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -pthread -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1
PANGLIBS=-lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lm -lcairo

# this version pretty much like the pacairodems0 one.
# # problem is it's rather a bit too complicated.
htblkz: htblkz.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

# here I stat varying some things.
htblkz2: htblkz2.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

# imred0 ... image reduce, but using only cairo. From cl3a.c in ciarobegs repo.
imclip: imclip.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
imred0: imred0.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

imred00: imred00.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

.PHONY: clean

clean:
	rm -f ${EXECUTABLES}
