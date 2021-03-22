#!/bin/bash

TOOL=/home/dylan/Workspaces/repo/OS/tool
KEL=/home/dylan/Workspaces/repo/OS/kernel

${TOOL}/gcc32.sh main.c main.o
${TOOL}/ld32.sh main.o 0xc0001500 main kernel.bin

dd if=${KEL}/kernel.bin of=/home/dylan/bochs/bin/hd60M.img bs=512 count=200 seek=9 conv=notrunc
