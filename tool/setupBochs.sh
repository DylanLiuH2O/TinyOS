#!/bin/bash

BOCHS=/home/dylan/Downloads/bochs-2.6.11/
HOME=/home/dylan
PWD=$(pwd)

sudo apt install libx11-dev
mkdir ${HOME}/bochs
cd ${BOCHS}
make clean

./configure \
--prefix=${HOME}/bochs \
--enable-debugger \
--enable-disasm \
--enable-iodebug \
--enable-x86-debugger \
--with-x \
--with-x11

make
make install
cd ${PWD}
