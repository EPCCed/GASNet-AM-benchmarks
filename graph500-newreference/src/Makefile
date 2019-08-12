include $(GASNET_INSTALL_DIR)/include/$(CONDUIT)-conduit/$(CONDUIT)-par.mak

CFLAGS 	= -g -Drestrict=__restrict__ -O3 -DGRAPH_GENERATOR_MPI -DREUSE_CSR_FOR_VALIDATION -I../aml
LDFLAGS	= -lpthread
LIBS	= -lm

ifndef MPICC
$(error MPICC not defined)
endif

ifndef TARGET
$(error TARGET not defined. Please choose either 'mpi' or 'gasnet')
endif

ifeq ($(TARGET), gasnet)
CFLAGS	+= $(GASNET_CPPFLAGS) $(GASNET_CFLAGS)
LDFLAGS	+= $(GASNET_LDFLAGS)
LIBS	+= $(GASNET_LIBS)
endif

all: graph500_reference_bfs_sssp graph500_reference_bfs 
#graph500_custom_bfs graph500_custom_bfs_sssp

GENERATOR_SOURCES = ../generator/graph_generator.c ../generator/make_graph.c ../generator/splittable_mrg.c ../generator/utils.c
SOURCES = main.c utils.c validate.c ../aml/aml_$(TARGET).c
HEADERS = common.h csr_reference.h bitmap_reference.h

graph500_reference_bfs_sssp: bfs_reference.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES) csr_reference.c sssp_reference.c
	$(MPICC) $(CFLAGS) -DSSSP $(LDFLAGS) -O1 -o $(TARGET)/graph500_reference_bfs_sssp bfs_reference.c sssp_reference.c csr_reference.c $(SOURCES) $(GENERATOR_SOURCES) $(LIBS) 

graph500_reference_bfs: bfs_reference.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES) csr_reference.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) -O1 -o $(TARGET)/graph500_reference_bfs bfs_reference.c csr_reference.c $(SOURCES) $(GENERATOR_SOURCES) $(LIBS)

graph500_custom_bfs: bfs_custom.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES) csr_reference.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) -O1 -o $(TARGET)/graph500_custom_bfs bfs_custom.c csr_reference.c $(SOURCES) $(GENERATOR_SOURCES) $(LIBS)

graph500_custom_bfs_sssp: bfs_custom.c sssp_custom.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES) csr_reference.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) -O1 -o $(TARGET)/graph500_custom_bfs_sssp bfs_custom.c sssp_custom.c csr_reference.c $(SOURCES) $(GENERATOR_SOURCES) $(LIBS)

clean:
	-rm -f $(TARGET)/graph500_*
