#!/usr/bin/bash

if [ "$#" -ne "5" ]; then
    echo "USAGE: $0 <image> <mbr> <boot> <loader> <kernel>"
    exit;
fi

image_file="$1"
mbr_file="$2"
boot_file="$3"
loader_file="$4"
kernel_file="$5"

# create a 16MB image file
dd if=/dev/zero of=$image_file bs=1024 count=16384 > /dev/null 2>&1

# write partition table
sfdisk $image_file < part.sfdisk > /dev/null 2>&1

# write mbr
dd if=$mbr_file of=$image_file bs=1 count=446 conv=notrunc > /dev/null 2>&1

# use losetup to virtualize the image file as a block device
# find a vacant loop device slot
loop_device=`sudo losetup -f`
sudo losetup -P $loop_device $image_file

# mannually make fs for each partition
# NOTE: partition 2 & 4 are for this lab, others are for fun
sudo mkfs.vfat -F 16 ${loop_device}p1 > /dev/null 2>&1
sudo mkfs.vfat -F 32 -s 8 -r 224 -M 0xf0 -h 0 -D 0x80 ${loop_device}p2 > /dev/null 2>&1
sudo mkfs.vfat -F 32 -s 8 -r 224 -M 0xf0 -h 0 -D 0x80 ${loop_device}p4 > /dev/null 2>&1
sudo mkfs.ext2 ${loop_device}p5 > /dev/null 2>&1
sudo mkfs.minix ${loop_device}p6 > /dev/null 2>&1
sudo mkfs.vfat -F 12 ${loop_device}p7 > /dev/null 2>&1

# install files for bootable partitions
mount_point=$(mktemp -d)
for i in 2 4; do
    part_device=${loop_device}p${i}
    sudo dd if=$boot_file of=$part_device bs=1 count=420 seek=90 conv=notrunc > /dev/null 2>&1
    sudo mount $part_device $mount_point
    sudo cp -f $loader_file $mount_point
    sudo cp -f $kernel_file $mount_point
    sudo umount $mount_point
done

# release the loop device
sudo losetup -d ${loop_device}
