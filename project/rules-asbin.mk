$(OBJDIR)%.bin: $(SRCDIR)%.asm $(filter %.inc,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,as,$(notdir $@))
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) -MD $(patsubst %.bin, %.asm.d, $@) -o $@ $<
	@$(call end-job,done,as,$(notdir $@))

$(LOADER_FILE): $(LOADER_OBJECTS) $(LIBRT_FILE) $(LOADER_LINKER) $(filter %.inc,$(GENERATED_FILES)) $(CACHED_FLAG_FILES)
	@$(call begin-job,ld,$(notdir $@))
	@mkdir -p $(@D)
	@$(LD) $(LDFLAGS) -T $(LOADER_LINKER) -s --oformat binary -o $@ $(LOADER_OBJECTS) $(LIBRT_FILE)
	@$(call end-job,done,ld,$(notdir $@))