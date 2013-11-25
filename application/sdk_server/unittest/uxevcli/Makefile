# Set the platform that we are working on, defualt is 'simulator'
GMI_PLATFORM           ?= ambarella_a5s_sdk_v3.3

# Set name of this module
GMI_MODULE_NAME         = uxevcli

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
GMI_MODULE_SRCS         = clientunixdemo.cpp uxevcli.cpp ../../source/sdk_client_unix_sock.cpp  ../../source/sdk_server_debug.cpp ../../source/sdk_client_comm.cpp

# Set compile flags
#GMI_CFLAGS              = -Wall -O3
GMI_CFLAGS             = -Wall -Wno-psabi

# Set link flags
#GMI_LFLAGS              =
GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = ev osal pthread

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = 

# Set other header file path
GMI_INCLUDE_DIR         = $(GMI_TOPDIR)/application/sdk_server/include $(GMI_TOPDIR)/application/ $(GMI_TOPDIR)/prebuilt/include/ $(GMI_TOPDIR)/prebuilt/include/osal/ \
       $(GMI_TOPDIR)/prebuilt/include/ $(GMI_TOPDIR)/prebuilt/include/command_pipeline/



# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk




