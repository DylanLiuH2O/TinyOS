#!/bin/bash

HD=/home/dylan/Workspaces/repo/OS/hd


nasm -f elf -o build/print.o lib/kernel/print.S
nasm -f elf -o build/kernel.o kernel/kernel.S


gcc -m32 -I lib/kernel -I lib/ -c -fno-builtin -fno-stack-protector -o build/timer.o device/timer.c

gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -fno-stack-protector \
    -o build/main.o kernel/main.c

gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -fno-stack-protector \
    -o build/interrupt.o kernel/interrupt.c

gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -I device/ -c -fno-builtin -fno-stack-protector \
    -o build/init.o kernel/init.c

ld -m elf_i386 build/main.o build/kernel.o build/init.o build/interrupt.o build/print.o build/timer.o \
    -Ttext 0xc0001500 -e main -o build/kernel.bin

dd if=build/kernel.bin of=${HD}/hd60M.img bs=512 count=200 seek=9 conv=notrunc
