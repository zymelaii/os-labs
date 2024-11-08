# rules for dirty flags detection
CACHED_FLAG_LIST  :=
CACHED_FLAG_LIST  += CFLAGS
CACHED_FLAG_LIST  += LDFLAGS
CACHED_FLAG_LIST  += ASFLAGS
CACHED_FLAG_FILES := $(patsubst %,$(OBJDIR).cache/%,$(CACHED_FLAG_LIST))
CACHED_FILES      += $(CACHED_FLAG_FILES)

RT_INCDIRS     :=
RT_INCDIRS     += include/rt/public
RT_INCDIRS     += include/rt/private
KERNEL_INCDIRS :=
KERNEL_INCDIRS += include/rt/public
KERNEL_INCDIRS += include/kernel

$(OBJDIR).cache/%: force
	@mkdir -p $(@D)
	@$(ECHO) "$($*)" | cmp -s $@ || echo "$($*)" > $@
.PHONY: force

# compile c objects
$(OBJDIR)loader/%.c.obj: $(SRCDIR)loader/%.c $(filter %.h,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,cc,$<)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(addprefix -I,$(KERNEL_INCDIRS)) -c -o $@ $<
	@$(call end-job,done,cc,$<)

$(OBJDIR)kernel/%.c.obj: $(SRCDIR)kernel/%.c $(filter %.h,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,cc,$<)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(addprefix -I,$(KERNEL_INCDIRS)) -c -o $@ $<
	@$(call end-job,done,cc,$<)

$(OBJDIR)rt/%.c.obj: $(SRCDIR)rt/%.c $(filter %.h,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,cc,$<)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(addprefix -I,$(RT_INCDIRS)) -c -o $@ $<
	@$(call end-job,done,cc,$<)

$(OBJDIR)user/%.c.obj: $(SRCDIR)user/%.c $(filter %.h,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,cc,$<)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(addprefix -I,$(RT_INCDIRS)) -c -o $@ $<
	@$(call end-job,done,cc,$<)

$(OBJDIR)%.c.obj: $(SRCDIR)%.c $(filter %.h,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,cc,$<)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<
	@$(call end-job,done,cc,$<)

# compile assembly objects
$(OBJDIR)%.asm.obj: $(SRCDIR)%.asm $(filter %.inc,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,as,$<)
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) -MD $(patsubst %.obj,%.d,$@) -f elf -o $@ $<
	@$(call end-job,done,as,$<)
