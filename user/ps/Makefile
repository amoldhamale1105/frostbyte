PROGRAM_NAME := ps
SRC_DIR := .
INCLUDES := -I. -I../lib
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
OBJS := $(BUILD_DIR)/start.o $(BUILD_DIR)/main.o ../lib/bin/stdlib.a

$(info $(shell mkdir -p $(BUILD_DIR) $(OUTPUT_DIR)))

.PHONY: all
all: $(OBJS)
	$(LINK) $(LDFLAGS) -T linker.ld -o $(OUTPUT_DIR)/$(PROGRAM_NAME).elf $? 
	$(OBJ_COPY) -O binary $(OUTPUT_DIR)/$(PROGRAM_NAME).elf $(OUTPUT_DIR)/$(PROGRAM_NAME).bin
	cp -ra $(OUTPUT_DIR)/*.bin $(MOUNT_POINT)/

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*
	rm -f $(OUTPUT_DIR)/*

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@