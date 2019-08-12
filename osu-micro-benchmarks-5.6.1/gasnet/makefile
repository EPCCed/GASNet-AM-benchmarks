ifndef GASNET_INSTALL_DIR
$(error please define GASNET_INSTALL_DIR)
endif

ifndef CONDUIT
$(error please define CONDUIT)
endif

include $(GASNET_INSTALL_DIR)/include/$(CONDUIT)-conduit/$(CONDUIT)-par.mak

all: bw latency

bw: gasnet_osu_bw.o osu_util_gasnet.o osu_util.o
	$(GASNET_LD) $(GASNET_LDFLAGS) gasnet_osu_bw.o ../util/osu_util_gasnet.o ../util/osu_util.o $(GASNET_LIBS) -o osu_bw-gasnet.out

latency: gasnet_osu_latency.o osu_util_gasnet.o osu_util.o
	$(GASNET_LD) $(GASNET_LDFLAGS) gasnet_osu_latency.o ../util/osu_util_gasnet.o ../util/osu_util.o $(GASNET_LIBS) -o osu_latency-gasnet.out
	
gasnet_osu_bw.o: gasnet_osu_bw.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) -I../util $(GASNET_CFLAGS) -c gasnet_osu_bw.c -o gasnet_osu_bw.o
	
gasnet_osu_latency.o: gasnet_osu_latency.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) -I../util $(GASNET_CFLAGS) -c gasnet_osu_latency.c -o gasnet_osu_latency.o #-DBLOCKING
	
osu_util_gasnet.o:
	cd ../util && $(MAKE) osu_util_gasnet.o CC="$(GASNET_CC)"
	
osu_util.o:
	cd ../util && $(MAKE) osu_util.o CC="$(GASNET_CC)"
	
clean:
	rm -f *.o
	rm -f *.out
	
.PHONY: all bw latency clean
