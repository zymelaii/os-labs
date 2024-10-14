# path to project mk files
PROJMK_PREFIX ?=

# collect mbr & boot objects
SOURCE_DIR := boot/
OUTPUT_DIR := $(OBJDIR)$(SOURCE_DIR)
include $(PROJMK_PREFIX)collect-objects.mk

MBR_FILE  := $(OBJDIR)boot/mbr.bin
BOOT_FILE := $(OBJDIR)boot/boot.bin

# collect loader objects
SOURCE_DIR := loader/
OUTPUT_DIR := $(OBJDIR)$(SOURCE_DIR)
include $(PROJMK_PREFIX)collect-objects.mk

LOADER_SIZE_LIMIT := $(shell echo $$[ 0x10000 ])

# loader objects
LOADER_OBJECTS := $(patsubst $(SRCDIR)%,$(OBJDIR)%.obj,$(SOURCE_FILES))
LOADER_LINKER  := $(SRCDIR)$(SOURCE_DIR)linker.ld

LOADER_FILE := $(OBJDIR)boot/loader.bin

GENERATED_FILES += $(GENERATED_INCDIR)/config.inc
GENERATED_FILES += $(GENERATED_INCDIR)/layout.inc
GENERATED_FILES += $(GENERATED_INCDIR)/config.h
