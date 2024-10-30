[SECTION .text]

extern main

    global _start
_start:
    call    main
    jmp	    $
