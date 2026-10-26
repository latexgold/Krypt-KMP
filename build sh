#!/bin/bash
# Clone the latest KernelPatch headers
git clone --depth 1 https://github.com/bmax121/KernelPatch.git kpatch
mkdir -p kernel/include
mkdir -p kernel/patch/include
cp -r kpatch/kernel/include/* kernel/include/
cp -r kpatch/kernel/patch/include/* kernel/patch/include/

# Build the KPM
export TARGET_COMPILE=aarch64-none-elf-
make
