# Frostbyte
A minimalistic multi-tasking OS developed from scratch for 64-bit ARM architecture. The current version is tested on a virtualized raspberry pi 3b. The OS behaves and works on the emulated board exactly as it would on real hardware because of a full system emulation using qemu.  

## Overview
Frostbyte comprises of a kernel, some userspace apps and a shell as one of the apps facilitating interaction with the system  

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
A successful build will create **kernel8.img** and **frostbyte** files in the **bin** directory.  

At the project root, run the following command to start the qemu based virtual raspberry pi 3b with our kernel image
```
qemu-system-aarch64 -m 1G -M raspi3b -serial mon:stdio -kernel bin/kernel8.img -nographic
```
On older qemu versions, you may have to use machine type as `raspi3` instead of `raspi3b`. Run `qemu-system-aarch64 -machine help` if in doubt.  

At the time of writing this readme, the OS boots up to a shell on the serial console where you can run commands to interact with the system.  

![Frostbyte](https://github.com/amoldhamale1105/frostbyte/assets/78597991/4a01b30d-7c95-4639-9cf4-cc98d943de5e)

## Features and Capabilities
This content is valid as of July 3, 2023. It might be outdated for current version of the kernel. Will be creating releases soon to keep track of features in a specific version  

You can execute commands and programs on the shell by simply entering their name with or without executable suffix. All executables on frostbyte need to have the `.bin` suffix to be qualified as an executable  

Programs can be run in the foreground or background. To run a program in background we follow the standard practice of appending an `&` at the end. The shell then prints the pid and name of the process it just pushed to background and gets ready for next user input  

The command list below will describe additional features  

### Commands
All commands are POSIX compliant but with limited options and arguments  
- **ls**        list files in root directory
- **ps**        list active processes on the system
- **cat**       read a file by printing its contents to the shell  
- **kill**      send a signal to a process
- **shutdown**  shutdown the system

## Contributions
At this stage, Frostbyte is a hobby project and you are welcome to use and contribute to it if you find it interesting enough.

As a potential contributor you are welcome to 
- Review pull requests and known issues from the **Issues** tab and check if you can resolve any of them
- Create new issues for bugs or feature requests so that either I or others in the community can get to it 
- Raise PRs to address existing issues, to fix potential bugs or make any improvements to existing source code

I have added elaborate comments in the code to make it easily comprehensible and I'd like anyone who is contributing to continue that practice for newly added code. Do get in touch with me in case of any questions or suggestions amoldhamale1105@gmail.com
