# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= ambarella_a5s_sdk_v3.3

# Set name of this module
GMI_MODULE_NAME         = $(notdir $(shell cd .. && pwd))

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst application/${GMI_MODULE_NAME}, ,$(shell pwd)))

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
GMI_MODULE_SRCS         = main.cpp tcp_source_buffer.cpp tcp_source_stream.cpp tcp_stream_acc.cpp tcp_stream_data.cpp

# Set compile flags
#GMI_CFLAGS              = -Wall -O3
GMI_CFLAGS             = -Wall 

# Set link flags
#GMI_LFLAGS              =
GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = pthread osal ipc_media_data_dispatch ev command_pipeline config xml2

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../include ${GMI_TOPDIR}/prebuilt/include/libev  
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/osal ${GMI_TOPDIR}/prebuilt/include/ipc_media_data_dispatch/
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/command_pipeline/ ${GMI_TOPDIR}/prebuilt/include/media/
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/config/

# Set other header file path
GMI_INCLUDE_DIR         = ../../

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk


