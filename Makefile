SHELL := $(shell echo `which bash 2> /dev/null`)
ifeq ($(SHELL),)
    $(error bash is required to complete the build)
endif

OBJDIR ?= build
OBJDIR := $(if $(OBJDIR),$(OBJDIR),.)

PREFIX ?=
ARCH   ?= i386
MEMORY ?= 128

AS      := nasm
CC      := $(PREFIX)gcc
AR      := $(PREFIX)ar
LD      := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
NM      := $(PREFIX)nm
GDB     := gdb
QEMU    := qemu-system-$(ARCH)
ECHO    := $(shell which echo)

DEFINES ?=
INCDIRS ?= include
LIBDIRS ?=

ASFLAGS ?=
ASFLAGS += $(addprefix -I,$(INCDIRS))

CFLAGS ?=
CFLAGS += $(addprefix -D,$(DEFINES))
CFLAGS += $(addprefix -I,$(INCDIRS)) -MD
CFLAGS += $(addprefix -L,$(LIBDIRS))
CFLAGS += -fno-stack-protector
CFLAGS += -O0
CFLAGS += -std=gnu99
CFLAGS += -fno-builtin -nostdinc -nostdlib
CFLAGS += -m32
CFLAGS += -static -fno-pie
CFLAGS += -g
CFLAGS += -Werror -Wno-unused-variable -Wno-unknown-attributes
CFLAGS += -mno-sse -mno-sse2

LDFLAGS ?=
LDFLAGS += -m elf_i386
LDFLAGS += -nostdlib
LDFLAGS += $(addprefix -L,$(LIBDIRS))

GDB_SCRIPTS_HOME ?= share/gdb/
GDB_REALMODE_XML := $(GDB_SCRIPTS_HOME)target.xml
GDB_FLAGS        ?=
GDB_FLAGS        += -q -nx
GDB_FLAGS        += -x '$(GDB_SCRIPTS_HOME)connect-qemu.gdb'
GDB_FLAGS        += -x '$(GDB_SCRIPTS_HOME)instr-level.gdb'
GDB_FLAGS        += -x '$(GDB_SCRIPTS_HOME)layout.gdb'
GDB_FLAGS        += -ex 'set pagination off'
GDB_FLAGS        += -ex 'set confirm off'
GDB_FLAGS        += -ex 'set disassembly-flavor intel'
GDB_FLAGS        += -ex 'connect-qemu'
GDB_FLAGS        += -ex 'focus cmd'

QEMU_DISPLAY ?=
QEMU_FLAGS   ?=
QEMU_FLAGS   += -boot order=c
QEMU_FLAGS   += -serial file:$(OBJDIR)/serial-$(shell date +%Y%M%d%H%M).log
QEMU_FLAGS   += -m $(MEMORY)m
QEMU_FLAGS   += $(if $(QEMU_DISPLAY),-display $(QEMU_DISPLAY),)

BOOT_FILE   = $(OBJDIR)/boot.bin
LOADER_FILE = $(OBJDIR)/loader.bin
IMAGE_FILE  = $(OBJDIR)/disk.img
TEXT_FILE   = $(OBJDIR)/aA1.txt

all: $(IMAGE_FILE)
.PHONY: all

$(IMAGE_FILE): $(BOOT_FILE) $(LOADER_FILE) $(TEXT_FILE)
	@$(ECHO) -ne "[PROC] build $(notdir $@)\r"
	@mkdir -p $(@D)
	@dd if=/dev/zero of=$@ bs=1024 count=16384 > /dev/null 2>&1
	@mkfs.vfat -F 32 -s 8 $@ > /dev/null 2>&1
	@dd if=$(BOOT_FILE) of=$@ bs=1 count=420 seek=90 conv=notrunc > /dev/null 2>&1
	@\
	mount_point=$$(mktemp -d);              \
	sudo mount -o loop $@ $$mount_point;    \
	sudo cp $(LOADER_FILE) $$mount_point;   \
	sudo cp $(TEXT_FILE) $$mount_point;     \
	sudo umount $$mount_point
	@$(ECHO) -e "\e[1K\r\e[32m[DONE]\e[0m build $(notdir $@)"

$(OBJDIR)/%.bin: src/%.asm
	@$(ECHO) -ne "[PROC] as $(notdir $@)\r"
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) -o $@ $<
	@$(ECHO) -e "\e[1K\r\e[32m[DONE]\e[0m as $(notdir $@)"

$(TEXT_FILE):
	@\
	if [ ! -f $(TEXT_FILE) ]; then                                      \
		$(ECHO) -ne "[PROC] generate $(notdir $@)\r";                   \
		mkdir -p $(@D);                                                 \
		cat /dev/urandom | tr -dc '[:alnum:]' | head -c 40960 > $@;     \
		$(ECHO) -e "\e[1K\r\e[32m[DONE]\e[0m generate $(notdir $@)";    \
	fi

help: #<! display this information
	@awk 'BEGIN {FS = ":.*?#<! "} /^[a-zA-Z_-]+:.*?#<! / {sub("\\\\n",sprintf("\n%22c"," "), $$2);printf " \033[36m%-12s\033[0m  %s\n", $$1, $$2}' $(MAKEFILE_LIST)
.PHONY: help

clean: #<! clean output files
	@\
	if [ ! -d $(OBJDIR) ]; then exit; fi;                           \
	$(ECHO) -ne "[PROC] clean up all stuffs\r";                     \
	if rm -rf $(OBJDIR) 2> /dev/null; then                          \
		$(ECHO) -e "\e[1K\r\e[32m[DONE]\e[0m clean up all stuffs";  \
	else                                                            \
		$(ECHO) -e "\e[1K\r\e[31m[FAIL]\e[0m clean up all stuffs";  \
	fi
.PHONY: clean

run: $(IMAGE_FILE) #<! run qemu with image
	@$(QEMU) $(QEMU_FLAGS) -drive file=$<,format=raw
.PHONY: run

debug: $(IMAGE_FILE) #<! run qemu with image in debug mode
	@$(QEMU) $(QEMU_FLAGS) -drive file=$<,format=raw -s -S
.PHONY: debug

monitor: #<! run monitor for qemu
	@$(GDB) $(GDB_FLAGS) -ex 'layout-sac' -ex 'c'
.PHONY: monitor

monitor-real: $(GDB_REALMODE_XML) #<! run monitor for qemu in real mode
	@$(GDB) $(GDB_FLAGS)            \
		-ex 'layout-rac'            \
		-ex 'set tdesc filename $<' \
		-ex 'b *0x7c00'             \
		-ex 'c'
.PHONY: monitor-real

build: all #<! build all
image: $(IMAGE_FILE) #<! build image file

b: build
r: run
d: debug
mon: monitor
mon-real: monitor-real

.DELETE_ON_ERROR:
