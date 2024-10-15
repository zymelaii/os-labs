# path to project mk files
PROJMK_PREFIX ?=

# collect kernel objects
SOURCE_DIR := kernel/
OUTPUT_DIR := $(OBJDIR)$(SOURCE_DIR)
include $(PROJMK_PREFIX)collect-objects.mk

# kernel objects
KERNEL_OBJECTS  := $(patsubst $(SRCDIR)%,$(OBJDIR)%.obj,$(SOURCE_FILES))
