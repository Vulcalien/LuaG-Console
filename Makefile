# Vulcalien's Executable Makefile
# version 0.2.0
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

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_SUBDIRS :=

CC := gcc

LIBS := sdl2 SDL2_image SDL2_mixer lua5.4 libarchive

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -pedantic -Wno-format-truncation\
            `pkg-config --cflags-only-other $(LIBS)`

ifeq ($(TARGET_OS),UNIX)
	# UNIX
	ifdef LINK_STATIC
		LDFLAGS := -Llib -static -rdynamic
		LDLIBS  := `pkg-config --static --libs-only-l $(LIBS)`\
		           -lpthread -ldl
	else
		LDFLAGS := -Llib -rdynamic
		LDLIBS  := `pkg-config --libs-only-l $(LIBS)`\
		           -lpthread -ldl
	endif
else ifeq ($(TARGET_OS),WINDOWS)
	ifeq ($(CURRENT_OS),WINDOWS)
		# WINDOWS
		LDFLAGS := -Llib
		LDLIBS  :=
	else ifeq ($(CURRENT_OS),UNIX)
		# UNIX to WINDOWS cross-compile
		CC := x86_64-w64-mingw32-gcc

		LDFLAGS := -Llib
		LDLIBS  :=
	endif
endif
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
SRC := $(wildcard $(SRC_DIR)/*.c)\
       $(foreach DIR,$(SRC_SUBDIRS),$(wildcard $(SRC_DIR)/$(DIR)/*.c))
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%$(OBJ_EXT))
OUT := $(BIN_DIR)/$(OUT_FILENAME)$(OUT_EXT)

OBJ_DIRECTORIES := $(OBJ_DIR) $(foreach DIR,$(SRC_SUBDIRS),$(OBJ_DIR)/$(DIR))

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

$(OBJ_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%.c | $(OBJ_DIRECTORIES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIRECTORIES):
	$(MKDIR) $(MKDIRFLAGS) "$@"

-include $(OBJ:$(OBJ_EXT)=.d)
