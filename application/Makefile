# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= simulator

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst application, ,$(shell pwd)))

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
GMI_ITEMS                = common
GMI_ITEMS               += daemon_server
GMI_ITEMS               += log_server
GMI_ITEMS               += media_center_server
GMI_ITEMS               += auth_server
GMI_ITEMS               += onvif
GMI_ITEMS               += sdk_server
GMI_ITEMS               += rtsp_server
GMI_ITEMS               += sys_server
GMI_ITEMS		+= web_cgi
GMI_ITEMS               += config_tool
GMI_ITEMS		+= system_resource_detect
# Include item_rules.mk
include $(GMI_TOPDIR)/build/item_rules.mk

