MAKE_IMAGE_SCRIPT := scripts/make-image.sh

$(IMAGE_FILE): $(MAKE_IMAGE_SCRIPT) $(PARTITION_FILE) $(MBR_FILE) $(BOOT_FILE) $(LOADER_FILE) $(KERNEL_FILE)
	@$(call begin-job,build image,$(notdir $@))
	@$(MAKE_IMAGE_SCRIPT) $@ $(MBR_FILE) $(BOOT_FILE) $(LOADER_FILE) $(KERNEL_FILE)
	@$(call end-job,done,build image,$(notdir $@))
