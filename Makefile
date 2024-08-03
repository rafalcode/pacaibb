# taking bits out of pacariodems to get the building blocks of the code.
EXECUTABLES=htblkz htblk jpgo imred0 imclip jpred0 jpred1 tblockr pabb cairosimple mycaisimp tb2 jpred2 xif0 jpred3 jpred4 convda

CC=gcc
CFLAGS=-g -Wall
DBGCFLAGS=-g -Wall -DDBG

PANGINCS=-I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -pthread -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1
PANGLIBS=-lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lm -lcairo
PANGLIBS2=-lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lm -lcairo -lcairo_jpg -ljpeg

# this version pretty much like the pacairodems0 one.
# # problem is it's rather a bit too complicated.
htblkz: htblkz.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

# here I stat varying some things.
htblkz2: htblkz2.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
# tblockr.c: I take htblkz2.c and improve it.
pabb: pabb.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
tblockr: tblockr.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
# a simpler tblockr so I can fit it in better with jpred1.c
tb2: tb2.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}


# setting indent just not working. There is a workaround, add a html symbol of some sort and space
# get cairosimple from imagemagick (sic) repo
# https://raw.githubusercontent.com/ImageMagick/pango/main/examples/cairosimple.c
cairosimple: cairosimple.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
# I've left the above intact, the following are my mods.
mycaisimp: mycaisimp.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

# imred0 ... image reduce, but using only cairo. From cl3a.c in ciarobegs repo.
imclip: imclip.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
jpgo: jpgo.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2}
imred0: imred0.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}
jpred0: jpred0.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2}
jpred1: jpred1.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2}
# picks the centre of the image and does a strip at bottom
jpred2: jpred2.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2}
# so jpred2.c is pretty good now,
jpred3: jpred3.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2}
# now we allow a focus point that is not the middle and requires cc.py to be run
jpred4: jpred4.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2} -lexif
xif0: xif0.c
	${CC} ${CFLAGS} -I/usr/include/x86_64-linux-gnu -o $@ $^ -lexif
	# ${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS2} -lexif

imred00: imred00.c
	${CC} ${CFLAGS} ${PANGINCS} -o $@ $^ ${PANGLIBS}

convda: convda.c
	${CC} ${CFLAGS} -o $@ $^

.PHONY: clean

clean:
	rm -f ${EXECUTABLES}
