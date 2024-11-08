#pragma once

#include <layout.h>
#include <macro_helper.h>
#include <stdint.h>
#include <sys/types.h>

#define NR_PCBS 20

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
#define EFLAGS_RESERVED 0x00000002             //<! always 1 in eflags
#define EFLAGS_CF 0x00000001                   //<! carry flag
#define EFLAGS_IF 0x00000200                   //<! interrupt enable flag
#define EFLAGS_IOPL(pl) (((pl) & 0b11) << 12)  //<! I/O privilege level
#define EFLAGS_PF 0x00000004                   //<! parity flag
#define EFLAGS_AF 0x00000010                   //<! auxiliary carry flag
#define EFLAGS_ZF 0x00000040                   //<! zero flag
#define EFLAGS_SF 0x00000080                   //<! sign flag
#define EFLAGS_TF 0x00000100                   //<! trap flag
#define EFLAGS_IF 0x00000200                   //<! interrupt flag
#define EFLAGS_DF 0x00000400                   //<! direction flag
#define EFLAGS_OF 0x00000800                   //<! overflow flag
#define EFLAGS_NT 0x00004000                   //<! nested task
#define EFLAGS_RF 0x00010000                   //<! resume flag
#define EFLAGS_VM 0x00020000                   //<! virtual 8086 mode
#define EFLAGS_AC 0x00040000                   //<! alignment check
#define EFLAGS_VIF 0x00080000                  //<! virtual interrupt flag
#define EFLAGS_VIP 0x00100000                  //<! virtual interrupt pending
#define EFLAGS_ID 0x00200000                   //<! id flag

//! cr0 register flags
#define CR0_PE 0x00000001  //<! protection enable
#define CR0_MP 0x00000002  //<! monitor coprocessor
#define CR0_EM 0x00000004  //<! emulation
#define CR0_TS 0x00000008  //<! task switched
#define CR0_ET 0x00000010  //<! extension type
#define CR0_NE 0x00000020  //<! numeric errror
#define CR0_WP 0x00010000  //<! write protect
#define CR0_AM 0x00040000  //<! alignment mask
#define CR0_NW 0x20000000  //<! not writethrough
#define CR0_CD 0x40000000  //<! cache disable
#define CR0_PG 0x80000000  //<! paging

//! cr4 register flags
#define CR4_PCE 0x00000100  //<! performance counter enable
#define CR4_MCE 0x00000040  //<! machine check enable
#define CR4_PSE 0x00000010  //<! page size extensions
#define CR4_DE 0x00000008   //<! debugging extensions
#define CR4_TSD 0x00000004  //<! time stamp disable
#define CR4_PVI 0x00000002  //<! protect mode virtual interrupts
#define CR4_VME 0x00000001  //<! v86 mode extensions

//! helpful macros to declare eflags, e.g. EFLAGS(IF, IOPL(1))
#define EFLAGS_IMPL1(x, ...) MH_EXPAND(MH_CONCAT(EFLAGS_, x))
#define EFLAGS_IMPL2(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL1(_, __VA_ARGS__))
#define EFLAGS_IMPL3(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL2(_, __VA_ARGS__))
#define EFLAGS_IMPL4(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL3(_, __VA_ARGS__))
#define EFLAGS_IMPL5(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL4(_, __VA_ARGS__))
#define EFLAGS_IMPL6(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL5(_, __VA_ARGS__))
#define EFLAGS_IMPL7(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL6(_, __VA_ARGS__))
#define EFLAGS_IMPL8(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL7(_, __VA_ARGS__))
#define EFLAGS_IMPL9(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL8(_, __VA_ARGS__))
#define EFLAGS_IMPL10(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL9(_, __VA_ARGS__))
#define EFLAGS_IMPL11(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL10(_, __VA_ARGS__))
#define EFLAGS_IMPL12(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL11(_, __VA_ARGS__))
#define EFLAGS_IMPL13(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL12(_, __VA_ARGS__))
#define EFLAGS_IMPL14(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL13(_, __VA_ARGS__))
#define EFLAGS_IMPL15(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL14(_, __VA_ARGS__))
#define EFLAGS_IMPL16(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL15(_, __VA_ARGS__))
#define EFLAGS_IMPL17(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL16(_, __VA_ARGS__))
#define EFLAGS_IMPL18(x, _, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_, x)) | MH_EXPAND(EFLAGS_IMPL17(_, __VA_ARGS__))
#define EFLAGS_IMPL(N, x, ...) \
  MH_EXPAND(MH_CONCAT(EFLAGS_IMPL, N)(x, __VA_ARGS__))
#define EFLAGS_WRAPPER(...) \
  MH_EXPAND(EFLAGS_IMPL(MH_EXPAND(MH_NARG(__VA_ARGS__)), __VA_ARGS__))
#define EFLAGS(...) (EFLAGS_WRAPPER(RESERVED, __VA_ARGS__))

#define KERNEL_STACK_SIZE (8 * NUM_1K)

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

typedef struct kernel_context {
  u32 eflags;
  u32 edi;
  u32 esi;
  u32 ebp;
  u32 ebx;
  u32 edx;
  u32 ecx;
  u32 eax;
  u32 esp;
} kernel_context_t;

//! \brief process status values
enum {
  PSTATUS_IDLE,      //<! available pcb slot
  PSTATUS_READY,     //<! running or ready to run
  PSTATUS_SLEEPING,  //<! sleeping
  PSTATUS_ZOMBIE,    //<! killed but not recycled
  PSTATUS_INITING,   //<! assigned but still on init
};

struct pcb;

/*!
 * \brief node of all assigned phy pages for a process
 * \note addr is set to 0xffffffff for the page dir and page table
 */
typedef struct page_node {
  struct page_node *next;
  phyaddr_t phy_addr;
  uintptr_t addr;
} page_node_t;

//! \brief process inheritance tree - son
typedef struct son_node {
  struct son_node *prev;
  struct son_node *next;
  struct pcb *son;
} son_node_t;

//! \brief process inheritance tree - node
typedef struct tree_node {
  struct pcb *fa;
  son_node_t *sons;
} tree_node_t;

typedef struct pcb {
  stack_frame_t user_regs;
  kernel_context_t kernel_regs;
  u32 lock;
  int status;
  u32 pid;
  int exit_code;
  phyaddr_t cr3;
  int priority;
  int ticks;
  page_node_t *pages;
  tree_node_t ptree;
} pcb_t;

typedef struct process {
  union {
    pcb_t pcb;
    u8 padding[KERNEL_STACK_SIZE];
  };
  u32 kernel_stack_bottom[0];
} process_t;

//! pointer to current running process
extern process_t *p_proc_ready;

//! global pcb table assigned to each process
extern process_t pcb_table[NR_PCBS];

pid_t get_pid(process_t *p_proc);

void restart();
void switch_kernel_context(kernel_context_t *current, kernel_context_t *next);

void init_segment_regs(pcb_t *pcb);
void init_pagetbl(pcb_t *pcb);
