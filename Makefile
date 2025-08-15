CC := clang

TARGET_EXEC := game

BUILD_DIR := build
SRC_DIRS := src lib

SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# pass folders to gcc so it can find headers
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CCFLAGS := -std=c11 -Wall -Wextra
LDFLAGS := -lglfw -lassimp -lcglm -lm

DEBUG_FLAGS := -g

all: run clean

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(INC_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

.PHONY: compile
compile: $(BUILD_DIR)/$(TARGET_EXEC)

.PHONY: run
run: $(BUILD_DIR)/$(TARGET_EXEC)
	$(BUILD_DIR)/$(TARGET_EXEC)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)/*
