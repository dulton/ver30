# Check if current shell is bash
GMI_CURRENT_SHELL   = $(notdir $(if $(shell [ -L $(SHELL) ] && echo 'y'),$(shell readlink -f $(SHELL)),$(SHELL)))
ifneq ($(GMI_CURRENT_SHELL),bash)
$(error Please make sure you are using bash. Now you are using $(GMI_CURRENT_SHELL))
endif

# Set all commands
CROSS_COMPILE      ?= 
GMI_MAKE            = make -s
GMI_MKDIR           = mkdir -p
GMI_RM              = rm -rf
GMI_COPY            = cp -rf
ifeq ($(GMI_CC),)
GMI_CC              = $(CROSS_COMPILE)g++
endif
GMI_AR              = $(CROSS_COMPILE)ar
GMI_STRIP           = $(CROSS_COMPILE)strip

# Get absolute dir for top dir
GMI_ABSOLUTE_TOPDIR = $(shell cd $(GMI_TOPDIR) && pwd)

# Set output dir
GMI_OUTPUT_DIR      = $(GMI_ABSOLUTE_TOPDIR)/output

# Set install dir
GMI_INSTALL_DIR     = $(GMI_ABSOLUTE_TOPDIR)/release

# Set modules dir
GMI_MODULES_DIR     = $(GMI_ABSOLUTE_TOPDIR)/modules/gmi

# Set prebuilt dir
GMI_PREBUILT_DIR    = $(GMI_ABSOLUTE_TOPDIR)/prebuilt
