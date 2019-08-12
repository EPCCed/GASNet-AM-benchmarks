ifndef GASNET_INSTALL_DIR
$(error please define GASNET_INSTALL_DIR)
endif

ifndef CONDUIT
$(error please define CONDUIT)
endif

ifndef MPICC
$(error please define MPICC)
endif

all: gasnet mpi

gasnet:
	cd gasnet && $(MAKE) CONDUIT=$(CONDUIT) GASNET_INSTALL_DIR=$(GASNET_INSTALL_DIR)
	
mpi:
	cd mpi    && $(MAKE) MPICC=$(MPICC)

clean:
	cd gasnet && $(MAKE) clean
	cd mpi    && $(MAKE) clean
	cd util   && $(MAKE) clean

.PHONY: all gasnet mpi clean
