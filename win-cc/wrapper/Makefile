# Vulcalien's Executable Makefile
# version 0.1.4
#
# One Makefile for Unix and Windows
# Made for the 'gcc' compiler

# === DETECT OS ===
ifeq ($(OS),Windows_NT)
	CURRENT_OS := WINDOWS
else
	CURRENT_OS := UNIX
endif
TARGET_OS := $(CURRENT_OS)

# ========= EDIT HERE =========
OUT_FILENAME := luag-console

SRC_DIR := .
OBJ_DIR := obj
BIN_DIR := ../bin

CC := x86_64-w64-mingw32-gcc
TARGET_OS := WINDOWS

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -pedantic

LDFLAGS := -L../bin -Wl,--enable-auto-import
LDLIBS  := -lluag-console
# =============================

# === OS SPECIFIC ===
ifeq ($(TARGET_OS),UNIX)
	OBJ_EXT := .o
	OUT_EXT :=
else ifeq ($(TARGET_OS),WINDOWS)
	OBJ_EXT := .obj
	OUT_EXT := .exe
endif

ifeq ($(CURRENT_OS),UNIX)
	MKDIR      := mkdir
	MKDIRFLAGS := -p

	RM      := rm
	RMFLAGS := -rfv
else ifeq ($(CURRENT_OS),WINDOWS)
	MKDIR      := mkdir
	MKDIRFLAGS :=

	RM      := rmdir
	RMFLAGS := /Q /S
endif

# === OTHER ===
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%$(OBJ_EXT))
OUT := $(BIN_DIR)/$(OUT_FILENAME)$(OUT_EXT)

# === TARGETS ===
.PHONY: all run build clean

all: build

run:
	./$(OUT)

build: $(OUT)

clean:
	@$(RM) $(RMFLAGS) $(BIN_DIR) $(OBJ_DIR)

$(OUT): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	$(MKDIR) $(MKDIRFLAGS) "$@"

-include $(OBJ:$(OBJ_EXT)=.d)
