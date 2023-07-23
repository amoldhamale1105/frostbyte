# Frostbyte
A minimalistic multi-tasking OS developed from scratch for 64-bit ARM architecture. The current version is tested on a virtualized raspberry pi 3b.  

The purpose of this project is to explore internals of an operating system and create a simple OS which can run on an ARM64 machine and do something useful like view contents on a disk, read files, execute programs and manage running processes.  

The overarching principle is to cross that utility threshold with bare minimum features such that it doesn't just become another shelved useless piece of software which technically works as per design but hardly exposes any interfaces to the user to experience or work with it.  

With **frostbyte**, you can interact with the system on the shell by running commands and programs as well as develop custom programs which can be executed from the shell  

## Overview
Frostbyte comprises of a kernel, some default userspace apps and a shell as one of the apps facilitating interaction with the system  

The source code is broadly categorized into 2 sections:  
- The kernel source tree at the top level
- The **user** directory containing userspace apps and libraries

The filesystem uses a FAT16 disk whose image is present in the **boot** directory. As of now, the kernel doesn't have a disk driver and loader, so the FAT16 disk image is appended to the end of the kernel image creating one monolithic image which will be be extracted at runtime and loaded at 2 different virtual addresses in memory by the kernel

## Prerequisites  
Download [qemu](https://www.qemu.org/download/) or build it from source to be able to emulate raspberry pi 3b.  

## Build instructions
For out of the box ready to use and test images, check out [Releases](https://github.com/amoldhamale1105/frostbyte/releases) and skip ahead to the [Run and Test](https://github.com/amoldhamale1105/frostbyte#run-and-test) section in this readme

### Environment setup
A Linux host machine  

You will need a pre-compiled cross toolchain to build the kernel. Download a Linux host based cross toolchain for aarch64 bare-metal target from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads  

You will either need [gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz](https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz?rev=981d8f7e91864070a466d852589598e2&hash=8D5397D4E41C99A96989ED813E8E95F0) or [gcc-arm-11.2-2022.02-aarch64-aarch64-none-elf.tar.xz](https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-aarch64-aarch64-none-elf.tar.xz?rev=6999776f159f49cbb12166e86dacd6c2&hash=703D7E8481C11FA8043E66EBF947A983) depending on your host machine architecture. You can choose a different or newer version but it should be for bare metal target only  

Append the toolchain path to your `PATH` env variable to enable `make` to run seamlessly with the cross compiler prefix:
```
export PATH=$PATH:/path/to/toolchain/directory/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf/bin
```

### Build
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
> **__Note:__** If the build fails upon any changes, the disk image might have to be unmounted with `make unmount` before `make all` can be run again  

To clean build and output artifacts,
```
make clean
```
You can selectively build and clean artifacts by running make on specific targets only. Check out the Makefile for more ideas  

All build artifacts will be present in the **build** directory and output artifacts in the **bin** directory. A successful build should generate **kernel8.img** and **frostbyte** files in the **bin** directory.

## Run and Test  
Replace `/path/to/kernel-image` in the following command with location of the `kernel8.img` file and run it to start a virtualized raspberry pi 3b instance running **frostbyte**.
```
qemu-system-aarch64 \
    -m 1G \
    -machine raspi3b \
    -serial mon:stdio \
    -kernel /path/to/kernel-image \
    -nographic
```
On older qemu versions, you may have to use machine type as `raspi3` instead of `raspi3b`. Run `qemu-system-aarch64 -machine help` if in doubt.  

At the time of writing this readme, the OS boots up to a shell on the serial console where you can run commands to interact with the system.  

![Frostbyte](https://github.com/amoldhamale1105/frostbyte/assets/78597991/4a01b30d-7c95-4639-9cf4-cc98d943de5e)

## Features and Capabilities
This content is valid as of July 23, 2023. It might be outdated for current version of the kernel. Check out [Releases](https://github.com/amoldhamale1105/frostbyte/releases) for a specific version and encompassing features  
> **__Note:__** This section is a more of a user guide than a development guide. Custom user programs can be written for frostbyte similar to the programs developed for various commands. A separate development guide and API documentation will be created shortly with all the custom library functions and system calls. In the meantime, if you want to develop your own userspace app for frostbyte, check out the header `user/lib/flib.h` for prototypes and `user/test` and `user/sampleapp` as reference applications

### Features
- Kernel runs at privileged [exception level](https://developer.arm.com/documentation/102412/0103/Privilege-and-Exception-levels/Exception-levels) 1 (EL1)
- Userspace apps run at exception level 0 (EL0)
- Synchronous and asynchronous exception handling
- Interrupt handling and interrupt vector table
- Timer interrupt based FIFO scheduling
- Paging and virtual memory management
- FAT16 filesystem support
- VFS (Virtual filesystem)
- SysV analogous **init** startup
- Serial console interactive shell
- POSIX compliant system calls and commands
- POSIX signal handling framework
- Custom standard library funtions
- Executing custom programs with command-line arguments
- User inputs to custom programs using `scanf`
- Foreground and background process management

You can execute commands and programs on the shell by simply entering their name with or without executable suffix. Commands entered on the shell are case insensitive. All executables on frostbyte need to have the `.bin` suffix to be qualified as an executable. However, during execution, usage of the `.bin` suffix is optional  
For example, all of the following commands will result in `frostbyte` as output
```
uname
uname.bin
UNAME.BIN
uNaMe.BiN
uNaMe
```
When you list files with `ls` command, their names will appear in uppercase. The kernel is currently capable of listing files from the FAT16 root directory only. Some directories are placed in the disk image only to test `ls` command's long listing output. Listing files inside subdirectories or changing working directory to a subdirectory is not supported as of the writing of this readme. 

Programs can be run in the foreground or background. To run a program in background we follow the standard practice of appending an `&` at the end. The shell then prints the pid and name of the process it just pushed to background and gets ready for next user input  

Programs running in the foreground can be terminated with a keyboard interrupt by pressing **Ctrl+C** which will make the shell avaiable again for user input. Apart from this, standard POSIX signals can be sent to any process using the `kill` command  

### Commands
The following POSIX commands are currently supported by **frostbyte** OS with options.  
```
uname, ls, ps, cat, kill, shutdown
```
Usage and short description of any command can be viewed with the `-h` option  
For example, `uname -h` will print the following output to the terminal
```
Usage:
	uname [OPTION]
Print certain system information.  With no OPTION, same as -s.

	-h	display this help and exit
	-a	print all system information in following order:
	-s	print the kernel name
	-r	print the kernel version
	-i	print the hardware platform architecture
```

## Contributions
At this stage, Frostbyte is a hobby project and you are welcome to use and contribute to it if you find it interesting enough.

As a potential contributor you are welcome to 
- Review pull requests and [open issues](https://github.com/amoldhamale1105/frostbyte/issues) and check if you can resolve any of them
- Create [new issues](https://github.com/amoldhamale1105/frostbyte/issues/new) for bugs or feature requests so that either I or others in the community can get to it 
- Raise [PR](https://github.com/amoldhamale1105/frostbyte/pulls)s to address existing issues, to fix potential bugs or make any improvements to existing source code

I have added elaborate comments in the code to make it easily comprehensible and I'd like anyone who is contributing to continue that practice for newly added code. Do get in touch with me in case of any questions or suggestions amoldhamale1105@gmail.com
