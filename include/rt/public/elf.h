#pragma once

#include <stdint.h>

#define ELF_MAGIC 0x464c457f  //<! i.e. '\x7f' 'E' 'L' 'F'

typedef struct elf32_hdr {
  uint32_t e_magic;  //<! always ELF_MAGIC
  uint8_t e_elf[12];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} elf32_hdr_t;

typedef struct elf32_phdr {
  uint32_t p_type;
  uint32_t p_offset;
  uint32_t p_vaddr;
  uint32_t p_paddr;
  uint32_t p_filesz;
  uint32_t p_memsz;
  uint32_t p_flags;
  uint32_t p_align;
} elf32_phdr_t;

typedef struct elf32_shdr {
  uint32_t sh_name;
  uint32_t sh_type;
  uint32_t sh_flags;
  uint32_t sh_addr;
  uint32_t sh_offset;
  uint32_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint32_t sh_addralign;
  uint32_t sh_entsize;
} elf32_shdr_t;

//! values of elf32_phdr.p_type
enum {
  PT_NULL = 0,
  PT_LOAD = 1,
  PT_DYNAMIC = 2,
  PT_INTERP = 3,
  PT_NOTE = 4,
  PT_SHLIB = 5,
  PT_PHDR = 6,
  PT_TLS = 7,
  PT_LOOS = 0x60000000,
  PT_HIOS = 0x6fffffff,
  PT_LOPROC = 0x70000000,
  PT_HIPROC = 0x7fffffff,
};

//! flag bits of elf32_phdr.p_flags
enum {
  ELF_PROG_FLAG_EXEC = 1,
  ELF_PROG_FLAG_WRITE = 2,
  ELF_PROG_FLAG_READ = 4,
};

//! values of elf32_shdr.sh_type
enum {
  ELF_SHT_NULL = 0,
  ELF_SHT_PROGBITS = 1,
  ELF_SHT_SYMTAB = 2,
  ELF_SHT_STRTAB = 3,
};

//! values of elf32_shdr.sh_name
enum {
  ELF_SHN_UNDEF = 0,
};
