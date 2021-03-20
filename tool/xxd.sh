#!/bin/bash
# 查看二进制文件的数据
xxd -u -a -g 1 -s $2 -l $3 $1
