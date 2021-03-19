#!/bin/bash
INCLUDE=include/
USRHOME=/home/dylan

echo "building mbr..."
nasm -I ${INCLUDE} -o mbr.bin mbr.S

echo "building loader..."
nasm -I ${INCLUDE} -o loader.bin loader.S

echo "building img..."
dd if=./mbr.bin of=${USRHOME}/bochs/bin/hd60M.img bs=512 count=1 conv=notrunc
dd if=./loader.bin of=${USRHOME}/bochs/bin/hd60M.img bs=512 count=1 seek=2 conv=notrunc

echo "build successfully!"
