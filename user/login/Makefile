PROGRAM_NAME := login
SRC_DIR := .
INCLUDES := -I. -I../lib
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
OBJS := $(BUILD_DIR)/start.o $(BUILD_DIR)/main.o $(BUILD_DIR)/login.o ../lib/bin/flib.a

ifeq ($(BOARD), rpi3)
    CFLAGS += -DRPI3
else ifeq ($(BOARD), rpi4)
    CFLAGS += -DRPI4
else
    CFLAGS += -DQEMU
endif

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
	$(CC) -DNAME=$(KERNEL_NAME) $(INCLUDES) $(CFLAGS) -c $< -o $@