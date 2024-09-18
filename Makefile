# BUILD SYSTEM
# LINUX MAKEFILE

# VARIABLES 
CC := gcc
DEBUGGER := gdb
STD := c11

DYNAMIC_LIB := third-party/so

LIBRARIES := -Lthird-party \
			 -L${DYNAMIC_LIB} -Wl,-rpath=${DYNAMIC_LIB} \
			 -lcglm \
			 -lm \
			 -lassimp \
			 -lglfw \
			 -lcimgui \
			 -lcimgui_backends

DEFINES := -DENABLE_ASSERT=1
WARNINGS := -Wall -Wextra

INCLUDE_DIRS := -Isrc -Ithird-party

C_SOURCES 	= $(shell find src -iname *.c)
C_HEADERS 	= $(shell find src -iname *.h)
LIB_SOURCES = $(shell find third-party/src -iname *.c)
LIB_HEADERS = $(shell find third-party -iname *.h)

TEST_FLAGS := -DMTEST=1
DEBUG_FLAGS := -DMDEBUG=1

FLAGS := ${DEBUG_FLAGS} -g -std=${STD} ${LIBRARIES} ${DEFINES} ${WARNINGS} ${C_SOURCES} ${LIB_SOURCES} ${C_HEADERS} ${INCLUDE_DIRS}

BUILD_DIR := build
EXEC_NAME := out

# ROUTINES
all:
	${CC} ${FLAGS} -o ${BUILD_DIR}/${EXEC_NAME}
	${BUILD_DIR}/${EXEC_NAME}

debug:
	${CC} ${FLAGS} -o ${BUILD_DIR}/${EXEC_NAME}
	${DEBUGGER} ${BUILD_DIR}/${EXEC_NAME}

compile:
	${CC} ${FLAGS} -o ${BUILD_DIR}/${EXEC_NAME}

test:
	${CC} ${FLAGS} ${TEST_FLAGS} -o ${BUILD_DIR}/${EXEC_NAME}
	${DEBUGGER} ${BUILD_DIR}/${EXEC_NAME}

run:
	${BUILD_DIR}/${EXEC_NAME}

num_lines:
	cat $(shell find shader -iname *.glsl) $(shell find shader -iname *.comp) ${C_SOURCES} ${C_HEADERS} | wc -l

clean:
	rm ./${BUILD_DIR}/*
