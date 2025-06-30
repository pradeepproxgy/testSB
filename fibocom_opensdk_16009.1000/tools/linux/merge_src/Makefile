#CFLAGS		+=	-O3 -g
CFLAGS		+=	-g 
#-lbz2 -L. -I /home/jinyf/0517/ZMPSDK7520V3V1.0.0B04P01_AP/linux_plat/base/linux/lib/libbzip2/ 

PREFIX		?=	/usr/local
INSTALL_PROGRAM	?=	${INSTALL} -c -s -m 555
INSTALL_MAN	?=	${INSTALL} -c -m 444

all:		clean bsdiff bspatch merge
bsdiff:		bsdiff.c bzlib.c crctable.c compress.c decompress.c randtable.c blocksort.c huffman.c
bspatch:	bspatch.c bzlib.c crctable.c compress.c decompress.c randtable.c blocksort.c huffman.c
merge:		merge.c

install:
	${INSTALL_PROGRAM} bsdiff bspatch ${PREFIX}/bin

clean:
	rm -rf bsdiff bspatch merge
