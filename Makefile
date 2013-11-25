# Set top dir of the compile system
GMI_TOPDIR             ?= $(shell pwd)

# Set if it will show the compile command line
GMI_MAKE_VERBOSE       ?= @

# Export these variables
export GMI_TOPDIR
export GMI_MAKE_VERBOSE

# Sub item list that need to be compiled
GMI_ITEMS               = modules application

# Set release package
GMI_RELEASE_PACKAGE     = $(GMI_TOPDIR)/release.tar.bz2

# Include common.mk
include $(GMI_TOPDIR)/build/common.mk

# Check if GMI_PLATFORM is empty
ifneq ($(GMI_PLATFORM),)
# Include platform.mk
include $(GMI_TOPDIR)/build/platform.mk

# Export variable GMI_PLATFORM
export GMI_PLATFORM
endif

# Get platform list
GMI_PLATFORM_LIST       = simulator ambarella_a5s_sdk_v3.3

# Check if GMI_PLATFORM is empty
ifeq ($(GMI_PLATFORM),)

# Compile the source code in all platform
all:
	$(GMI_MAKE_VERBOSE) \
	for item in $(GMI_PLATFORM_LIST); do \
	  $(GMI_MAKE) $$item; \
	done

# Make release package
release: all
	$(GMI_MAKE_VERBOSE)echo "==================== Pack Release Package ===================="
	$(GMI_MAKE_VERBOSE)echo "Packing $(notdir $(GMI_RELEASE_PACKAGE)) ..."
	$(GMI_MAKE_VERBOSE)cd $(GMI_INSTALL_DIR) && tar cjf $(GMI_RELEASE_PACKAGE) *

# Clean all files that created while compiling on all platform
clean:
	$(GMI_MAKE_VERBOSE)$(GMI_RM) $(GMI_OUTPUT_DIR) $(GMI_INSTALL_DIR) $(GMI_RELEASE_PACKAGE)

.PHONY: release
else

all: $(GMI_SUBDIRS)

# Compile and install all modules
$(GMI_SUBDIRS):
	$(GMI_MAKE_VERBOSE)$(GMI_MAKE) -C $@
	$(GMI_MAKE_VERBOSE)$(GMI_MAKE) -C $@ install

# Clean all files that created while compiling
clean:
	$(GMI_MAKE_VERBOSE)$(GMI_RM) $(GMI_OUTPUT_DIR) $(GMI_INSTALL_DIR)

.PHONY: $(GMI_SUBDIRS)
endif

$(GMI_PLATFORM_LIST):
	$(GMI_MAKE_VERBOSE)echo "==================== Platform '$@' ===================="
	$(GMI_MAKE_VERBOSE)$(GMI_MAKE) GMI_PLATFORM=$@

.PHONY: all clean $(GMI_PLATFORM_LIST)
