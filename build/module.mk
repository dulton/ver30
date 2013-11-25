# Include common.mk
include $(GMI_TOPDIR)/build/common.mk

# Include platform.mk
include $(GMI_TOPDIR)/build/platform.mk

# Set output directory for this module
GMI_MODULE_OUTPUT_DIR = $(GMI_PLATFORM_OUTPUT_DIR)/$(subst $(GMI_TOPDIR),,$(shell pwd))
