SRC_DIR := .
INCLUDES := -I.
BUILD_DIR := ./build
OUTPUT_DIR := ./bin
OBJS := $(BUILD_DIR)/test.o

$(info $(shell mkdir -p $(BUILD_DIR) $(OUTPUT_DIR)))

.PHONY: all
all: $(OBJS)
	cp -ra $(OUTPUT_DIR)/*.bin $(MOUNT_POINT)/

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*
	rm -f $(OUTPUT_DIR)/*

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
	$(OBJ_COPY) -O binary $@ $(OUTPUT_DIR)/$(basename $(@F)).bin

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
	$(OBJ_COPY) -O binary $@ $(OUTPUT_DIR)/$(basename $(@F)).bin