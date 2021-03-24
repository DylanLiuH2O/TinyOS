#!/bin/bash

LIB=/home/dylan/Workspaces/repo/OS/lib
TOOL=/home/dylan/Workspaces/repo/OS/tool
KEL=/home/dylan/Workspaces/repo/OS/kernel
HD=/home/dylan/Workspaces/repo/OS/hd

nasm -f elf -o ${LIB}/kernel/print.o ${LIB}/kernel/print.S
gcc -m32 -fno-builtin -nostdlib -nostdinc -fno-stack-protector -c main.c -o main.o
ld -m elf_i386 ${KEL}/main.o ${LIB}/kernel/print.o -Ttext 0xc0001500 -e main -o kernel.bin

dd if=${KEL}/kernel.bin of=${HD}/hd60M.img bs=512 count=200 seek=9 conv=notrunc
