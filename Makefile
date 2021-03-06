BUILD=./build
ENTRY=0xc0001500
AS=nasm
CC=gcc
LD=ld
HD=/home/dylan/Workspaces/repo/TinyOS/hd

ASFLAGS=-f elf
CFLAGS=-m32 $(INCLUDE) -c -fno-builtin -fno-stack-protector
LDFLAGS=-m elf_i386 -Ttext $(ENTRY) -e main -Map $(BUILD)/kernel.map

#OBJS=$(shell find ./build/ -name "*.o")
OBJS=$(BUILD)/main.o $(BUILD)/init.o $(BUILD)/interrupt.o $(BUILD)/timer.o \
     $(BUILD)/string.o $(BUILD)/debug.o $(BUILD)/kernel.o $(BUILD)/print.o \
     $(BUILD)/bitmap.o $(BUILD)/memory.o $(BUILD)/thread.o $(BUILD)/list.o \
     $(BUILD)/switch.o $(BUILD)/sync.o $(BUILD)/console.o $(BUILD)/keyboard.o \
	 $(BUILD)/ioqueue.o

#INCLUDE=-I./lib -I./lib/kernel -I./lib/user -I./kernel -I./device -I./thread
INCLUDE=-I./

# C
$(BUILD)/main.o: kernel/main.c lib/kernel/print.h lib/stdint.h kernel/init.h \
                 lib/string.h lib/kernel/bitmap.h thread/thread.h            \
                 lib/kernel/list.h thread/switch.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/init.o: kernel/init.c kernel/init.h lib/kernel/print.h lib/stdint.h \
                 kernel/interrupt.h device/timer.h kernel/memory.h          \
                 thread/thread.h thread/sync.h device/console.h 			\
				 device/keyboard.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/interrupt.o: kernel/interrupt.c kernel/interrupt.h lib/stdint.h \
                      kernel/global.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/timer.o: device/timer.c device/timer.h lib/stdint.h lib/kernel/io.h \
                  lib/kernel/print.h kernel/interrupt.h kernel/debug.h       \
                  thread/thread.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/debug.o: kernel/debug.c kernel/debug.h lib/kernel/print.h \
                  lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/string.o: lib/string.c lib/string.h lib/stdint.h kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/bitmap.o: lib/kernel/bitmap.c lib/kernel/bitmap.h lib/string.h \
                   lib/stdint.h kernel/debug.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/memory.o: kernel/memory.c kernel/memory.h lib/string.h lib/stdint.h \
                   lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/thread.o: thread/thread.c thread/thread.h kernel/memory.h \
                   lib/string.h lib/stdint.h lib/kernel/list.h     \
                   thread/switch.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/list.o: lib/kernel/list.c lib/kernel/list.h kernel/interrupt.h \
                 lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/sync.o: thread/sync.c lib/kernel/list.h kernel/interrupt.h \
                 kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/console.o: device/console.c device/console.h lib/kernel/print.h lib/stdint.h \
                    thread/sync.h thread/thread.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/keyboard.o: device/keyboard.c device/keyboard.h lib/kernel/print.h \
                     lib/kernel/io.h kernel/interrupt.h kernel/global.h     \
					 device/ioqueue.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/ioqueue.o: device/ioqueue.c device/ioqueue.h kernel/global.h lib/stdint.h \
                    thread/sync.h device/console.h
	$(CC) $(CFLAGS) $< -o $@

# 汇编
$(BUILD)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD)/print.o: lib/kernel/print.S
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD)/switch.o: thread/switch.S
	$(AS) $(ASFLAGS) $< -o $@

# 链接所有目标文件
$(BUILD)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY: clean hd clean all
clean:
	-rm $(OBJS) $(BUILD)/kernel.bin

mkdir:
	if [[ ! -d $(BUILD) ]];then mkdir $(BUILD);fi

build: $(BUILD)/kernel.bin

hd:
	dd if=$(BUILD)/kernel.bin \
	of=$(HD)/hd60M.img \
	bs=512 count=200 seek=9 conv=notrunc

all: build hd
