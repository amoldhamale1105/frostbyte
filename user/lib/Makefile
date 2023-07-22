PROGRAM_NAME := flib
STATIC_LIB := $(PREFIX)ar
LIBFLAGS := rcs

SRC_DIR := .
INCLUDES := -I.
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
OBJS := $(BUILD_DIR)/print.o $(BUILD_DIR)/flib.o $(BUILD_DIR)/flib_asm.o

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