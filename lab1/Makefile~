CC = mpicc.mpich2
CFLAGS = 

LFLAGS= -lpthread -lrt -lm

all: mpiblurc thresc

clean:
	-$(RM) *.o mpiblur thresc

mpiblurc: ppmio.o gaussw.o blurfilter.o mpiblurmain.o
	$(CC) -o $@ ppmio.o gaussw.o blurfilter.o mpiblurmain.o $(LFLAGS)

thresc: thresmain.o ppmio.o thresfilter.o
	$(CC) -o $@ thresmain.o ppmio.o thresfilter.o $(LFLAGS)

arc:
	tar cf - *.c *.h *.f90 Makefile Makefile.sgi|gzip - > filters.tar.gz
