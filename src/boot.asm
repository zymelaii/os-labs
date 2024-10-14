%include "consts.inc"

MsgLen       equ 8    ; fixed length of embedded message
DriverNumber equ 0x80 ; boot from first hard disk

; NOTE: 0x00~0x5a is reserved for BPB and is initialized on mkfs fat32
; ATTENTION: BPB will also be loaded into memory at 0x00~0x5a, and you can see access to BPB_* field in the code below

    org     OffsetOfBootMain
Start:
    ; init seg regs
    mov     ax, cs
    mov     ds, ax
    mov     ss, ax
    mov     es, ax

    jmp     Init

; variables
EndOfBuffer   dw 0 ; end of boot buffer; global constant for traversal of fat entries to find loader
SecOfDataZone dd 0 ; base sector of data zone in fat32; global constant for method ReadCluster

; rodata
LoaderName db "LOADER  BIN" ; fat32 short file name of our loader file
                            ; 8 bytes base name + 3 bytes extension

%macro MSG_BEGIN 0
OffsetOfEmbeddedMsg:
%endmacro

%macro MSG_END 0
%endmacro

%macro NewMessage 3
    MSG_%2  equ %1
    _MSG_%2 db %3
%endmacro

MSG_BEGIN
    NewMessage 0, Booting,        "Booting "
    NewMessage 1, Ready,          "Ready   "
    NewMessage 2, FailedToRead,   "ReadFail"
    NewMessage 3, LoaderNotFound, "NoLoader"
MSG_END

; \brief show message on screen
; \param [in] dh index of the specified embedded message
; \note use msg name instead of literal index, e.g. MSG_FailedToRead
ShowMessage:
    pusha
    push    es

    ; compute the address of the msg at index dh
    mov     ax, MsgLen
    mul     dh
    add     ax, OffsetOfEmbeddedMsg

    mov     bp, ax
    mov     ax, ds
    mov     es, ax
    mov     cx, MsgLen
    mov     ax, 0x1301
    mov     bx, 0x0007
    mov     dl, 0
    int     10h

    ; you may notice that we don't specify the row number for the int call,
    ; and that's why dh is used as the message index here, it also refers to
    ; the output row in the screen

    pop     es
    popa
    ret

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
    mov     dword [si + Packet_StartSectors], eax
    mov     dword [si + Packet_StartSectors + 4], 0

    mov     dl, DriverNumber
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

Init:
    ; use temporary stack for fn call
    mov     sp, OffsetOfBoot

    ; clear screen for a better view of messages
    mov     ax, 0x0003
    int     10h

    mov     dh, MSG_Booting
    call    ShowMessage

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

    ; test current dir entry for loader name
    mov     di, bx
.find_loader:
    push    di
    mov     si, LoaderName
    mov     ecx, 11
    repe    cmpsb
    jz      .found

    ; shift to next dir entry
    pop     di
    add     di, SizeOfDIR
    cmp     di, [EndOfBuffer]
    jnz     .find_loader

    ; otherwise the cluster has been fully searched, go to the next cluster
    call    NextCluster
    cmp     eax, CLUSTER_Last
    jnz     .load_dir_cluster

.fail:
    mov     dh, MSG_LoaderNotFound
    call    ShowMessage
    jmp     $

    ; for the following code, let:
    ; * es  <- seg base of loader
    ; * bx  <- offset of loader
    ; * eax <- cluster number of loader
.found:
    movzx   ax, byte [BPB_SecPerClus]
    mul     word [BPB_BytsPerSec]
    mov     dx, ax

    pop     di
    mov     ax, [es:di + DIR_FstClusHI]
    shl     eax, 8
    mov     ax, [es:di + DIR_FstClusLO]

    ; prepare params for ReadCluster
    mov     cx, BaseOfLoader
    mov     es, cx
    mov     bx, OffsetOfLoader
    movzx   cx, byte [BPB_SecPerClus]

    ; a loop procedure to load the entire loader to its destination
.load_loader:
    call    ReadCluster
    add     bx, dx
    call    NextCluster
    cmp     eax, CLUSTER_Last
    jnz     .load_loader

    mov     dh, MSG_Ready
    call    ShowMessage

    ; long jump to the loader address
    ; after this you will no longer be able to see assembly code in gdb's asm
    ; layout, but `x` or `disass` command will still work
    jmp     BaseOfLoader:OffsetOfLoader

    times   SizeOfBoot - ($-$$) db 0
