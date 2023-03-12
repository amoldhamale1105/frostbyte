PREFIX := aarch64-none-elf-
CC := $(PREFIX)gcc
LINK := $(PREFIX)ld
OBJ_COPY := $(PREFIX)objcopy

CFLAGS := -g -ffreestanding -mgeneral-regs-only -nostdlib -std=c99 -O0 -nostartfiles
LDFLAGS := -nostdlib

SRC_DIR := .
INCLUDES := -I.
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
OBJS := $(BUILD_DIR)/boot.o $(BUILD_DIR)/main.o $(BUILD_DIR)/libc.o $(BUILD_DIR)/uart.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o

$(info $(shell mkdir -p $(BUILD_DIR) $(OUTPUT_DIR)))

.PHONY: all
all: $(OBJS)
	$(LINK) $(LDFLAGS) -T linker.ld -o $(OUTPUT_DIR)/pious $? 
	$(OBJ_COPY) -O binary $(OUTPUT_DIR)/pious $(OUTPUT_DIR)/kernel8.img

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*
	rm -f $(OUTPUT_DIR)/*

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s
	$(CC) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@