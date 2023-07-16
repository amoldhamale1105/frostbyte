export TARGET_ARCH := aarch64
export PREFIX := $(TARGET_ARCH)-none-elf-
export CC := $(PREFIX)gcc
export LINK := $(PREFIX)ld
export OBJ_COPY := $(PREFIX)objcopy

export CFLAGS := -g -ffreestanding -mgeneral-regs-only -nostdlib -std=c99 -O0 -nostartfiles
export LDFLAGS := -nostdlib

SRC_DIR := .
INCLUDES := -I.
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
export MOUNT_POINT := $(PWD)/temp
export KERNEL_NAME := frostbyte
export KERNEL_VERSION := 1.0.0
export FAT16_DISK := $(PWD)/boot/$(KERNEL_NAME)_disk.img
export KERNEL_IMAGE := kernel8.img
OBJS := $(BUILD_DIR)/boot.o $(BUILD_DIR)/main.o $(BUILD_DIR)/libc_asm.o $(BUILD_DIR)/uart.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o \
		$(BUILD_DIR)/handler.o $(BUILD_DIR)/exception.o $(BUILD_DIR)/mmu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/file.o ${BUILD_DIR}/process.o \
		$(BUILD_DIR)/syscall.o $(BUILD_DIR)/libc.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/signal.o

$(info $(shell mkdir -p $(BUILD_DIR) $(OUTPUT_DIR)))

.PHONY: all mount unmount clean user
all: mount kernel user unmount
	dd if=$(FAT16_DISK) >> $(OUTPUT_DIR)/$(KERNEL_IMAGE)

mount:
	mkdir -p $(MOUNT_POINT)
	sudo mount -t vfat -o loop,offset=$$((63*512)),uid=$$(id -u),gid=$$(id -g) $(FAT16_DISK) $(MOUNT_POINT)

unmount:
	sync && sudo umount $(MOUNT_POINT)
	rm -rf $(MOUNT_POINT)

kernel: $(OBJS)
	$(LINK) $(LDFLAGS) -T linker.ld -o $(OUTPUT_DIR)/$(KERNEL_NAME) $? 
	$(OBJ_COPY) -O binary $(OUTPUT_DIR)/$(KERNEL_NAME) $(OUTPUT_DIR)/$(KERNEL_IMAGE)

user:
	cd ./user/lib && $(MAKE)
	cd ./user/init && $(MAKE)
	cd ./user/shell && $(MAKE)
	cd ./user/ps && $(MAKE)
	cd ./user/list && $(MAKE)
	cd ./user/cat && $(MAKE)
	cd ./user/kill && $(MAKE)
	cd ./user/uname && $(MAKE)
	cd ./user/shutdown && $(MAKE)
	cd ./user/test && $(MAKE)

user_clean:
	cd ./user/lib && $(MAKE) clean
	cd ./user/init && $(MAKE) clean
	cd ./user/shell && $(MAKE) clean
	cd ./user/ps && $(MAKE) clean
	cd ./user/list && $(MAKE) clean
	cd ./user/cat && $(MAKE) clean
	cd ./user/kill && $(MAKE) clean
	cd ./user/uname && $(MAKE) clean
	cd ./user/shutdown && $(MAKE) clean
	cd ./user/test && $(MAKE) clean

clean: user_clean
	rm -f $(BUILD_DIR)/*
	rm -f $(OUTPUT_DIR)/*

$(BUILD_DIR)/main.o : $(SRC_DIR)/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/*/%.s
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/*/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@