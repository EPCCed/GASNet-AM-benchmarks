ifndef CC
CC=$(MPICC)
endif

all: osu_util.o osu_util_mpi.o osu_util_gasnet.o

osu_util.o:
	$(CC) -c osu_util.c -o osu_util.o

osu_util_gasnet.o:
	$(CC) -c osu_util_gasnet.c -o osu_util_gasnet.o

osu_util_mpi.o:
	$(MPICC) -c osu_util_mpi.c -o osu_util_mpi.o

clean:
	rm -f *.o

.PHONY: clean all
