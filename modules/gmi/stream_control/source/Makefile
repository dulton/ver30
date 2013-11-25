# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= simulator

# Set name of this module
GMI_MODULE_NAME         = stream_control

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst modules/gmi, ,$(shell pwd)))

# Set if it will show the compile command line
GMI_MAKE_VERBOSE       ?= @

# Export these variables
export GMI_PLATFORM
export GMI_TOPDIR
export GMI_MAKE_VERBOSE

# Include module.mk
include $(GMI_TOPDIR)/build/module.mk

# Set type of this module, value is in ['executable', 'shared_lib', 'static_lib']
GMI_MODULE_TYPE         = static_lib

# Set the target name of this module
GMI_MODULE_TARGET_NAME  = $(GMI_MODULE_NAME)

# Set the source code files to compile
GMI_MODULE_SRCS         = stream_control.cpp

# Set compile flags
GMI_CFLAGS              = -Wall 
#GMI_CFLAGS             = -Wall -g -pg

# Set link flags
GMI_LFLAGS              =
#GMI_LFLAGS             = -g -pg

# Set related modules
GMI_RELATED_MODULES     = xml2 config

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../include

# Set other header file path
GMI_INCLUDE_DIR         = ../include ${GMI_TOPDIR}/prebuilt/include/osal/ ${GMI_TOPDIR}/application/

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk


