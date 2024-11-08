# path to project mk files
PROJMK_PREFIX ?=

# collect standard library objects
SOURCE_DIR := rt/public/
OUTPUT_DIR := $(OBJDIR)$(SOURCE_DIR)
include $(PROJMK_PREFIX)collect-objects.mk

SOURCE_DIR := rt/private/
OUTPUT_DIR := $(OBJDIR)$(SOURCE_DIR)
include $(PROJMK_PREFIX)collect-objects.mk

# collect ring0 library objects
SOURCE_DIR := kernel/public/
OUTPUT_DIR := $(OBJDIR)$(SOURCE_DIR)
include $(PROJMK_PREFIX)collect-objects.mk
