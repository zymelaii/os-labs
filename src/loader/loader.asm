; MEMORY LAYOUT
; ===
; 0xc0300000               kernel start linear address
; 0x03200000~              buffer for reading kernel elf segment
; 0x03100000~              kernel elf file
; 0x03000000~              fat info/data in the current partition
; 0x00300000               kernel start physical address
; ---
; 0x00201000~              page table
; 0x00200000~0x00201000    page directory
; ---
; 0x000f0000~              [STABLE] system rom
; 0x000e0000~0x000f0000    [STABLE] expansion of system rom
; 0x000c0000~0x000e0000    [STABLE] reserved for rom expansion
; 0x000b8000               base of video memory for gs
; 0x000a0000~0x000c0000    [STABLE] reserved for display adapter
; 0x0009fc00~0x000a0000    [STABLE] extended bios data area
; 0x0009fc00~0x00100000    [STABLE] reserved for hardware
; ---
; 0x00090400~              loader file
;           ~0x00090400    stack for loader in real mode
; ---
; 0x00007e00~              buffer used to read sectors
; 0x00007c00~0x00007e00    [STABLE] boot sector
; 0x00007c00~0x00007e00    [STABLE] mbr, moved to other place before booting
; 0x00007c00~0x00007c5a    fat32 bpb
;           ~0x00007c00    stack for boot
; ---
; 0x00000800~0x00000a00    a copy of mbr
; ---
; 0x00000400~0x00000500    [STABLE] rom bios parameter block
; 0x00000000~0x00000400    [STABLE] interrupt vectors
; ===

%include "layout.inc"
%include "config.inc"
%include "fat32.inc"
%include "packet.inc"
%include "protect.inc"
%include "paging.inc"

[SECTION .text.s16]
[BITS 16]

align 16

    global _start
_start:
    jmp     Main

; variables
DriverNumber       db 0 ; current driver number
DummyPartitionLBA  dd 0 ; current partition start LBA

align 4

; layout of our gdt specified in loader:
;
; ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━┓
; ┃ Descriptors         ┃ Selectors             ┃
; ┣━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━┫
; ┃ RESVERD             ┃                       ┃
; ┣━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━┫
; ┃ DESC_FLAT_C 0~4G    ┃ 0x08 = cs             ┃ <- 0x00000000
; ┣━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━┫
; ┃ DESC_FLAT_RW 0~4G   ┃ 0x10 = ds, es, fs, ss ┃ <- 0x00000000
; ┣━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━┫
; ┃ DESC_VIDEO          ┃ 0x1b = gs             ┃ <- 0x000b8000
; ┗━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━┛

; NOTE: entry 0 should always be null and subsequent entries should be used instead
LABEL_GDT:          times 8 db 0
LABEL_DESC_FLAT_C:  Descriptor 0x00000000, 0xfffff, DA_DPL0 | DA_CR  | DA_32 | DA_LIMIT_4K ; 0 ~ 4G
LABEL_DESC_FLAT_RW: Descriptor 0x00000000, 0xfffff, DA_DPL0 | DA_DRW | DA_32 | DA_LIMIT_4K ; 0 ~ 4G
LABEL_DESC_VIDEO:   Descriptor 0x000b8000, 0x0ffff, DA_DPL3 | DA_DRW                       ; video memory

GdtLen equ $ - LABEL_GDT - 1

GdtPtr: dw GdtLen                    ; limit
        dd PhyLoaderBase + LABEL_GDT ; base addr

; selector = [13 bits index] << 3 | [1 bits TI tag] << 2 | [2 bits RPL tag] << 0
SelectorFlatC  equ (LABEL_DESC_FLAT_C  - LABEL_GDT) | SA_TIG | SA_RPL0
SelectorFlatRW equ (LABEL_DESC_FLAT_RW - LABEL_GDT) | SA_TIG | SA_RPL0
SelectorVideo  equ (LABEL_DESC_VIDEO   - LABEL_GDT) | SA_TIG | SA_RPL3

align 16

Main:
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, OffsetOfLoader

    ; save boot info from boot
    mov     [DriverNumber], dl
    mov     [DummyPartitionLBA], ebx

    ; clear screen and hide cursor for a better view of messages
    mov     ax, 0x0003
    int     10h
    mov     ah, 0x01
    mov     ch, 0b00100000
    int     10h

    ; load gdt
    lgdt    [GdtPtr]

    ; turn off interrupts
    cli

    ; enable A20 gate
    in      al, 0x92
    or      al, 0b00000010
    out     0x92, al

    ; set PE flag in cr0 and prepare to jump into protect mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; use a long jump to switch to protect mode, after the jump, the processor
    ; will set cs to 0x8, i.e. selector LABEL_DESC_FLAT_C
    jmp     dword SelectorFlatC:ProtectMain

[SECTION .text]
[BITS 32]

align 32

extern PartitionLBA

extern load_kernel

ProtectMain:
    mov     ebx, [DummyPartitionLBA]

    ; reset seg regs
    mov     ax, SelectorVideo
    mov     gs, ax
    mov     ax, SelectorFlatRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax

    mov     esp, StackTop

    mov     [PartitionLBA], ebx

    ; enable page table
    call    SetupPaging

    ; NOTE: you're not required to make insight of how it works currently, but
    ; only to know that it allows you to access a memory space till 4G

    ; transfer control to kernel
    jmp     SelectorFlatC:load_kernel

SetupPaging:
    ; compute required num of page table and PDE
    ; NOTE: IDT is not setup yet, so assume that qemu provide memory of 128M at here
    mov     eax, 0x8000000
    xor     edx, edx
    mov     ebx, 0x400000       ; 4M per page table
    div     ebx
    mov     ecx, eax            ; ecx <- num of page table, i.e pde
    test    edx, edx
    jz      .done
    inc     ecx                 ; align to size of page table
.done:
    push    ecx                 ; total pdes of 128M

    ; NOTE: maps addrs to phy addrs linearly and equivalently, regardless of memory holes for convenience
    ; init page dir
    mov     ax, SelectorFlatRW
    mov     es, ax
    xor     eax, eax
    mov     eax, PhyPageTableBase | PG_P | PG_USU | PG_RWW

    ; NOTE: make page tables contiguous in memory for convenience

    ; fill 0~128M pde
    mov     edi, PhyPageDirBase
.pde.loop.1:
    stosd
    add     eax, 0x1000
    loop    .pde.loop.1

    ; fill 3G~3G+128M pde
    mov     edi, PhyPageDirBase + 0xc00
    mov     ecx, [esp]
.pde.loop.2:
    stosd
    add     eax, 0x1000
    loop    .pde.loop.2

.init:
    ; init all page tables
    pop     eax
    mov     ebx, 1024
    mul     ebx
    mov     ecx, eax            ; total ptes of 128M
    push    ecx

    xor     eax, eax
    mov     eax, PG_P | PG_USU | PG_RWW
    push    eax

    ; map 0~128M to 0~128M
    mov     edi, PhyPageTableBase
.pte.loop.1:
    stosd
    add     eax, 0x1000
    loop    .pte.loop.1

    ; map 3G~3G+128M to 0~128M
    pop     eax
    pop     ecx
.pte.loop.2:
    stosd
    add     eax, 0x1000
    loop    .pte.loop.2

.complete:
    ; update cr3 to page dir base
    mov     eax, PhyPageDirBase
    mov     cr3, eax
    ; enable paging
    mov     eax, cr0
    or      eax, 0x80000000
    mov     cr0, eax
    ret

[SECTION .data]
[BITS 32]

align 32

; allocate 4KB stack
times 4096 db 0
StackTop:
