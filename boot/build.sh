#!/bin/bash
INCLUDE=include/
USRHOME=/home/dylan
OS=/home/dylan/Workspaces/repo/OS/hd

echo "building mbr..."
nasm -I ${INCLUDE} -o mbr.bin mbr.S

echo "building loader..."
nasm -I ${INCLUDE} -o loader.bin loader.S

echo "building img..."
dd if=./mbr.bin of=${OS}/hd60M.img bs=512 count=1 conv=notrunc
dd if=./loader.bin of=${OS}/hd60M.img bs=512 count=200 seek=2 conv=notrunc

echo "build successfully!"
