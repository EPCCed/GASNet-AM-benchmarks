include $(GASNET_INSTALL_DIR)/include/$(CONDUIT)-conduit/$(CONDUIT)-seq.mak

all: mpi gasnet

mpi:
	$(MPICC) -g -w test.c aml_mpi.c -o test_mpi.out
	
gasnet:
	$(MPICC) -g $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(GASNET_LDFLAGS) test.c aml_gasnet.c $(GASNET_LIBS) -o test_gasnet.out

clean:
	rm -f *.out

.PHONY: mpi gasnet clean
