PROGRAM_NAME := flib
STATIC_LIB := $(PREFIX)ar
LIBFLAGS := rcs

SRC_DIR := .
INCLUDES := -I./$(TARGET_ARCH)-$(VENDOR)-$(TARGET_OS)/include -I./lib/gcc/$(TARGET_ARCH)-$(VENDOR)-$(TARGET_OS)/$(GCC_VERSION)/include -I.
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
OBJS := $(BUILD_DIR)/print.o $(BUILD_DIR)/flib.o $(BUILD_DIR)/flib_asm.o

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
	$(STATIC_LIB) $(LIBFLAGS) $(OUTPUT_DIR)/$(PROGRAM_NAME).a $?

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*
	rm -f $(OUTPUT_DIR)/*

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@