# [in] SOURCE_DIR where to collect the source files
# [in] OUTPUT_DIR where to output the object files
# [out] SOURCE_FILES source files
# [out] OBJECT_FILES object files, include the old ones

SOURCE_DIR ?=
OUTPUT_DIR ?=

SOURCE_FILES :=
SOURCE_FILES += $(wildcard $(SRCDIR)$(SOURCE_DIR)*.c)
SOURCE_FILES += $(wildcard $(SRCDIR)$(SOURCE_DIR)*.s)
SOURCE_FILES += $(wildcard $(SRCDIR)$(SOURCE_DIR)*.S)
SOURCE_FILES += $(wildcard $(SRCDIR)$(SOURCE_DIR)*.asm)

OBJECT_FILES += $(patsubst $(SRCDIR)$(SOURCE_DIR)%,$(OUTPUT_DIR)%.obj,$(SOURCE_FILES))
