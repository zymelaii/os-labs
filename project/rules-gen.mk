ENVS         := $(OBJDIR).cache/.envs
CACHED_FILES += $(ENVS)

define configure-file
	vars=`cat $(1) | grep -Po '(?<=@)\w+(?=@)' | tr '\n' ' '`;     \
	cat $(ENVS) | while IFS= read -r row; do                       \
		read -r key value <<<                                      \
			`echo "$${row}" | awk -F' = ' '{print $$1, $$2}'`;     \
		if [[ " $${vars} " =~ " $${key} " ]]; then                 \
			subst=$$(echo "$${value}" | sed -r 's/([/\])/\\\1/g'); \
			sed -i "s/@$${key}@/$${subst}/g" $(1);                 \
		fi;                                                        \
	done
endef

dup-envs:
	@$(foreach v, $(.VARIABLES), $(info $(v) = $($(v)))):
.PHONY: dup-envs

# make environs
$(ENVS): force
	@$(call begin-job,write out make envs,)
	@mkdir -p $(@D)
	@$(MAKE) -s dup-envs | grep -P '^\w+\s*=.*$$' > $@.swp
	@cmp -s $@ $@.swp || cp $@.swp $@
	@$(call end-job,done,write out make envs,)
.PHONY: force

# generated headers for bootloader
$(GENERATED_INCDIR)/%.inc: include/bootloader/%.inc.in $(ENVS)
	@$(call begin-job,generate,$(notdir $@))
	@mkdir -p $(@D)
	@cp $< $@.swp
	@$(call configure-file,$@.swp);                    \
	if cmp -s $@ $@.swp; then                          \
		$(call end-job,skip,generate,$(notdir $@));    \
	else                                               \
		cp $@.swp $@;                                  \
		$(call end-job,done,generate,$(notdir $@));    \
	fi

# generated linker scripts
$(LOADER_LINKER): $(LOADER_LINKER_IN) $(ENVS)
	@$(call begin-job,generate,$(notdir $@))
	@mkdir -p $(@D)
	@cp $< $@.swp
	@$(call configure-file,$@.swp);                    \
	if cmp -s $@ $@.swp; then                          \
		$(call end-job,skip,generate,$(notdir $@));    \
	else                                               \
		cp $@.swp $@;                                  \
		$(call end-job,done,generate,$(notdir $@));    \
	fi

# generated headers for kernel & lib
$(GENERATED_INCDIR)/%.h: include/%.h.in $(ENVS)
	@$(call begin-job,generate,$(notdir $@))
	@mkdir -p $(@D)
	@cp $< $@.swp
	@$(call configure-file,$@.swp);                    \
	if cmp -s $@ $@.swp; then                          \
		$(call end-job,skip,generate,$(notdir $@));    \
	else                                               \
		cp $@.swp $@;                                  \
		$(call end-job,done,generate,$(notdir $@));    \
	fi
