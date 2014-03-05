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
GMI_MODULE_SRCS         = main.cpp sdk_client_login.cpp sdk_client_sock.cpp 
GMI_MODULE_SRCS        += sdk_server_buffer.cpp sdk_server_client.cpp
GMI_MODULE_SRCS        += sdk_server_client_stream.cpp sdk_server_client_sys.cpp
GMI_MODULE_SRCS        += sdk_server_main.cpp sdk_server_mgmt.cpp
GMI_MODULE_SRCS        += sdk_server_session.cpp sdk_server_sock.cpp
GMI_MODULE_SRCS        += sdk_server_stream.cpp sdk_server_sys_comm.cpp
GMI_MODULE_SRCS        += sdk_server_sys_req.cpp sdk_server_debug.cpp sdk_client_daemon.cpp
GMI_MODULE_SRCS        += sdk_audio_buffer.cpp sdk_audio_stream.cpp sdk_audio_dec.cpp 
GMI_MODULE_SRCS        += sdk_audio_dbuffer.cpp sdk_client_comm.cpp sdk_sys_cp.cpp
GMI_MODULE_SRCS        += sdk_server_alarm.cpp sdk_proto_pack.cpp sdk_proto_parse.cpp

# Set compile flags
GMI_CFLAGS              = -Wall -O3 -Wno-psabi
#GMI_CFLAGS             = -Wall -Wno-psabi

# Set link flags
GMI_LFLAGS              =
#GMI_LFLAGS             = -g

# Set related modules
GMI_RELATED_MODULES     = pthread ev osal ipc_media_data_dispatch config xml2 command_pipeline log_utils streaming_pipeline rt

# Set header file path for this module
GMI_MODULE_INCLUDE_DIR  = ../include ${GMI_TOPDIR}/prebuilt/include/  
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/osal ${GMI_TOPDIR}/prebuilt/include/ipc_media_data_dispatch/
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/command_pipeline/ ${GMI_TOPDIR}/prebuilt/include/media/
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/prebuilt/include/config/ ${GMI_TOPDIR}/application/ ${GMI_TOPDIR}/modules/gmi/auth_center/include
GMI_MODULE_INCLUDE_DIR += ${GMI_TOPDIR}/application/auth_server/include/

# Set other header file path
GMI_INCLUDE_DIR         = ../../

# Include rules.mk
include $(GMI_TOPDIR)/build/rules.mk


