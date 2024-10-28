# shell used by mkfiles
SHELL := $(shell echo `which bash 2> /dev/null`)
ifeq ($(SHELL),)
    $(error bash is required to complete the build)
endif

# avoid using builtin `echo` command
ECHO := $(shell which echo)

# pretty print method for jobs
define begin-job
	printf "\e[?25l"; \
	printf "%s.%03d [PROC] $(1) $(2)" "$$(date '+%Y-%m-%d %H:%M:%S')" "$$(echo "$$(date +%N) / 1000000" | bc)"
endef

define end-job-as-done
	$(ECHO) -e "\e[32m[DONE]\e[0m $(1) $(2)"
endef

define end-job-as-skip
	$(ECHO) -e "\e[33m[SKIP]\e[0m $(1) $(2)"
endef

define end-job-as-fail
	$(ECHO) -e "\e[31m[FAIL]\e[0m $(1) $(2)"
endef

define end-job
	printf "\r%s.%03d " "$$(date '+%Y-%m-%d %H:%M:%S')" "$$(echo "$$(date +%N) / 1000000" | bc)"; \
	$(call end-job-as-$(1),$(2),$(3)); \
	printf "\e[?25h"
endef

# path to project mk files
PROJMK_PREFIX ?=

# path to generated headers
GENERATED_INCDIR := $(OBJDIR)include

# output image
IMAGE_FILE := $(OBJDIR)$(IMAGE_NAME).img

# partition file for image
PARTITION_FILE := part.sfdisk

# memory layout of loader
LOADER_LOAD_BASE   := 0x9000
LOADER_LOAD_OFFSET := 0x0400
LOADER_PHY_BASE    := $(shell \
printf "0x%08x" "$$[ $(LOADER_LOAD_BASE) * 16 ]" \
)

# output kernel
KERNEL_FULL_NAME       := $(KERNEL_NAME).bin
KERNEL_DEBUG_FULL_NAME := $(KERNEL_NAME)d.bin
KERNEL_FILE            := $(OBJDIR)kernel/$(KERNEL_FULL_NAME)
KERNEL_DEBUG_FILE      := $(OBJDIR)kernel/$(KERNEL_DEBUG_FULL_NAME)

# memory layout of kernel
KERNEL_START_ADDR := 0x04000000
KERNEL_FAT_ADDR   := 0x00300000
KERNEL_ELF_ADDR   := 0x00400000
KERNEL_BUF_ADDR   := 0x00500000

# fat name of kernel
KERNEL_FAT_NAME := $(shell \
basename=$(shell echo -n $(basename $(notdir $(KERNEL_FULL_NAME))) | cut -c -8 | tr 'a-z' 'A-Z');    \
suffix=$(shell echo -n $(patsubst .%,%,$(suffix $(KERNEL_FULL_NAME))) | cut -c -3 | tr 'a-z' 'A-Z'); \
printf "%-*s%-*s" "8" "$${basename}" "3" "$${suffix}"                                                \
)

# standard library for kernel
LIBRT_FILE := $(OBJDIR)lib/lib$(LIBRT).a

# game lib for lab4
LIBGAME_FILE := libch4Core.a

# configure toolchain
DEFINES  ?=
INCDIRS  ?=
INCDIRS  += include
INCDIRS  += include/bootloader
LINKDIRS ?=
LINKDIRS += $(OBJDIR)lib
include $(PROJMK_PREFIX)conf-toolchain.mk

# configure qemu
QEMU_ARCH   := i386
QEMU_MEMORY := 256
include $(PROJMK_PREFIX)conf-qemu.mk

# configure gdb
GDB_SCRIPTS_HOME := share/gdb/
include $(PROJMK_PREFIX)conf-gdb.mk

# libgcc, introduced mainly for some useful built-in functions
LIBGCC_FILE := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
ifeq ($(LIBGCC_FILE),)
    $(error requires gcc-multilib to complete the build)
endif
