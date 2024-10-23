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

# standard library for the kernel
LIBRT ?= rt

include $(PROJMK_PREFIX)conf-internal.mk
