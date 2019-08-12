ifndef MPICC
$(error please define MPICC)
endif

all: osu_bw osu_latency

osu_bw: osu_bw.o osu_util_mpi.o osu_util.o
	$(MPICC) osu_bw.o ../util/osu_util_mpi.o ../util/osu_util.o -o osu_bw-mpi.out

osu_latency: osu_latency.o osu_util_mpi.o osu_util.o
	$(MPICC) osu_latency.o ../util/osu_util_mpi.o ../util/osu_util.o -o osu_latency-mpi.out
	
osu_bw.o: osu_bw.c
	$(MPICC) -I../util/ -c osu_bw.c -o osu_bw.o
	
osu_latency.o: osu_latency.c
	$(MPICC) -I../util/ -c osu_latency.c -o osu_latency.o

osu_util_mpi.o: 
	cd ../util && $(MAKE) osu_util_mpi.o MPICC="$(MPICC)"
	
osu_util.o:
	cd ../util && $(MAKE) osu_util.o CC="$(MPICC)"

clean:
	rm -f *.o
	rm -f *.out
	
.PHONY: clean all osu_bw osu_latency
