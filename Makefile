HOST_ARCH := $(shell uname -i)
export TARGET_ARCH := aarch64
export VENDOR := none
export TARGET_OS := elf
export PREFIX := $(TARGET_ARCH)-$(VENDOR)-$(TARGET_OS)-
export CC := $(PREFIX)gcc
export GCC_VERSION := $(shell $(CC) --version | sed -n 's/.* \([0-9]\+\.[0-9]\+\.[0-9]\+\) .*$$/\1/p')
export LINK := $(PREFIX)ld
export OBJ_COPY := $(PREFIX)objcopy

export CFLAGS := -ffreestanding -mgeneral-regs-only -nostdlib -std=c99 -O0 -nostartfiles
ASMLAGS := -x assembler-with-cpp
# Target platform
BOARD ?= qemu
ifeq ($(BOARD), rpi3)
    KERN_CFLAGS += -DRPI3
else ifeq ($(BOARD), rpi4)
    KERN_CFLAGS += -DRPI4
else
    KERN_CFLAGS += -DQEMU
endif

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
else
    CFLAGS += -DNDEBUG
endif
export LDFLAGS := -nostdlib

SRC_DIR := $(shell pwd)
INCLUDES := -I./$(TARGET_ARCH)-$(VENDOR)-$(TARGET_OS)/include -I./lib/gcc/$(TARGET_ARCH)-$(VENDOR)-$(TARGET_OS)/$(GCC_VERSION)/include -I.
BUILD_DIR := ./build
OUTPUT_DIR := ./bin/$(BOARD)
export MOUNT_POINT := $(SRC_DIR)/build/fdisk
export KERNEL_NAME := frostbyte
export KERNEL_VERSION := 2.4.1
export FAT16_DISK := $(KERNEL_NAME)_disk.img
export KERNEL_IMAGE := kernel8.img
OBJS := $(BUILD_DIR)/boot.o $(BUILD_DIR)/main.o $(BUILD_DIR)/lib_asm.o $(BUILD_DIR)/uart.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o \
		$(BUILD_DIR)/handler.o $(BUILD_DIR)/exception.o $(BUILD_DIR)/mmu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/file.o ${BUILD_DIR}/process.o \
		$(BUILD_DIR)/syscall.o $(BUILD_DIR)/lib.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/signal.o

$(info $(shell mkdir -p $(BUILD_DIR) $(OUTPUT_DIR)))

.PHONY: all mount unmount clean user
all: mount kernel user unmount
	dd if=$(BUILD_DIR)/$(FAT16_DISK) >> $(OUTPUT_DIR)/$(KERNEL_IMAGE)

mount:
	mkdir -p $(MOUNT_POINT)
	cp -ran ./boot/$(FAT16_DISK) $(BUILD_DIR)
	sudo mount -t vfat -o loop,offset=$$((63*512)),uid=$$(id -u),gid=$$(id -g) $(BUILD_DIR)/$(FAT16_DISK) $(MOUNT_POINT)

unmount:
	sync && sudo umount $(MOUNT_POINT)
	rm -rf $(MOUNT_POINT)

kernel: $(OBJS)
	$(LINK) $(LDFLAGS) -T linker.ld -o $(OUTPUT_DIR)/$(KERNEL_NAME) $? 
	$(OBJ_COPY) -O binary $(OUTPUT_DIR)/$(KERNEL_NAME) $(OUTPUT_DIR)/$(KERNEL_IMAGE)

user:
	cd ./user/lib && $(MAKE) BOARD=$(BOARD)
	cd ./user/init && $(MAKE)
	cd ./user/login && $(MAKE) BOARD=$(BOARD)
	cd ./user/shell && $(MAKE) BOARD=$(BOARD)
	cd ./user/ps && $(MAKE)
	cd ./user/jobs && $(MAKE)
	cd ./user/jobctl && $(MAKE)
	cd ./user/list && $(MAKE)
	cd ./user/env && $(MAKE)
	cd ./user/export && $(MAKE)
	cd ./user/echo && $(MAKE)
	cd ./user/unset && $(MAKE)
	cd ./user/cat && $(MAKE)
	cd ./user/kill && $(MAKE)
	cd ./user/uname && $(MAKE) BOARD=$(BOARD)
	cd ./user/exit && $(MAKE)
	cd ./user/shutdown && $(MAKE)
	cd ./user/test && $(MAKE)
	cd ./user/sampleapp && $(MAKE)

user_clean:
	cd ./user/lib && $(MAKE) clean
	cd ./user/init && $(MAKE) clean
	cd ./user/login && $(MAKE) clean
	cd ./user/shell && $(MAKE) clean
	cd ./user/ps && $(MAKE) clean
	cd ./user/jobs && $(MAKE) clean
	cd ./user/jobctl && $(MAKE) clean
	cd ./user/list && $(MAKE) clean
	cd ./user/env && $(MAKE) clean
	cd ./user/export && $(MAKE) clean
	cd ./user/echo && $(MAKE) clean
	cd ./user/unset && $(MAKE) clean
	cd ./user/cat && $(MAKE) clean
	cd ./user/kill && $(MAKE) clean
	cd ./user/uname && $(MAKE) clean
	cd ./user/exit && $(MAKE) clean
	cd ./user/shutdown && $(MAKE) clean
	cd ./user/test && $(MAKE) clean
	cd ./user/sampleapp && $(MAKE) clean

clean: user_clean
	rm -f $(BUILD_DIR)/*
	rm -f $(OUTPUT_DIR)/*

$(BUILD_DIR)/main.o : $(SRC_DIR)/main.c
	$(CC) $(INCLUDES) $(CFLAGS) $(KERN_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/*/%.s
	$(CC) $(INCLUDES) $(CFLAGS) $(ASMLAGS) $(KERN_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/*/%.c
	$(CC) $(INCLUDES) $(CFLAGS) $(KERN_CFLAGS) -c $< -o $@