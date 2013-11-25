# Check if GMI_PLATFORM is empty
ifeq ($(GMI_PLATFORM),)
$(error Please set GMI_PLATFORM)
endif

# Check if GMI_PLATFORM is supported
ifneq ($(shell [ -d $(GMI_TOPDIR)/platform/$(GMI_PLATFORM) ] && echo 'y'),y)
$(error Do not support platform '$(GMI_PLATFORM)')
endif

# Set platform dir
GMI_PLATFORM_DIR          = $(GMI_TOPDIR)/platform/$(GMI_PLATFORM)

# Set output dir for platform
GMI_PLATFORM_OUTPUT_DIR   = $(GMI_OUTPUT_DIR)/$(GMI_PLATFORM)

# Set install dir for platform
GMI_PLATFORM_INSTALL_DIR  = $(GMI_INSTALL_DIR)/$(GMI_PLATFORM)

# Set prebuilt dir for platform
GMI_PLATFORM_PREBUILT_DIR = $(GMI_PREBUILT_DIR)/$(GMI_PLATFORM)

# Include platform.mk
include $(GMI_PLATFORM_DIR)/build/platform.mk
