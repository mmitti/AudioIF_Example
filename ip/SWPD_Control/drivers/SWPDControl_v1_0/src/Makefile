COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

INCLUDEFILES=SWPDPeripheral.h
LIBSOURCES=$(wildcard *.cpp)
OUT = SWPD.o

libs:
	echo "Compiling SWPD..."
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(LIBSOURCES) -o ${OUT}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OUT}
	make clean

include:
	${CP} $(INCLUDEFILES) $(INCLUDEDIR)

clean:
	rm -rf ${OUT}
