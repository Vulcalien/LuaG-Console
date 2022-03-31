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

SRC_DIR := ../src
OBJ_DIR := obj
BIN_DIR := bin

CC := x86_64-w64-mingw32-gcc
TARGET_OS := WINDOWS

CPPFLAGS := -I../include -Iinclude -MMD -MP
CFLAGS   := -Wall -pedantic -Wno-format-truncation

LDFLAGS := -Llib
LDLIBS  := `SDL2-2.0.20/x86_64-w64-mingw32/bin/sdl2-config --libs`\
		   -lSDL2_image -lSDL2_mixer -ldlfcn -llua54 -larchive
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
	x86_64-w64-mingw32-dlltool $@ -l $@.lib --export-all-symbols

$(OBJ_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	$(MKDIR) $(MKDIRFLAGS) "$@"

-include $(OBJ:$(OBJ_EXT)=.d)