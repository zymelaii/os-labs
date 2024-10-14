#include <config.h>
#include <elf.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <terminal.h>
#include <vga.h>

//! load kernel elf and jump to the entry point
void load_kernel() {
  term_clear();

  const char *msg_enter = "--- start loading kernel elf ---";
  const char *msg_leave = "---    kernel elf loaded     ---";
  const u8 char_attr = (u8)(DEFAULT_COLOR >> 8);

  term_write_str(TERMINAL_POS(0, 0), msg_enter, char_attr);

  Elf32_Ehdr *kernel_ehdr = (Elf32_Ehdr *)KERNEL_ELF;
  Elf32_Phdr *kernel_phdr = (void *)kernel_ehdr + kernel_ehdr->e_phoff;

  for (u32 i = 0; i < kernel_ehdr->e_phnum; i++, kernel_phdr++) {
    //! only PT_LOAD segments are required to be loaded into memory
    if (kernel_phdr->p_type != PT_LOAD) {
      continue;
    }
    //! copy elf segments to destination virtual addr
    memcpy((void *)kernel_phdr->p_vaddr,
           (void *)kernel_ehdr + kernel_phdr->p_offset, kernel_phdr->p_filesz);
    //! zero the remaining bytes in the segment, normally .bss section
    memset((void *)kernel_phdr->p_vaddr + kernel_phdr->p_filesz, 0,
           kernel_phdr->p_memsz - kernel_phdr->p_filesz);
  }

  term_write_str(TERMINAL_POS(1, 0), msg_leave, char_attr);

  busy_delay(32);

  ((void (*)(void))(kernel_ehdr->e_entry))();
}
