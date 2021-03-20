#!/bin/bash
# 链接生成32位平台的可执行程序
file=$1
addr=$2
entry=$3
out=$4

ld -m elf_i386 ${file} -Ttext ${addr} -e ${entry} -o ${out}
