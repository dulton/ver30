# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= simulator

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst modules, ,$(shell pwd)))

# Set if it will show the compile command line
GMI_MAKE_VERBOSE       ?= @

# Export these variables
export GMI_PLATFORM
export GMI_TOPDIR
export GMI_MAKE_VERBOSE

# Sub item list that need to be compiled
GMI_ITEMS               = source unitest

# Include item_rules.mk
include $(GMI_TOPDIR)/build/item_rules.mk

