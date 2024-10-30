# path to project mk files
PROJMK_PREFIX ?=

# collect user objects
SOURCE_DIR   := user/
OUTPUT_DIR   := $(OBJDIR)$(SOURCE_DIR)
SOURCE_FILES := $(shell find $(SRCDIR)$(SOURCE_DIR) -type f)
OBJECT_FILES += $(patsubst $(SRCDIR)$(SOURCE_DIR)%,$(OUTPUT_DIR)%.obj,$(SOURCE_FILES))

USER_PROGRAMS      := $(notdir $(wildcard $(SRCDIR)$(SOURCE_DIR)*))
USER_PROGRAM_FILES := $(patsubst %,$(OUTPUT_DIR)%.bin,$(USER_PROGRAMS))

# collect objects for each user program into variable USER_$(program)_OBJECTS
$(foreach program,$(USER_PROGRAMS),$(eval USER_$(program)_OBJECTS := $(filter $(OUTPUT_DIR)$(program)/%.obj,$(OBJECT_FILES))))
