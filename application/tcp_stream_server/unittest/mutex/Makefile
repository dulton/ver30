# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= ambarella_a5s_sdk_v3.3

# Set name of this module
GMI_MODULE_NAME         = test_mutex

# Set top dir of the compile system
GMI_TOPDIR             ?= $(word 1, $(subst application/, ,$(shell pwd)))

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
GMI_MODULE_SRCS         = test_mutex.cpp

# Set compile flags
#GMI_CFLAGS              = -Wall -O3
GMI_CFLAGS             = -Wall 

# Set link flags
#GMI_LFLAGS              =
GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = pthread osal

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  =  ${GMI_TOPDIR}/prebuilt/include/libev  
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/osal ${GMI_TOPDIR}/prebuilt/include/ipc_media_data_dispatch/
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/command_pipeline/ ${GMI_TOPDIR}/prebuilt/include/media/

# Set other header file path
GMI_INCLUDE_DIR         = 

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk



