# [in] PREFIX prefix of the toolchain
# [in] DEFINES definations
# [in] INCDIRS include directories
# [in] LIBDIRS link directories
# [out] CC c compiler executable
# [out] AS assembler executable
# [out] AR ar executable
# [out] LD ld executable
# [out] NM nm executable
# [out] OBJCOPY objcopy executable
# [out] OBJDUMP objdump executable
# [in, out] CFLAGS flags for compiler
# [in, out] ASFLAGS flags for assembler
# [in, out] LDFLAGS flags for linker

PREFIX ?=

AS      := nasm
CC      := $(PREFIX)gcc
AR      := $(PREFIX)ar
LD      := $(PREFIX)ld
NM      := $(PREFIX)nm
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump

DEFINES ?=
INCDIRS ?=
INCDIRS += $(GENERATED_INCDIR)
LIBDIRS ?=

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
CFLAGS += -mno-sse -mno-sse2 #<! default qemu cpu core may not support sse

ASFLAGS ?=
ASFLAGS += $(addprefix -I,$(INCDIRS))

LDFLAGS ?=
LDFLAGS += -m elf_i386
LDFLAGS += -nostdlib
LDFLAGS += $(addprefix -L,$(LIBDIRS))
