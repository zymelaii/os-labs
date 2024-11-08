# path to project mk files
PROJMK_PREFIX ?=

# configure output dir
OBJDIR ?= build/

# configure source dir
SRCDIR ?= src/

# output name of the image
IMAGE_NAME ?= disk

# output name of the kernel
KERNEL_NAME ?= kernel

# standard library for both kernel and user space
LIBRT ?= rt

# ring0 library shared with kernel and loader
LIBR0 ?= ring0

include $(PROJMK_PREFIX)conf-internal.mk
