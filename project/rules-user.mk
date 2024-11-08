define add-user-program-deps
$(OBJDIR)user/$(1).bin: $(USER_$(1)_OBJECTS) $(LIBRT_FILE) $(LIBGCC_FILE)
endef

# attach deps for each user program
$(foreach program,$(USER_PROGRAMS),$(eval $(call add-user-program-deps,$(program))))

# dispatch recipe to all user program rules
$(USER_PROGRAM_FILES):
	@$(call begin-job,ld,$(notdir $@))
	@\
	if [ ! -z "$^" ]; then         \
		mkdir -p $(@D);            \
		$(LD) $(LDFLAGS) -o $@ $^; \
	fi
	@$(call end-job,done,ld,$(notdir $@))
