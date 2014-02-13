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
# To facilitate reading and compiling system can work, please keep module compiling sequence, principle as follows:
# 1 module dependency
# 2 alplabet sequence
GMI_ITEMS               = amba_rtsp
GMI_ITEMS              += media_center
GMI_ITEMS              += auth_center
GMI_ITEMS              += stream_control
GMI_ITEMS              += event_center
GMI_ITEMS              += sys_info_readonly

# Include item_rules.mk
include $(GMI_TOPDIR)/build/item_rules.mk

