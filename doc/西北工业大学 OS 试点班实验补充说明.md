<div align="center">
    <font size="6">西北工业大学 OS 试点班实验补充说明</font>
    <div align="right">
    <table>
        <tr>
            <td style="border:none;">作者</td>
            <td align="center" style="border:none;">项乔栋</td>
        </tr>
        <tr>
            <td style="border:none;">版本</td>
            <td align="center" style="border:none;">v0.1 (20240922)</td>
        </tr>
    </table>
    </div>
</div>

# 总览

- 编译的基本要求
- 如何制作系统盘
- QEMU 启动
- 调试方法
- 关于汇编的补充

# 编译的基本要求

操作系统实验在 QEMU i386 的 system 模式下运行，在该模式下，QEMU 将对整个系统进行仿真，包括 CPU、内存、磁盘等。

对于你提供给 QEMU 的二进制文件，其自身必须包含能够支持其运行的全部内容，包括 bootloader、kernel、system-libs 等。

或者用并不准确但足够直白的话来说：不要动态链接！

在以上基本前提下，你可以同平日一样使用 gcc 编译你的代码。

编译示例：

**单文件：C 源码**

```shell
gcc -m32 -static -fno-pie -o target.bin source.c
```

参数说明：

- **-m32** 指定编译为 32 位程序
- **-fno-pie** 禁用地址无关代码
- **-static** 编译静态程序

**单文件：汇编源码**

本实验使用 Intel 格式的 x86 汇编，需要搭配使用 nasm 进行编译。

```shell
nasm -o target.bin source.asm
```

**多文件混编**

```shell
gcc -m32 -static -fno-pie -c -o obj1.o source.c
nasm -f elf -o obj2.o source.asm
ld -m elf_i386 -T linker.ld --oformat binary -o target.bin obj1.o obj2.o
```

其中 linker.ld 为 ld 链接脚本，用于精准把控链接的产出文件。

详细可自行阅读 ld linker script 的相关文档或分析后续实验中将会提供的 linker.ld 脚本。

**推荐追加的 GCC 参数**

- **-Wall** 打开所有警告
- **-Werror** 报错时停止编译
- **-g** 编译生成调试信息
- **-O0** 关闭优化（保证代码语义，便于调试）
- **-gnu99** 使用 GNU C99 语言标准
- **-fno-stack-protector** 禁用栈溢出保护机制
- **-MD** 编译的同时产出 \*.d 依赖指示文件（可用于 Makefile 判断目标是否脏）

# 如何制作系统盘

制作系统盘实质上是要回答系统盘的结构是怎样的问题。

本实验中涉及从软盘启动与从硬盘启动两种方式，见下。

> 在之后的实验中，均会给出制作系统盘的脚本，故此处仅作简要叙述

## 制作软盘启动盘

软盘，也即 floppy，通常为 FAT12 的文件系统格式，容量 1.44MB。

零位置开始的 512 字节（即 1 扇区）为引导扇区，你需要将你的 boot 程序写入此处，剩余的部分则视情况自行处理。

如你的 boot 程序将引导控制流转交给 loader.bin 且需要依赖到 info.txt 文件，则你或许应当挂载其文件系统并将其拷贝入软盘。

假定你的 boot 程序为 boot.bin，则软盘的制作方法如下：

```shell
dd if=/dev/zero of=floppy.img bs=512 count=2880
mkfs -t vfat floppy.img
dd if=boot.bin of=floppy.img bs=512 count=1 conv=notrunc
sudo mount -o loop floppy.img /mnt
sudo cp loader.bin /mnt
sudo cp info.txt /mnt
sudo umount /mnt
```

步骤简述：

1. 使用 dd 命令创建 1.44MB 的空软盘文件 floppy.img
2. 使用 mkfs.vfat 命令格式化为 FAT12 文件系统
3. boot.bin 写入扇区 0
4. 将 floppy.img 视为 fs 挂载到 /mnt
5. 将 loader.bin 及 info.txt 拷贝到软盘
6. 卸载 floppy.img 挂载

## 制作硬盘启动盘

制作硬盘的思路与软盘类似，其主要的差别在于分区表的存在。

主流分区表为 MBR 与 GPT，本实验使用 MBR 格式。

MBR 由 446 字节的引导代码、64 字节的分区表、2 字节的 0x55AA 签名组成。

> 此处引导代码指代 MBR 的引导程序，由于其实在太小，故通常作为 boot 的第一阶段用于启动 bootloader

创建任意大小的空磁盘文件，容量任意。

> 创建的磁盘容量需小于 2TB，2TB 以上需要使用 GPT

创建后，可使用工具 sfdisk 对磁盘进行分区。

> 有需要的请自行了解 sfdisk 工具的用法

使用 losetup 创建环路设备并将磁盘挂载后，可通过 `/dev/loop0p${n}` 访问对应分区，如 `/dev/loop0p1` 为第一个分区，`/dev/loop0p3` 为第三个分区。

> 此处假定创建得到的环路设备为 /dev/loop0
>
> 环路设备槽是有限的，使用完后记得使用 losetup -d /dev/loop0 释放

sfdisk 未提供格式化分区的功能，故完成分区后，你仍然需要手动对各个分区进行文件系统的格式化（同软盘，使用何种文件系统自定）。

以上便是制作硬盘的全过程，不过当然，此时其还未必是一个启动盘/存在可引导分区。

可引导分区在使用 sfdisk 时通过指定 bootable 标志完成，假定你指定了分区 1 为可引导分区，则你应当向其写入启动所需要的全部内容。

需要注意的是，在写入分区扇区 0 的 boot 程序时，你应该关注该分区建立的 fs。如 FAT12 包含 62 字节的 fs 元信息，为了不破坏该信息，你实际上应当从 62 字节处写入，并应当确保 boot 程序始终处于 0 扇区内。

> 关于该部分的详细介绍可参考实验 3 的相关文档

# QEMU 启动

**从软盘启动**

```shell
qemu-system-i386 -fda floppy.img
```

**从硬盘启动**

```shell
qemu-system-i386 -hda disk.img
```

**或者**

```shell
qemu-system-i386 -boot c -drive file=disk.img,format=raw
```

以上三种启动方式简单地指定了喂给 QEMU 的驱动器，你可以多次指定 `-drive`，QEMU 默认将按照给出的 drive 的顺序尝试启动。

QEMU 中还有很多好用的启动参数，你可以通过 --help 详细了解。

1. -monitor stdio 开启串口监视器，便于调试
2. -serial file:serial.log 开启串口日志记录
3. -display curses 使用 curses 为显示器（直接在终端中显示）
4. -nographic 禁用图形输出
5. -S 启动后，暂停在 BIOS 程序的第一条指令
6. -s 参数别名，等同于 `-gdb tcp::1234`
7. -gdb tcp::\<port> 指定开放 GDB 端口 \<port> 以供调试

> 使用 curses 显示器时，结束 QEMU 会成为一件困难的事，以下提供三种退出 QEMU 的方法：
>
> 1. alt+2 进入到 QEMU console，使用 quit 命令退出
> 2. 开放 gdb 端口，gdb 连接后，使用 kill 命令结束 QEMU
> 3. 另开终端获取 QEMU 的 pid，使用 kill 强杀
>
> 除外，既然你选择了 curses，或许可以尝试搭配 tmux 在单个终端窗口同时完成编码、调试、仿真运行三项工作。

# 调试方法

QEMU 携带 -S 参数启动后，启动 GDB，使用 `target remote localhost:1234` 连接到 QEMU。

> 可视情况决定是否为 QEMU 携带 -s 参数
>
> 对于 GDB，可以携带 `-ex <command>` 直接在启动时执行命令，如 `gdb -ex target remote localhost:1234`

以下给出关于 GDB 常用命令的使用，详细内容请自行查阅 GDB 官方文档。

**基础命令**

```shell
si             # 单步步入下一条指令
ni             # 单步步过下一条指令
b *0x7c00      # 在地址 7c00 处打断点
c              # 执行直到遇到一个断点
p $pc          # 打印寄存器 pc
disp $ah       # 显示寄存器 ah，每一步执行打印一次
info registers # 打印所有寄存器值
q              # 退出调试（虚拟机需要手动退出）
```

**推荐命令**

```shell
disass 0x7c00,0x7e00 # 反汇编地址段 [0x7c00, 0x7e00) 的所有数据（刚好是 boot 程序）
x /10i $pc           # 打印 pc 后的 10 条命令
x /16x 0x7c00        # 打印 0x7c00 后的 16 个字节数据（十六进制）
disp /10i $pc        # 显示 pc 后的 10 条命令，每一步执行打印一次
layout asm           # 显示汇编窗口
layout regs          # 显示寄存器窗口
```

# 关于汇编的补充

对于 x86，GCC 套件及 GDB 反汇编的默认汇编表示为 AT&T 格式，而在实验中我们使用的是 Intel 格式，该项差异或许会对大家实验的进行产生一定的困扰。

对于 GDB，请使用 `set disassembly-flavor intel` 切换至 Intel 格式。

对于 objdump，请使用 `-M intel` 参数切换至 Intel 格式。

若需要使用 objdump 反汇编非 elf 格式的 raw binary，可以搭配 `-b` `-m` 分别指定文件格式与架构，如：

```shell
objdump -b binary -m i386 -D target.bin
```
