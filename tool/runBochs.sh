#!/bin/bash

BOCHS=/home/dylan/bochs/bin
OS=/home/dylan/Workspaces/repo/OS/configs

${BOCHS}/bochs -f ${OS}/bochsrc.disk
