APPLICATION = MiyakoLink
export APPLICATION

PLATFORM_LIST = Julis Izana

.PHONY: all clean $(PLATFORM_LIST) $(addsuffix _clean, $(PLATFORM_LIST))

all:
	@if [ ! -e RIOT/Makefile ]; then \
	echo "Init git submodules"; \
		git submodule init ;\
		git submodule update ;\
	fi
	@for platform in $(PLATFORM_LIST); do \
		$(MAKE) $(MFLAGS) -C src BOARD=$$platform ;\
	done
	$(MAKE) $(MFLAGS) -C src

clean: $(addsuffix _clean, $(PLATFORM_LIST))
	
Julis:
	$(MAKE) $(MFLAGS) -C src BOARD=$@

Julis_clean:
	
Izana:
	$(MAKE) $(MFLAGS) -C src/platforms/Izana/aux-firmware

Izana_clean:
	$(MAKE) $(MFLAGS) -C src/platforms/Izana/aux-firmware clean
