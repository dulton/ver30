# Check if source file list is empty
ifeq ($(GMI_MODULE_SRCS),)
$(error No source file, please set GMI_MODULE_SRCS)
endif

# Check if target name of this module is empty
ifeq ($(GMI_MODULE_TARGET_NAME),)
$(error Please set GMI_MODULE_TARGET_NAME)
endif

# Check if simulator is X86_64, we shall add flag "-m32" to compile source code as elf32-i386
ifeq ($(GMI_PLATFORM),simulator)
ifeq ($(shell uname -p),x86_64)
GMI_CFLAGS += -m32
GMI_LFLAGS += -m32
endif
endif


# Set the final target of this module
ifeq ($(GMI_MODULE_TYPE),executable)
GMI_TARGET         = $(GMI_MODULE_OUTPUT_DIR)/$(GMI_MODULE_TARGET_NAME)
ifeq ($(findstring unitest,$(GMI_MODULE_TARGET_NAME)),unitest)
GMI_INSTALL_TARGET = $(GMI_PLATFORM_INSTALL_DIR)/unitest/$(GMI_MODULE_TARGET_NAME)
else
GMI_INSTALL_TARGET = $(GMI_PLATFORM_INSTALL_DIR)/binary/$(GMI_MODULE_TARGET_NAME)
endif
else
ifeq ($(GMI_MODULE_TYPE),shared_lib)
# Add complie flags and link flags for shared library
GMI_CFLAGS += -fpic
GMI_LFLAGS += -shared
GMI_TARGET         = $(GMI_MODULE_OUTPUT_DIR)/lib$(GMI_MODULE_TARGET_NAME).so
GMI_INSTALL_TARGET = $(GMI_PLATFORM_INSTALL_DIR)/library/lib$(GMI_MODULE_TARGET_NAME).so
else
ifeq ($(GMI_MODULE_TYPE),static_lib)
GMI_TARGET         = $(GMI_MODULE_OUTPUT_DIR)/lib$(GMI_MODULE_TARGET_NAME).a
GMI_INSTALL_TARGET = $(GMI_PLATFORM_INSTALL_DIR)/library/lib$(GMI_MODULE_TARGET_NAME).a
else
$(error Please set GMI_MODULE_TYPE use 'executable', 'shared_lib' or 'static_lib')
endif
endif
endif

# Search libraries in mouldes or prebuilt
GMI_SOURCE_LIBRARYS   = $(foreach item,$(GMI_RELATED_MODULES),$(shell [ -f $(GMI_PLATFORM_INSTALL_DIR)/library/lib$(item).a ] && echo "$(GMI_PLATFORM_INSTALL_DIR)/library/lib$(item).a"))
ifeq ($(shell [ -d $(GMI_PREBUILT_DIR) ] && echo 'y'),y)
GMI_PREBUILT_LIBRARYS = $(foreach item,$(GMI_RELATED_MODULES),$(shell [ -f $(GMI_PLATFORM_PREBUILT_DIR)/library/lib$(item).a ] && echo "$(GMI_PLATFORM_PREBUILT_DIR)/library/lib$(item).a"))
endif

# Add module header file path
GMI_INCLUDE_DIR += $(GMI_MODULE_INCLUDE_DIR)
ifneq ($(GMI_EXTRA_LIBRARIES),)
GMI_RELATED_MODULES += $(GMI_EXTRA_LIBRARIES)
endif
# Add other compile flags
# Set header files search path
GMI_CFLAGS += $(foreach item,$(GMI_INCLUDE_DIR),-I$(shell cd $(item) && pwd))
# Set related modules header file search path, both prebuilt and uncompiled modules
GMI_CFLAGS += $(foreach item,$(GMI_RELATED_MODULES),$(shell [ -d $(GMI_MODULES_DIR)/$(item)/include ] && echo "-I$(GMI_MODULES_DIR)/$(item)/include"))
ifeq ($(shell [ -d $(GMI_PREBUILT_DIR) ] && echo 'y'),y)
GMI_CFLAGS += $(foreach item,$(GMI_RELATED_MODULES),$(shell [ -d $(GMI_PREBUILT_DIR)/include/$(item) ] && echo "-I$(GMI_PREBUILT_DIR)/include/$(item)"))
GMI_CFLAGS += -I$(GMI_PREBUILT_DIR)/include
endif
GMI_CFLAGS += $(GMI_EXTRA_CFLAGS)

# Add other link flags
ifeq ($(GMI_MODULE_TYPE),executable)
GMI_LFLAGS += -L$(GMI_PLATFORM_INSTALL_DIR)/library
ifeq ($(shell [ -d $(GMI_PLATFORM_PREBUILT_DIR) ] && echo 'y'),y)
GMI_LFLAGS += -L$(GMI_PLATFORM_PREBUILT_DIR)/library
endif
GMI_LFLAGS += $(foreach item,$(GMI_RELATED_MODULES),-l$(item))
endif

# Get object file list through source file list
GMI_RELATIVE_MODULE_OBJS  = $(patsubst %.cpp,$(GMI_MODULE_OUTPUT_DIR)/%.o,$(filter %.cpp,$(GMI_MODULE_SRCS)))
GMI_RELATIVE_MODULE_OBJS += $(patsubst %.c,$(GMI_MODULE_OUTPUT_DIR)/%.o,$(filter %.c,$(GMI_MODULE_SRCS)))

# Get absolute file path for all object files
GMI_GET_ABSOLUTE_PATH  = $(shell $(GMI_MKDIR) $(dir $(1)) && cd $(dir $(1)) && pwd)/$(notdir $(1))
GMI_MODULE_OBJS       := $(foreach item,$(GMI_RELATIVE_MODULE_OBJS),$(call GMI_GET_ABSOLUTE_PATH,$(item)))

# Get dependence file list through object file list
GMI_MODULE_DEPS := $(patsubst %.o,%.d,$(GMI_MODULE_OBJS))

all: related_modules $(GMI_TARGET)

# Create all dependence files
$(GMI_PLATFORM_OUTPUT_DIR)/%.d: $(GMI_ABSOLUTE_TOPDIR)/%.cpp
	$(GMI_MAKE_VERBOSE)$(GMI_MKDIR) $(dir $@)
	$(GMI_MAKE_VERBOSE)$(GMI_CC) -MM -MT $(@:.d=.o) $(GMI_CFLAGS) $< | sed "s^ *: *^ $@: ^g" > $@

$(GMI_PLATFORM_OUTPUT_DIR)/%.d: $(GMI_ABSOLUTE_TOPDIR)/%.c
	$(GMI_MAKE_VERBOSE)$(GMI_MKDIR) $(dir $@)
	$(GMI_MAKE_VERBOSE)$(GMI_CC) -MM -MT $(@:.d=.o) $(GMI_CFLAGS) $< | sed "s^ *: *^ $@: ^g" > $@

# Include all dependence files if exists
ifneq ($(GMI_MODULE_DEPS),)
-include $(GMI_MODULE_DEPS)
endif

# Compile all the source files to object files
$(GMI_PLATFORM_OUTPUT_DIR)/%.o: $(GMI_ABSOLUTE_TOPDIR)/%.cpp
	$(GMI_MAKE_VERBOSE)echo "  Compiling $< ..."
	$(GMI_MAKE_VERBOSE)$(GMI_CC) $(GMI_CFLAGS) $< -c -o $@

$(GMI_PLATFORM_OUTPUT_DIR)/%.o: $(GMI_ABSOLUTE_TOPDIR)/%.c
	$(GMI_MAKE_VERBOSE)echo "  Compiling $< ..."
	$(GMI_MAKE_VERBOSE)$(GMI_CC) $(GMI_CFLAGS) $< -c -o $@

# Link all the object files to final target
$(GMI_TARGET): $(GMI_MODULE_OBJS) $(GMI_SOURCE_LIBRARYS) $(GMI_PREBUILT_LIBRARYS)
	$(GMI_MAKE_VERBOSE)echo "Building $(notdir $@) ..."
ifeq ($(GMI_MODULE_TYPE),executable)
	$(GMI_MAKE_VERBOSE)$(GMI_CC) -Wl,--start-group $(GMI_MODULE_OBJS) $(GMI_LFLAGS) -Wl,--end-group -o $@
	#$(GMI_MAKE_VERBOSE)$(GMI_STRIP) $@
else
ifeq ($(GMI_MODULE_TYPE),shared_lib)
	$(GMI_MAKE_VERBOSE)$(GMI_CC) $(GMI_MODULE_OBJS) $(GMI_LFLAGS) -o $@
else
ifeq ($(GMI_MODULE_TYPE),static_lib)
	$(GMI_MAKE_VERBOSE)$(GMI_AR) crs $@ $(GMI_MODULE_OBJS)
else
$(error Please set GMI_MODULE_TYPE use 'executable', 'shared_lib' or 'static_lib')
endif
endif
endif

# Build related modules
related_modules:
	$(GMI_MAKE_VERBOSE) \
	for item in $(GMI_RELATED_MODULES); do \
	  if [ -d $(GMI_MODULES_DIR)/$$item ]; then \
	    $(GMI_MAKE) -C $(GMI_MODULES_DIR)/$$item; \
	    $(GMI_MAKE) -C $(GMI_MODULES_DIR)/$$item install; \
	  fi; \
	done

# Clean all files that created while compiling
clean:
	$(GMI_MAKE_VERBOSE)$(GMI_RM) $(GMI_TARGET) $(GMI_MODULE_OBJS) $(GMI_MODULE_DEPS)

# Install binary file and header files for library
install: $(dir $(GMI_INSTALL_TARGET))
ifneq ($(shell diff $(GMI_TARGET) $(GMI_INSTALL_TARGET) 2>/dev/null && echo 'y'),y)
	$(GMI_MAKE_VERBOSE)echo "Installing $(notdir $(GMI_TARGET)) ..."
	$(GMI_MAKE_VERBOSE)$(GMI_COPY) $(GMI_TARGET) $(GMI_INSTALL_TARGET)
endif
ifneq ($(GMI_MODULE_TYPE),executable)
ifneq ($(shell diff --exclude=.svn $(GMI_MODULE_INCLUDE_DIR) $(GMI_INSTALL_DIR)/include/$(GMI_MODULE_NAME) 2>/dev/null && echo 'y'),y)
	$(GMI_MAKE_VERBOSE)echo "Installing header files for $(GMI_MODULE_NAME) ..."
	$(GMI_MAKE_VERBOSE)$(GMI_RM) $(GMI_INSTALL_DIR)/include/$(GMI_MODULE_NAME)
	$(GMI_MAKE_VERBOSE)$(GMI_MKDIR) $(GMI_INSTALL_DIR)/include
	-$(GMI_MAKE_VERBOSE)$(GMI_COPY) $(GMI_MODULE_INCLUDE_DIR) $(GMI_INSTALL_DIR)/include/$(GMI_MODULE_NAME) 2>/dev/null
	# Remove all subversion information
	$(GMI_MAKE_VERBOSE)find $(GMI_INSTALL_DIR)/include/$(GMI_MODULE_NAME) -name .svn | xargs $(GMI_RM)
endif
endif

# Create install directory for this module
$(dir $(GMI_INSTALL_TARGET)):
	$(GMI_MAKE_VERBOSE)$(GMI_MKDIR) $(dir $(GMI_INSTALL_TARGET))

.PHONY: all clean install related_modules
