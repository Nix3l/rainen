CC := clang

TARGET_EXEC := game

BUILD_DIR := build
SRC_DIRS := src lib

SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# pass folders to gcc so it can find headers
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CCFLAGS := -std=c11 -Wall -Wextra # -O2
LDFLAGS := -lglfw -lassimp -lcglm -lm -Llib -Llib/so -Wl,-rpath,lib/so -lcimgui

DEBUG_FLAGS := -g

# force it to run under xwayland to prevent weird wayland issues
RUN_ENV := GTK_IM_MODULE="" XDG_SESSION_TYPE=x11

all: run

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(INC_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

compile: clean $(BUILD_DIR)/$(TARGET_EXEC)

run: $(BUILD_DIR)/$(TARGET_EXEC)
	$(RUN_ENV) $(BUILD_DIR)/$(TARGET_EXEC)

clean:
	rm -r $(BUILD_DIR)/*

.PHONY: clean run compile
