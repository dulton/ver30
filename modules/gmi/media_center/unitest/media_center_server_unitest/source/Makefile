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
GMI_MODULE_TYPE         = executable

# Set the target name of this module
GMI_MODULE_TARGET_NAME  = $(GMI_MODULE_NAME)

# Set the source code files to compile
GMI_MODULE_SRCS         = $(wildcard *.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/command_executor/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/communication/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/media_filter/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/media_filter/audio/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/media_filter/image/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/media_filter/image/picture/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/media_filter/image/video/*.cpp)
GMI_MODULE_SRCS        += $(wildcard ../../../source/rtsp_server/*.cpp)

# Set compile flags
GMI_CFLAGS              = -Wall -O3
#GMI_CFLAGS             = -Wall -g -pg

# Set link flags
GMI_LFLAGS              =
#GMI_LFLAGS             = -g -pg

# Set related modules
GMI_RELATED_MODULES     = pthread osal command_pipeline log_utils rudp daemon_heartbeat streaming_pipeline ipc_media_data_dispatch xml2 config sys_info_readonly
GMI_RELATED_MODULES    += amba_rtsp media freetype textinsert amp IrCutAe IrCutAdc capability tinyxml img_algo_a5s gmi_brdwrapper Gmi3A asound rt 3dpos

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../../../include

# Set other header file path
GMI_INCLUDE_DIR         = ./
GMI_INCLUDE_DIR        += ../../../source/
GMI_INCLUDE_DIR        += ../../../source/command_executor/
GMI_INCLUDE_DIR        += ../../../source/communication/
GMI_INCLUDE_DIR        += ../../../source/media_filter/
GMI_INCLUDE_DIR        += ../../../source/media_filter/audio/
GMI_INCLUDE_DIR        += ../../../source/media_filter/image/
GMI_INCLUDE_DIR        += ../../../source/media_filter/image/picture/
GMI_INCLUDE_DIR        += ../../../source/media_filter/image/video/
GMI_INCLUDE_DIR        += ../../../source/rtsp_server/
GMI_INCLUDE_DIR        += ../../../../../../application/

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk

