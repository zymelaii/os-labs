%include "layout.inc"
%include "mbr.inc"
%include "packet.inc"
%include "msg_helper.inc"

    org     OffsetOfMBR
Start:
    ; init seg reg
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ss, ax

    ; copy self to BaseOfMBR:OffsetOfMBR since 0x7c00 will be overwritten by
    ; boot sector later
    cld
    mov     cx, SizeOfSector
    mov     si, OffsetOfBoot
    mov     di, OffsetOfMBR
    rep movsb

    jmp     BaseOfMBR:Init

; variales
DriverNumber db 0 ; driver number

; declare messages and introduce ShowMessage
MSG_BEGIN 11
    NewMessage FindPart,     "Find Part  "
    NewMessage Ready,        "Ready      "
    NewMessage FailedToRead, "Read Fail  "
    NewMessage NoBootable,   "No Bootable"
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

Init:
    ; use temporary stack for fn call
    mov     sp, OffsetOfMBR

    ; backup driver number
    mov     [DriverNumber], dl

    ; clear screen for a better view of messages
    mov     ax, 0x0003
    int     10h

    mov     dh, MSG_FindPart
    call    ShowMessage

    ; TODO: search for bootable partition in mbr partition table

    mov     dh, MSG_NoBootable
    call    ShowMessage
    jmp     $

    mov     dh, MSG_Ready
    call    ShowMessage

    ; TODO: load boot sector to BaseOfBoot:OffsetOfBoot
    ; TODO: maybe the bootable partition is not a valid boot sector, why not check it?

    ; TODO: transfer info of driver number and partition to boot

    jmp     BaseOfBoot:OffsetOfBoot

    times   SizeOfMBR - ($-$$) db 0
