#!/bin/bash
# 以32位机器为目标平台进行编译
file=$1
out=$2

gcc -m32 -c ${file} -o ${out}
