#pragma once

#include <stdint.h>
#include <sys/types.h>

#define NR_PCBS 3

#define NR_GSREG 0
#define NR_FSREG 1
#define NR_ESREG 2
#define NR_DSREG 3
#define NR_EDIREG 4
#define NR_ESIREG 5
#define NR_EBPREG 6
#define NR_KERNELESPREG 7
#define NR_EBXREG 8
#define NR_EDXREG 9
#define NR_ECXREG 10
#define NR_EAXREG 11
#define NR_RETADR 12
#define NR_EIPREG 13
#define NR_CSREG 14
#define NR_EFLAGSREG 15
#define NR_ESPREG 16
#define NR_SSREG 17

#define P_STACKBASE 0
#define GSREG (NR_GSREG * 4)
#define FSREG (NR_FSREG * 4)
#define ESREG (NR_ESREG * 4)
#define DSREG (NR_DSREG * 4)
#define EDIREG (NR_EDIREG * 4)
#define ESIREG (NR_ESIREG * 4)
#define EBPREG (NR_EBPREG * 4)
#define KERNELESPREG (NR_KERNELESPREG * 4)
#define EBXREG (NR_EBXREG * 4)
#define EDXREG (NR_EDXREG * 4)
#define ECXREG (NR_ECXREG * 4)
#define EAXREG (NR_EAXREG * 4)
#define RETADR (NR_RETADR * 4)
#define EIPREG (NR_EIPREG * 4)
#define CSREG (NR_CSREG * 4)
#define EFLAGSREG (NR_EFLAGSREG * 4)
#define ESPREG (NR_ESPREG * 4)
#define SSREG (NR_SSREG * 4)
#define P_STACKTOP (SSREG + 4)

//! NOTE: see https://en.wikipedia.org/wiki/FLAGS_register
#define EFLAGS_RESERVED 0x0002                 //<! always 1 in eflags
#define EFLAGS_IF 0x0200                       //<! interrupt enable flag
#define EFLAGS_IOPL(pl) (((pl) & 0b11) << 12)  //<! I/O privilege level

typedef struct stack_frame_s {
  u32 gs;          //<! ━┓
  u32 fs;          //<!  ┃
  u32 es;          //<!  ┃
  u32 ds;          //<!  ┃
  u32 edi;         //<!  ┃
  u32 esi;         //<!  ┣━┫ push by `save`
  u32 ebp;         //<!  ┃
  u32 kernel_esp;  //<!  ┣ ignored by popad
  u32 ebx;         //<!  ┃
  u32 edx;         //<!  ┃
  u32 ecx;         //<!  ┃
  u32 eax;         //<! ━┫
  u32 retaddr;     //<!  ┣ retaddr for `save`
  u32 eip;         //<! ━┫
  u32 cs;          //<!  ┃
  u32 eflags;      //<!  ┣━┫ push by interrupt
  u32 esp;         //<!  ┃
  u32 ss;          //<! ━┛
} stack_frame_t;

typedef struct process {
  stack_frame_t regs;  //<! registers of process saved in stack frame
  pid_t pid;           //<! id of process
} process_t;

//! pointer to current running process
extern process_t *p_proc_ready;

//! global pcb table assigned to each process
extern process_t pcb_table[NR_PCBS];
