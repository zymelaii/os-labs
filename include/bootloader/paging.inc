; consts for paging
PG_P   equ 1 << 0 ; P   存在
PG_RWR equ 0 << 1 ; R/W 读/执行
PG_RWW equ 1 << 1 ; R/W 读/写/执行
PG_USS equ 0 << 2 ; U/S 系统级
PG_USU equ 1 << 2 ; U/S 用户级
