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
GMI_MODULE_SRCS         = $(wildcard *.cpp)
GMI_MODULE_SRCS        += $(wildcard live555ex/*.cpp)
GMI_MODULE_SRCS        += $(wildcard rtsp_service/*.cpp)
GMI_MODULE_SRCS        += $(wildcard media_capture/*.cpp)
GMI_MODULE_SRCS        += $(wildcard task_scheduler/*.cpp)
GMI_MODULE_SRCS        += $(wildcard stream_ctrl/*.cpp)
GMI_MODULE_SRCS        += $(wildcard log_module/*.cpp)

# Set compile flags
GMI_CFLAGS              = -Wall -O3 -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64

# Set link flags
GMI_LFLAGS              =

# Set related modules
GMI_RELATED_MODULES     = streaming_pipeline ipc_media_data_dispatch osal media command_pipeline pthread live555 daemon_heartbeat rudp rt sys_client auth_center des

# Set other header file path
GMI_INCLUDE_DIR         = $(GMI_TOPDIR)/application
GMI_INCLUDE_DIR        += $(GMI_TOPDIR)/prebuilt/include/live555/UsageEnvironment
GMI_INCLUDE_DIR        += $(GMI_TOPDIR)/prebuilt/include/live555/groupsock
GMI_INCLUDE_DIR        += $(GMI_TOPDIR)/prebuilt/include/live555/liveMedia
GMI_INCLUDE_DIR        += $(GMI_TOPDIR)/prebuilt/include/live555/BasicUsageEnvironment
GMI_INCLUDE_DIR        += $(shell pwd)
GMI_INCLUDE_DIR        += $(shell pwd)/live555ex
GMI_INCLUDE_DIR        += $(shell pwd)/rtsp_service
GMI_INCLUDE_DIR        += $(shell pwd)/media_capture
GMI_INCLUDE_DIR        += $(shell pwd)/task_scheduler
GMI_INCLUDE_DIR        += $(shell pwd)/stream_ctrl
GMI_INCLUDE_DIR        += $(shell pwd)/log_module

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk

