# build standard library
$(LIBRT_FILE): $(filter $(OBJDIR)rt/%.obj,$(OBJECT_FILES))
	@$(call begin-job,ar,$(notdir $@))
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) -o $@ $^ > /dev/null 2>&1
	@$(call end-job,done,ar,$(notdir $@))

# build public-only standard library
$(LIBRT_PUBLIC_FILE): $(filter $(OBJDIR)rt/public/%.obj,$(OBJECT_FILES))
	@$(call begin-job,ar,$(notdir $@))
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) -o $@ $^ > /dev/null 2>&1
	@$(call end-job,done,ar,$(notdir $@))

# build ring0 library
$(LIBR0_FILE): $(filter $(OBJDIR)kernel/public/%.obj,$(OBJECT_FILES))
	@$(call begin-job,ar,$(notdir $@))
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) -o $@ $^ > /dev/null 2>&1
	@$(call end-job,done,ar,$(notdir $@))
