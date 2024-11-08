[SECTION .text]

extern main
extern exit

    global _start
_start:
    call    main
    push    eax
    call    exit

    ; ATTENTION: code below is unreachable
.1:
    hlt
    jmp     .1
