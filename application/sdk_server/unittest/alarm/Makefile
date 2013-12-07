# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= ambarella_a5s_sdk_v3.3

# Set name of this module
GMI_MODULE_NAME         = sdkalarm

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst application/sdk_server, ,$(shell pwd)))

# Set if it will show the compile command line
GMI_MAKE_VERBOSE       ?= @

# Export these variables
export GMI_PLATFORM
export GMI_TOPDIR
export GMI_MAKE_VERBOSE

# Include module.mk
include $(GMI_TOPDIR)/build/module.mk

# Set type of this module, value is in ['executable', 'shared_lib', 'static_lib']
GMI_MODULE_TYPE         = executable

# Set the target name of this module
GMI_MODULE_TARGET_NAME  = $(GMI_MODULE_NAME)_unitest

# Set the source code files to compile
GMI_MODULE_SRCS         = alarm.cpp

# Set compile flags
#GMI_CFLAGS              =  -Wall -O3
GMI_CFLAGS             = -DSDK_UNITTEST -Wall

# Set link flags
#GMI_LFLAGS              =
GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = rudp rt

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR=
GMI_MODULE_INCLUDE_DIR= 

# Set other header file pathm
GMI_INCLUDE_DIR         = $(GMI_TOPDIR)/application/sdk_server/include/ $(GMI_TOPDIR)/prebuilt/include/  $(GMI_TOPDIR)/prebuilt/include/osal/ $(GMI_TOPDIR)/application/
GMI_INCLUDE_DIR         += $(GMI_TOPDIR)/prebuilt/include/command_pipeline/ $(GMI_TOPDIR)/application/sdk_server/source/


# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk


