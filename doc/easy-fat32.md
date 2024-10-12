<div align="center">
    <font size="6">Easy FAT32</font>
    <div align="right">
    <table>
        <tr>
            <td style="border:none;">作者</td>
            <td align="center" style="border:none;">管孙迪</td>
        </tr>
        <tr>
            <td style="border:none;">版本</td>
            <td align="center" style="border:none;">v0.2 (20241012)</td>
        </tr>
    </table>
    </div>
</div>

实验二中我们用到的磁盘镜像是一个 16MB 的硬盘（虽然在现实当中不存在这么小的硬盘）。我们可以通过 `file` 命令可以看到文件系统的相关信息：

```sh
$ file ./a.img
./a.img: DOS/MBR boot sector, code offset 0x58+2, OEM-ID "mkfs.fat", sectors/cluster 8, sectors 32768 (volumes <=32 MB), Media descriptor 0xf8, sectors/track 32, FAT (32 bit), sectors/FAT 32, serial number 0xef04294d, unlabeled
```

这种方式太难看了，所有的东西都堆一行，看都看麻了，不过有种方法就是通过 `tr` 命令将 `,` 替换为换行符，这样所有信息都能一行行输出了

```sh
$ file ./a.img | tr ',' '\n'
./a.img: DOS/MBR boot sector
 code offset 0x58+2
 OEM-ID "mkfs.fat"
 sectors/cluster 8
 sectors 32768 (volumes <=32 MB)
 Media descriptor 0xf8
 sectors/track 32
 FAT (32 bit)
 sectors/FAT 32
 serial number 0xef04294d
 unlabeled
```

虽然这是一个非标准镜像，但是可以看到一共有 32768 个扇区。

## 引导扇区 0~0

引导扇区作为文件镜像的第一个扇区，存储 boot 程序和文件系统信息，`file` 命令也是通过这个扇区得出文件系统信息

## 保留扇区 1~31

紧接着在引导扇区之后，就是一系列的保留扇区，保留扇区另作他用，我们可以暂时不用管这些保留扇区的作用，它是通过引导扇区数据结构中的 `BPB_RsvdSecCnt` 项得来的（那么它的值是什么？）

## FAT 区 32~95

`a.img` 一共有两张 FAT 表，每张 FAT 表占 32 个扇区。FAT 表用于存储文件的索引信息，对于一个文件系统尤为重要，这部分数据可以抽象为一个 unsigned int 数组（想想看为什么这个文件系统叫 FAT32），数组中的每个元素记录的是簇的后继簇，如果你对链表比较熟悉的话，你会发现每一个文件是由一个链表串接起来的。

## 数据区 96~32767

如果你已经阅读过了 orange 这本教材知道 FAT12 是怎么分配扇区的，你会发现一件比较诧异的事，FAT32 根本没有根目录区（都什么年代了还在 ~~抽~~ 用传统 FAT，~~好像 FAT32 也算传统 FAT 了~~）。

但是不对啊，一个文件系统怎么没有根，想象以下你用的系统不能用 `/`，那你就没办法索引到其他文件了，而仔细想想，根文件不就是一个目录文件嘛，为什么需要特殊划分一块根目录区专门处理……

在 FAT32 中的数据结构中，`BPB_RootClus` 能够帮助你找到根目录文件的一项，具体的含义就需要你自己读手册了。

## FAT32 目录项

FAT32 的目录文件引入了长目录项和短目录项，如果将长目录项考虑进来会导致理解复杂不易于理解，出于实验简单考虑，我们只考虑短目录项，比如下面就是一个 FAT32 镜像被放入一个名为 `1.c` 的文件。

```plain
00000000: 4131 002e 0063 0000 00ff ff0f 00f4 ffff  A1...c..........
00000010: ffff ffff ffff ffff ffff 0000 ffff ffff  ................
00000020: 3120 2020 2020 2020 4320 2020 0099 775f  1       C   ..w_
00000030: 3959 3959 0000 775f 3959 0300 ab00 0000  9Y9Y..w_9Y......
```

通过 `xxd` 可以看到一共占 64 个字节，但其实是一个长目录项和一个短目录项的组合。前 32 个字节存放的是一个长目录项，后 32 个字节存放的是一个短目录项，我们在做实验的时候只需要关心短目录项就好了。

## 簇号和数据区扇区

FAT 官方规定，FAT 表的前两个 FAT 项为保留数据，所以数据区的第 0 个簇对应的是 FAT 表中的第 2 个 FAT 项，即数据区的第 $x$ 个簇的后继簇的值对应 FAT 表中的第 $x+2$ 个 FAT 项。

这里需要特别提醒的是这次一个簇可能有多个扇区，虽然一个簇对应一个扇区从简单角度理解方便，但是这样效率太低了，为了提升效率，一个簇对应多个扇区能够有效提升磁盘读取性能（现代的 SSD 还好，特别是之前的机械硬盘的随机读老慢了）。

在 FAT32 中的数据结构中，`BPB_SecPerClus` 能够帮助你搞清楚这一点，具体的含义就需要你自己读手册了。

## 文档

[FAT 官方文档](https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf)