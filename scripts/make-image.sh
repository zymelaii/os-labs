#!/usr/bin/bash

if [ "$#" -le "5" ]; then
    echo "USAGE: $0 <image> <mbr> <boot> <loader> <kernel> [user-files...]"
    exit;
fi

image_file="$1"
mbr_file="$2"
boot_file="$3"
loader_file="$4"
kernel_file="$5"

user_files=($@)
user_files=(${user_files[@]:5})

# create a 64MB image file
dd if=/dev/zero of=$image_file bs=1024 count=65536 > /dev/null 2>&1

# write partition table
sfdisk $image_file < part.sfdisk > /dev/null 2>&1

# write mbr
dd if=$mbr_file of=$image_file bs=1 count=446 conv=notrunc > /dev/null 2>&1

# use losetup to virtualize the image file as a block device
# find a vacant loop device slot
loop_device=`sudo losetup -f`
sudo losetup -P $loop_device $image_file

# mannually make fs for each partition
sudo mkfs.vfat -F 32 -s 8 -r 224 -M 0xf0 -h 0 -D 0x80 ${loop_device}p1 > /dev/null 2>&1

# install files for bootable partitions
mount_point=$(mktemp -d)
for i in 1; do
    part_device=${loop_device}p${i}
    sudo dd if=$boot_file of=$part_device bs=1 count=420 seek=90 conv=notrunc > /dev/null 2>&1
    sudo mount $part_device $mount_point
    sudo cp -f $loader_file $mount_point
    sudo cp -f $kernel_file $mount_point
    for user_file in ${user_files[@]}; do
        sudo cp -f $user_file $mount_point
    done
    sudo umount $mount_point
done

# release the loop device
sudo losetup -d ${loop_device}
