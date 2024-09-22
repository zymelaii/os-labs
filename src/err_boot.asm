    org     0x7c00              ; 告诉编译器程序加载到 7c00 处

    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    call    DispStr             ; 调用显示字符串例程
    jmp     $                   ; 无限循环

DispStr:
    mov     ax, BootMessage
    mov     bp, ax              ; ES:BP = 串地址

    ; TODO: 参考文档，在此处填充寄存器传参

    int     10h                 ; 10h 号中断
    ret

BootMessage:
    db      "Hello, OS world!"
    times   510-($-$$) db 0     ; 填充剩下的空间，使生成的二进制代码恰好为 512 字节
    dw      0x55aa              ; 结束标志
