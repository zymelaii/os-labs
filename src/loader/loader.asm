; MEMORY LAYOUT
; ===
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
; 0x00090000~0x0009005a    cloned fat32 bpb for loader
;           ~0x00090400    stack for loader in real mode
; ---
; 0x00080000~              kernel elf file, the kernel itself must be loaded
;                          into memory at the virtual address specified in the
;                          elf before entering
; 0x00030400               kernel entry point
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
%include "msg_helper.inc"
%include "paging.inc"

[SECTION .text.s16]
[BITS 16]

align 16

    global _start
_start:
    ; clone fat32 bpb to BaseOfLoader:0
    mov     ax, BaseOfBoot
    mov     ds, ax
    mov     ax, BaseOfLoader
    mov     es, ax
    cld
    mov     cx, SizeOfBPB
    mov     si, OffsetOfBoot
    mov     di, 0
    rep movsb

    ; ATTENTION: bpb can be overwritten by stack in extreme cases

    jmp     Main

; variables
DriverNumber  db 0 ; current driver number; global constant for method ReadSector
EndOfBuffer   dw 0 ; end of boot buffer; global constant for traversal of fat entries to find loader
SecOfDataZone dd 0 ; base sector of data zone in fat32; global constant for method ReadCluster
PartitionLBA  dd 0 ; current partition start LBA; global constant used in method ReadSector

; kernel fat name
DeclareKernelFAT KernelName

; declare messages and introduce ShowMessage
MSG_BEGIN 25
    NewMessage EnterLoader,    "begin your adventure!    "
    NewMessage FoundKernel,    "cheers! you've got me!   "
    NewMessage KernelNotFound, "show me the kernel, plz! "
    NewMessage FailedToRead,   "you're kidding me, right?"
MSG_END

; \brief read sectors from disk and write to the buffer
; \param [in] eax start sector index
; \param [in] cx total sectors to read
; \param [out] es:bx buffer address
ReadSector:
    pushad
    sub     sp, SizeOfPacket    ; allocate packet on stack

    ; now si points to packet and we can fill it with expected data, see lab guide for more details
    mov     si, sp
    mov     word [si + Packet_BufferPacketSize], SizeOfPacket
    mov     word [si + Packet_Sectors], cx
    mov     word [si + Packet_BufferOffset], bx
    mov     word [si + Packet_BufferSegment], es
    add     eax, [PartitionLBA]
    mov     dword [si + Packet_StartSectors], eax
    mov     dword [si + Packet_StartSectors + 4], 0

    mov     dl, [DriverNumber]
    mov     ah, 42h
    int     13h
    jc      .fail               ; cf=1 if read error occurs, and we assume that the bios is broken

    add     sp, SizeOfPacket    ; free packet
    popad
    ret

.fail:
    mov     dh, MSG_FailedToRead
    call    ShowMessage
    jmp     $                   ; well, simply halt then, and you should check your env or program before next try

; \brief read the specified cluster from disk and write to the buffer
; \param [in] eax cluster number
; \param [out] es:bx buffer address
ReadCluster:
    pushad

    ; compute the sector index of the cluster
    ; * ecx <- total sectors of one cluster
    ; * eax <- start sector index of the cluster
    sub     eax, CLUSTER_Base   ; first 2 clusters are reserved, compute relative cluster index to the root dir
    movzx   ecx, byte [BPB_SecPerClus]
    mul     ecx
    add     eax, [SecOfDataZone]

    call    ReadSector

    popad
    ret

; \brief get next cluster number
; \param [in] eax current cluster number
; \param [out] eax next cluster number
; \note the boot buffer will be modified since the method needs to read sectors
NextCluster:
    pushad
    push    es

    mov     cx, BaseOfBootBuffer
    mov     es, cx

    ; compute start sector of the cluster
    ; start-sector = cluster * 4 / bytesPerSec + rsvdSecCnt
    shl     eax, 2
    mov     di, ax
    shr     eax, 9
    movzx   ecx, word [BPB_RsvdSecCnt]
    add     eax, ecx

    mov     cx, 1
    mov     bx, OffsetOfBootBuffer
    call    ReadSector

    ; compute offset to cluster in the sector
    ; offset = cluster * 4 % bytesPerSec
    and     di, 0x1ff

    mov     eax, [es:di + bx]
    and     eax, CLUSTER_Mask

    ; here popad will restore the eax and overwite our expected next-cluster,
    ; so manually save it to the stack-pos of old eax to avoid being flushed
    pop     es
    mov     bx, sp
    mov     [bx + 28], eax

    popad
    ret

LoadKernelFile:
    pushad
    push    es

    ; compute end of buffer for the termination condition of the find-fat-entry loop
    movzx   ax, byte [BPB_SecPerClus]
    mul     word [BPB_BytsPerSec]
    add     ax, OffsetOfBootBuffer
    mov     [EndOfBuffer], ax

    ; compute start sector of data zone in fat32
    movzx   eax, byte [BPB_NumFATs]
    mul     dword [BPB_FATSz32]
    movzx   edx, word [BPB_RsvdSecCnt]
    add     eax, edx
    mov     [SecOfDataZone], eax

    ; for the following code, let:
    ; * es  <- seg base of boot buffer
    ; * eax <- current cluster number
    ; * di  <- fat dir entry offset 0, also points to the DIR_Name field

    mov     ax, BaseOfBootBuffer
    mov     es, ax
    mov     eax, [BPB_RootClus]

.load_dir_cluster:
    mov     bx, OffsetOfBootBuffer
    call    ReadCluster

    ; test current dir entry with kernel name
    mov     di, bx
.find:
    push    di
    mov     si, KernelName
    mov     ecx, 11
    repe    cmpsb
    jz      .found

    ; shift to next dir entry
    pop     di
    add     di, SizeOfDIR
    cmp     di, [EndOfBuffer]
    jnz     .find

    ; otherwise the cluster has been fully searched, go to the next cluster
    call    NextCluster
    cmp     eax, CLUSTER_Last
    jnz     .load_dir_cluster

.fail:
    mov     dh, MSG_KernelNotFound
    call    ShowMessage

    ; for the following code, let:
    ; * es  <- seg base of kernel
    ; * bx  <- offset of kernel
    ; * eax <- cluster number of kernel
.found:
    movzx   ax, byte [BPB_SecPerClus]
    mul     word [BPB_BytsPerSec]
    mov     dx, ax

    pop     di
    mov     ax, [es:di + DIR_FstClusHI]
    shl     eax, 8
    mov     ax, [es:di + DIR_FstClusLO]

    ; prepare params for ReadCluster
    mov     cx, BaseOfKernel
    mov     es, cx
    mov     bx, OffsetOfKernel
    movzx   cx, byte [BPB_SecPerClus]

    ; a loop procedure to load the entire kernel to its destination
.load:
    call    ReadCluster
    add     bx, dx
    call    NextCluster
    cmp     eax, CLUSTER_Last
    jnz     .load

    mov     dh, MSG_FoundKernel
    call    ShowMessage

    pop     es
    popad
    ret

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
    mov     [PartitionLBA], ebx

    ; clear screen and hide cursor for a better view of messages
    mov     ax, 0x0003
    int     10h
    mov     ah, 0x01
    mov     ch, 0b00100000
    int     10h

    mov     dh, MSG_EnterLoader
    call    ShowMessage

    ; search and load kernel file
    call    LoadKernelFile

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

extern load_kernel

ProtectMain:
    ; reset seg regs
    mov     ax, SelectorVideo
    mov     gs, ax
    mov     ax, SelectorFlatRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax

    mov     esp, StackTop

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
    push  ecx
    ; NOTE: maps addrs to phy addrs linearly and equivalently, regardless of memory holes for convenience
    ; init page dir
    mov     ax, SelectorFlatRW
    mov     es, ax
    mov     edi, PhyPageDirBase
    xor     eax, eax
    mov     eax, PhyPageTableBase | PG_P | PG_USU | PG_RWW
 .loop.1:
    stosd
    ; NOTE: make page tables contiguous in memory for convenience
    add     eax, 0x1000
    loop    .loop.1
 .init:
    ; init all page tables
    pop     eax
    mov     ebx, 1024           ; 1024 pte per page table
    mul     ebx
    mov     ecx, eax            ; num of pte = num of page table * 1024
    mov     edi, PhyPageTableBase
    xor     eax, eax
    mov     eax, PG_P | PG_USU | PG_RWW
 .loop.2:
    stosd
    add     eax, 0x1000         ; 4K per page
    loop    .loop.2
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
