; memory layout
BaseOfBoot       equ 0x0000
BaseOfBootBuffer equ 0x0000
BaseOfLoader     equ 0x9000

SizeOfSector     equ 512
SizeOfBootBuffer equ SizeOfSector
SizeOfBPB        equ 90
SizeOfBoot       equ SizeOfSector - SizeOfBPB - 2

OffsetOfBoot       equ 0x7c00
OffsetOfBPB        equ OffsetOfBoot
OffsetOfBootBuffer equ OffsetOfBoot + SizeOfSector
OffsetOfBootMain   equ OffsetOfBPB + SizeOfBPB
OffsetOfLoader     equ 0x0400

; consts of packet used in bootloader
Packet_BufferPacketSize equ 0
Packet_Sectors          equ 2
Packet_BufferOffset     equ 4
Packet_BufferSegment    equ 6
Packet_StartSectors     equ 8
SizeOfPacket            equ 16

; fat32 BPB fields
BS_jmpBoot     equ OffsetOfBPB + 0
BS_OEMName     equ OffsetOfBPB + 3
BPB_BytsPerSec equ OffsetOfBPB + 11
BPB_SecPerClus equ OffsetOfBPB + 13
BPB_RsvdSecCnt equ OffsetOfBPB + 14
BPB_NumFATs    equ OffsetOfBPB + 16
BPB_RootEntCnt equ OffsetOfBPB + 17
BPB_TotSec16   equ OffsetOfBPB + 19
BPB_Media      equ OffsetOfBPB + 21
BPB_FATSz16    equ OffsetOfBPB + 22
BPB_SecPerTrk  equ OffsetOfBPB + 24
BPB_NumHeads   equ OffsetOfBPB + 26
BPB_HiddSec    equ OffsetOfBPB + 28
BPB_TotSec32   equ OffsetOfBPB + 32
BPB_FATSz32    equ OffsetOfBPB + 36
BPB_ExtFlags   equ OffsetOfBPB + 40
BPB_FSVer      equ OffsetOfBPB + 42
BPB_RootClus   equ OffsetOfBPB + 44
BPB_FSInfo     equ OffsetOfBPB + 48
BPB_BkBootSec  equ OffsetOfBPB + 50
BPB_Reserved   equ OffsetOfBPB + 52
BS_DrvNum      equ OffsetOfBPB + 64
BS_Reserved1   equ OffsetOfBPB + 65
BS_BootSig     equ OffsetOfBPB + 66
BS_VolID       equ OffsetOfBPB + 67
BS_VolLab      equ OffsetOfBPB + 71
BS_FilSysType  equ OffsetOfBPB + 82
EndOfBPB       equ OffsetOfBPB + 90

; fat32 consts
DIR_Name         equ 0
DIR_Attr         equ 11
DIR_NTRes        equ 12
DIR_CrtTimeTenth equ 13
DIR_CrtTime      equ 14
DIR_CrtDate      equ 16
DIR_LstAccDate   equ 18
DIR_FstClusHI    equ 20
DIR_WrtTime      equ 22
DIR_WrtDate      equ 24
DIR_FstClusLO    equ 26
DIR_FileSize     equ 28
SizeOfDIR        equ 32

CLUSTER_Free equ 0x00000000
CLUSTER_Base equ 0x00000002
CLUSTER_Bad  equ 0x0ffffff7
CLUSTER_Last equ 0x0fffffff
CLUSTER_Mask equ 0x0fffffff
