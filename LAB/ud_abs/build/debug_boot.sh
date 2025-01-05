#!/bin/bash

qemu-system-x86_64 \
	-machine accel=tcg \
	-S -s \
	-drive format=raw,file=./hello-x86_64-qemu &
pid=$!

gdb -x bootloader.gdb; kill -9 $!
