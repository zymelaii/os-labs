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
LOADER_OBJECTS     := $(patsubst $(SRCDIR)%,$(OBJDIR)%.obj,$(SOURCE_FILES))
LOADER_OBJECT_FILE := $(filter %loader.asm.obj,$(LOADER_OBJECTS))
LOADER_LINKER_IN   := $(SRCDIR)$(SOURCE_DIR)linker.ld.in
LOADER_LINKER      := $(OBJDIR)linker.ld

ifeq ($(LOADER_OBJECT_FILE),)
	$(error loader.asm.obj not found)
endif

LOADER_FILE       := $(OBJDIR)boot/loader.bin
LOADER_DEBUG_FILE := $(OBJDIR)boot/loaderd.bin

GENERATED_FILES += $(GENERATED_INCDIR)/config.inc
GENERATED_FILES += $(GENERATED_INCDIR)/layout.inc
GENERATED_FILES += $(GENERATED_INCDIR)/config.h
GENERATED_FILES += $(LOADER_LINKER)
