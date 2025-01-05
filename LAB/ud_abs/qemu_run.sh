#!/bin/bash
make ARCH=x86_64-qemu
qemu-system-x86_64 -drive format=raw,file=build/hello-x86_64-qemu