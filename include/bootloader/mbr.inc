; 0X000~0x1be    mbr boot code
; 0x1be~0x1fe    partition table
; 0x1fe~0x200    signagure 0x55aa

SizeOfMBRPartEnt equ 0x10
StartOfMBRTable  equ 0x1be
EndOfMBRTable    equ StartOfMBRTable + SizeOfMBRPartEnt * 4
SizeOfMBR        equ StartOfMBRTable

; struct of mbr partition entry
MBRPartEnt_State    equ 0x00
MBRPartEnt_FirstCHS equ 0x01
MBRPartEnt_Type     equ 0x04
MBRPartEnt_LastCHS  equ 0x05
MBRPartEnt_StartLBA equ 0x08
MBRPartEnt_Sectors  equ 0x0c
