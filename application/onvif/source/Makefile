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
GMI_MODULE_TARGET_NAME  = $(GMI_MODULE_NAME)

# Set the source code files to compile
GMI_MODULE_SRCS         = $(wildcard common/*.c)
GMI_MODULE_SRCS        += $(wildcard gsoap_288/custom/*.c)
GMI_MODULE_SRCS        += $(wildcard gsoap_288/*.c)
GMI_MODULE_SRCS        += $(wildcard generation/*.c)
GMI_MODULE_SRCS        += $(wildcard *.c)

# Set compile flags
GMI_CFLAGS              = -Wall -O3
#GMI_CFLAGS             += -I common/
#GMI_CFLAGS             += -I gsoap_288/
#GMI_CFLAGS             += -I gsoap_288/plugin/
#GMI_CFLAGS             += -I generation/
#GMI_CFLAGS             = -Wall -g

# Set link flags
GMI_LFLAGS              =
#GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = pthread osal log_utils rudp daemon_heartbeat rt command_pipeline sys_client auth_center des

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../include
GMI_MODULE_INCLUDE_DIR += common/
GMI_MODULE_INCLUDE_DIR += gsoap_288/
GMI_MODULE_INCLUDE_DIR += gsoap_288/plugin
GMI_MODULE_INCLUDE_DIR += generation/

# Set other header file path
GMI_INCLUDE_DIR         = ../../ 

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk

