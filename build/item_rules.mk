# Include common.mk
include $(GMI_TOPDIR)/build/common.mk

all:

%:
	$(GMI_MAKE_VERBOSE) \
	for item in $(GMI_ITEMS); do \
	  if [ -d $$item ]; then \
	    $(GMI_MAKE) -C $$item $@; \
	  fi; \
	  if [ -f $$item ]; then \
	    $(GMI_MAKE) -f  $$item $@; \
	fi; \
	done

.PHONEY: all clean install

