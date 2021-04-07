#!/bin/bash

BOCHS=/home/dylan/bochs/bin
OS=/home/dylan/Workspaces/repo/TinyOS/configs

${BOCHS}/bochs -f ${OS}/bochsrc.disk
