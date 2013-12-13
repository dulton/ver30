# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= simulator

# Set name of this module
GMI_MODULE_NAME         = $(notdir $(shell cd .. && pwd))

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst modules, ,$(shell pwd)))

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
GMI_MODULE_SRCS         = $(wildcard *.c)
GMI_MODULE_SRCS         += $(wildcard *.cpp)

# Set compile flags
GMI_CFLAGS              = -Wall -O3
#GMI_CFLAGS             = -Wall -g

# Set link flags
GMI_LFLAGS              =
#GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = pthread osal rudp 

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../include
GMI_INCLUDE_DIR = ../../../../application/

# Set other header file path
GMI_INCLUDE_DIR += ./
GMI_INCLUDE_DIR += ../../../../prebuilt/include/sqlite3
GMI_INCLUDE_DIR += ../../../../prebuilt/include/ipc_media_data_dispatch
GMI_INCLUDE_DIR += ../../../../prebuilt/include/command_pipeline
GMI_INCLUDE_DIR += ../../../../prebuilt/include/media

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk

