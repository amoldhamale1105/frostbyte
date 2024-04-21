# Frostbyte
A baremetal lightweight kernel with [POSIX](https://en.wikipedia.org/wiki/POSIX) compliant system call API (`user/lib/flib.h`) and shell commands. Runs in headless mode.  
### Architecture 
ARM64
### Board support
qemu, raspberrypi 3, raspberrypi 4

## Build from source
For out of box images, check out [Releases](https://github.com/amoldhamale1105/frostbyte/releases) and skip ahead to the [Run and Test](https://github.com/amoldhamale1105/frostbyte#run-and-test) section

### Environment setup
A Linux host machine  

You will need a pre-compiled cross toolchain to build the kernel. Download a Linux host based cross toolchain for aarch64 bare-metal target from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads  

You will either need [gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz](https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz?rev=981d8f7e91864070a466d852589598e2&hash=8D5397D4E41C99A96989ED813E8E95F0) or [gcc-arm-11.2-2022.02-aarch64-aarch64-none-elf.tar.xz](https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-aarch64-aarch64-none-elf.tar.xz?rev=6999776f159f49cbb12166e86dacd6c2&hash=703D7E8481C11FA8043E66EBF947A983) depending on your host machine architecture. You can choose a different or newer version but it should be for bare metal target only  

Append the toolchain root and binary path to your `PATH` env variable to enable `make` to resolve include paths and run seamlessly with the cross compiler prefix:
```
export PATH=$PATH:/path/to/toolchain/directory/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf:/path/to/toolchain/directory/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf/bin
```

### Build
Select target platform using the optional `BOARD` make variable. Currently available options based on [board support](https://github.com/amoldhamale1105/frostbyte/edit/master/README.md#board-support):  
- rpi3 (WIP, builds image for qemu emulated raspberrypi 3)
- rpi4 (builds image for raspberrypi 4 SoC)
- qemu => default

To build the entire project with the kernel and userspace apps, at the top of the source tree
```
make all
```
For a release build, set the `DEBUG` make variable to 0
```
make all DEBUG=0
```
To mount and unmount the FAT16 disk image, you can use the mount and unmount targets as below
```
make mount
make unmount
```
> **__Note:__** If the build fails upon any changes, the disk image may have to be unmounted with `make unmount` before `make all` can be run again  

To clean build and output artifacts,
```
make clean
```

## Run and Test 
### Hardware
Flash an SD card with a 64-bit [raspberrypi OS](https://www.raspberrypi.com/software/operating-systems/) image. Mount the boot parition and edit the **config.txt** file to add the following 2 lines:
```
dtoverlay=disable-bt
enable_uart=1
```
Replace the **kernel8.img** file in the mounted boot partition directory with frostbyte's. Insert the SD card into the board.  

Connect the board serially to your host machine. Set the serial port device (mostly `/dev/ttyUSB0`) and baud rate to 115200 in your serial port communication program. Boot.  
### Emulator
Start a raspberry pi 3 emulator running frostbyte  
```
qemu-system-aarch64 \
    -m 1G \
    -machine raspi3b \
    -serial mon:stdio \
    -kernel /path/to/kernel8.img \
    -nographic
```
On older qemu versions, you may have to use machine type as `raspi3` instead of `raspi3b`. Run `qemu-system-aarch64 -machine help` if in doubt.   

The OS boots up to a login prompt on the serial console. The login process parses the **passwd** file on the disk for registered users. Default user is *root* with default password *toor*  

![frostbyte_login](https://github.com/amoldhamale1105/frostbyte/assets/78597991/b6e38f13-7c5a-448b-b2d3-ae787ebeed37)

On successful login, a shell for current user is activated.  

![frostbyte_shell](https://github.com/amoldhamale1105/frostbyte/assets/78597991/6a71443b-1e87-4047-8a35-1cec4d5748c7)

## Features and Capabilities
This section may not be up to date with the current version of the kernel on master branch. However, it is highly recommended to read through it to get an idea of what **frostbyte** offers as its core functionality and extended features.  

### Features
- Kernel runs at privileged [exception level](https://developer.arm.com/documentation/102412/0103/Privilege-and-Exception-levels/Exception-levels) 1 (EL1)
- Userspace apps run at exception level 0 (EL0)
- Synchronous and asynchronous exception handling
- Interrupt handling and interrupt vector table
- Timer interrupt based FIFO scheduler
- Paging and virtual memory management
- FAT16 filesystem support
- VFS (Virtual filesystem)
- Multi-user mode with login prompt
- Serial console interactive shell
- POSIX compliant system calls, library functions and commands
- POSIX signal handling framework with default and custom handlers
- Custom program execution on shell with command-line arguments
- User inputs (stdin) to foreground user programs
- Foreground and background process control
- Environment variables

You can execute commands and programs on the shell by simply entering their name with or without extension. Commands entered on the shell are case insensitive. All executables on frostbyte need to have the `.bin` extension to be qualified as an executable. However, during execution, usage of the `.bin` extension is optional  
For example, all of the following commands will yield same output
```
uname
uname.bin
UNAME.BIN
uNaMe.BiN
uNaMe
``` 
### Commands
The following POSIX commands are currently supported by **frostbyte** with options.  
```
sh, uname, ls, ps, jobs, fg, bg, export, echo, env, unset, cat, kill, exit, shutdown
```
Usage and short description of any command can be viewed with the `-h` option. For instance, `uname -h` will yield the following output:
```
Usage:	uname [OPTION]
	Print certain system information.  With no OPTION, same as -s.

	-h	display this help and exit
	-a	print all system information in following order:
	-s	print the kernel name
	-r	print the kernel release
	-m	print the machine hardware name
	-i	print the hardware platform architecture
```

## Contributions
You can contribute to this project if you find it interesting enough.

As a potential contributor you are welcome to 
- Review pull requests and address [open issues](https://github.com/amoldhamale1105/frostbyte/issues)
- Create [new issues](https://github.com/amoldhamale1105/frostbyte/issues/new) for bugs or feature requests  
- Raise [PR](https://github.com/amoldhamale1105/frostbyte/pulls)s to address existing issues, fix bugs, make improvements or add new features  

Point of contact in case of any questions or suggestions: amoldhamale1105@gmail.com
