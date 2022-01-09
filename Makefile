# Vulcalien's Executable Makefile
# version 0.1.3
#
# One Makefile for Unix and Windows
# Made for the 'gcc' compiler
#
# To cross-compile:
#     make build TARGET_OS=<UNIX or WINDOWS> CC=<cross compiler>

# === DETECT OS ===
ifeq ($(OS),Windows_NT)
	CURRENT_OS := WINDOWS
else
	CURRENT_OS := UNIX
endif
TARGET_OS := $(CURRENT_OS)

# ========= EDIT HERE =========
OUT_FILENAME := luag-console

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -pedantic

ifeq ($(TARGET_OS),UNIX)
	# UNIX
	LDFLAGS := -Llib -rdynamic
	LDLIBS  := -lSDL2 -lSDL2_image -ldl -llua5.4
else ifeq ($(TARGET_OS),WINDOWS)
	# WINDOWS
	LDFLAGS := -Llib
	LDLIBS  :=
endif
# =============================

# === OS SPECIFIC ===
ifeq ($(TARGET_OS),UNIX)
	CC := gcc

	OBJ_EXT := .o
	OUT_EXT :=
else ifeq ($(TARGET_OS),WINDOWS)
	CC := gcc

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

all: build run

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
