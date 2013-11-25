# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= simulator

# Set name of this module
GMI_MODULE_NAME         = $(notdir $(shell cd .. && pwd))

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst application, ,$(shell pwd)))

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
GMI_MODULE_SRCS         = $(wildcard *.cpp)
GMI_MODULE_SRCS        += $(wildcard ../source/common/*.cpp)

# Set compile flags
GMI_CFLAGS              = -Wall -O3

# Set link flags
GMI_LFLAGS              =

# Set related modules
GMI_RELATED_MODULES     = osal pcap pcap_session tool_protocol tinyxml

# Set include directory
GMI_INCLUDE_DIR         = ../source/common

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk

