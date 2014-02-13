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

SvnVersion=$(shell (svn info | grep "Revision:" | cut -d " " -f 2 ) 2>/dev/null)
ifeq ($(SvnVersion),)
SvnVersion=$(shell (git svn info | grep "Revision:" | cut -d " " -f 2 ) 2>/dev/null)
endif

ifeq ($(SvnVersion),)
$(error not set SvnVersion)
endif


# Set type of this module, value is in ['executable', 'shared_lib', 'static_lib']
GMI_MODULE_TYPE         = executable

# Set the target name of this module
GMI_MODULE_TARGET_NAME  = $(GMI_MODULE_NAME)

# Set the source code files to compile
GMI_MODULE_SRCS         = $(wildcard *.c)

# Set compile flags
GMI_CFLAGS              = -Wall -O3 -DSVN=$(SvnVersion)
#GMI_CFLAGS             = -Wall -g

# Set link flags
GMI_LFLAGS              =
#GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = pthread osal log_utils net_manage command_pipeline rudp daemon_heartbeat xml2 config gmi_brdwrapper rt

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../include

# Set other header file path
GMI_INCLUDE_DIR         = ../../
GMI_INCLUDE_DIR        += ../../../

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk

