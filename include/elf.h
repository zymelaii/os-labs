#pragma once

#include <stdint.h>

#define ELF_MAGIC 0x464c457f  //<! i.e. '\x7f' 'E' 'L' 'F'

typedef struct elf32_hdr {
  u32 e_magic;  //<! always ELF_MAGIC
  u8 e_elf[12];
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  u32 e_entry;
  u32 e_phoff;
  u32 e_shoff;
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize;
  u16 e_phnum;
  u16 e_shentsize;
  u16 e_shnum;
  u16 e_shstrndx;
} elf32_hdr_t;

typedef struct elf32_phdr {
  u32 p_type;
  u32 p_offset;
  u32 p_vaddr;
  u32 p_paddr;
  u32 p_filesz;
  u32 p_memsz;
  u32 p_flags;
  u32 p_align;
} elf32_phdr_t;

struct elf32_shdr {
  u32 sh_name;
  u32 sh_type;
  u32 sh_flags;
  u32 sh_addr;
  u32 sh_offset;
  u32 sh_size;
  u32 sh_link;
  u32 sh_info;
  u32 sh_addralign;
  u32 sh_entsize;
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
