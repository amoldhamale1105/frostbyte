# pious
A multi-tasking kernel developed from scratch for 64-bit ARM architecture. The current version is supported for a virtualized raspberry pi 3b. Due to lack of hardware availability, it is not tested on real hardware yet.  

## Overview
The source code is broadly categorized into 2 sections:  
- The kernel source tree at the top level
- The **user** directory containing userspace apps and libraries
The filesystem uses a FAT16 disk whose image is present in the **boot** directory  

The kernel doesn't have a disk driver and loader, so the FAT16 disk image is appended to the end of the kernel image creating one monolithic image which will be be extracted at runtime by the kernel and loaded at 2 different virtual addresses in memory  

## Prerequisites
A Linux host machine  
You will need a pre-compiled cross toolchain to build the kernel. Download a Linux host based cross toolchain for aarch64 bare-metal target from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads  
You will either need **gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz** or **gcc-arm-11.2-2022.02-aarch64-aarch64-none-elf.tar.xz** depending on your host machine architecture. You can choose a different or newer version but it should be for bare metal target only  
Append the toolchain path to your `PATH` env variable to enable `make` to run seamlessly with the cross compiler prefix:
```
export PATH=$PATH:/path/to/toolchain/directory/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf/bin
```
Download **qemu** or build it from source to be able to emulate raspberry pi 3b. You'll find plenty of resources on the internet for that so I'm just going to skip the details.

## Build instructions
The top level **Makefile** in the source tree is designed to build the entire project including the userspace and test utilities. It will also take care of copying over required binaries and other files to the FAT16 disk image. 
To build the entire project with the kernel and userspace binaries, navigate to the root of this project
```
make all
```
To mount and unmount the FAT16 disk image, you can use the mount and unmount targets as below
```
make mount
make unmount
```
**Note:** If the build fails upon any changes, the disk image might have to be unmounted with `make unmount` before `make all` can be run again  
To clean build and output artifacts,
```
make clean
```
You can selectively build and clean artifacts by running make on specific targets only. Check out the Makefile for more ideas  
All build artifacts will be present in the **build** directory and output artifacts in the **bin** directory on successful build.

## Run and Test
A successful build will create **kernel8.img** and **pious** files in the **bin** directory.  
At the project root, run the following command to start the qemu based virtual raspberry pi 3b with our kernel image
```
qemu-system-aarch64 -m 1G -M raspi3b -serial stdio -kernel bin/kernel8.img
```
At the time of writing this readme, **init.bin** has been setup as the first userspace process (PID 1) which prints it's status 5 times at 1 sec interval and exits, handing over control to the idle process (PID 0)  

## Contributions
The kernel is currently work in progress. I have more features planned and will develop them as and when possible. Until then, kindly review the existing source code, play around, test and suggest improvements. I have added elaborate and copious comments in the code to make it easily comprehensible. Do get in touch with me in case of any questions or suggestions amoldhamale1105@gmail.com
