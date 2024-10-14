%macro MSG_BEGIN 1
MsgLen equ %1        ; fixed length of embedded message
OffsetOfEmbeddedMsg: ; base addr of embedded messages
%endmacro

%macro MSG_END 0
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
%endmacro

%macro NewMessage 2
    MSG_%1 equ ($ - OffsetOfEmbeddedMsg) / MsgLen
    db %2
%endmacro
