# Vulcalien's Library Makefile
# version 0.1.8
#
# This Makefile can create both
# Static and Shared libraries

# === DETECT OS ===
ifeq ($(OS),Windows_NT)
	CURRENT_OS := WINDOWS
else
	CURRENT_OS := UNIX
endif
TARGET_OS := $(CURRENT_OS)

# ========= EDIT HERE =========
OUT_FILENAME := luag-lib-1.4

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CPPFLAGS := -I../../include -MMD -MP

CFLAGS_STATIC := -Wall -pedantic
CFLAGS_SHARED := -fPIC -Wall -pedantic

ifeq ($(TARGET_OS),UNIX)
	# UNIX
	LDFLAGS := -shared -Llib
	LDLIBS  := -llua5.4
else ifeq ($(TARGET_OS),WINDOWS)
	# WINDOWS
	LDFLAGS := -shared -Llib
	LDLIBS  :=
endif
# =============================

# === OS SPECIFIC ===
ifeq ($(TARGET_OS),UNIX)
	CC := gcc

	OBJ_EXT    := .o
	STATIC_EXT := .a
	SHARED_EXT := .so
else ifeq ($(TARGET_OS),WINDOWS)
	CC := gcc

	OBJ_EXT    := .obj
	STATIC_EXT := -win.a
	SHARED_EXT := .dll
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

OBJ_STATIC_DIR := $(OBJ_DIR)/static
OBJ_SHARED_DIR := $(OBJ_DIR)/shared

OBJ_STATIC := $(SRC:$(SRC_DIR)/%.c=$(OBJ_STATIC_DIR)/%$(OBJ_EXT))
OBJ_SHARED := $(SRC:$(SRC_DIR)/%.c=$(OBJ_SHARED_DIR)/%$(OBJ_EXT))

OUT_STATIC := $(BIN_DIR)/$(OUT_FILENAME)$(STATIC_EXT)
OUT_SHARED := $(BIN_DIR)/$(OUT_FILENAME)$(SHARED_EXT)

# === TARGETS ===
.PHONY: all build-static build-shared clean

all: build-static build-shared
build-static: $(OUT_STATIC)
build-shared: $(OUT_SHARED)

clean:
	@$(RM) $(RMFLAGS) $(BIN_DIR) $(OBJ_DIR)

$(OUT_STATIC): $(OBJ_STATIC) | $(BIN_DIR)
	$(AR) rcs $@ $^

$(OUT_SHARED): $(OBJ_SHARED) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_STATIC_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%.c | $(OBJ_STATIC_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS_STATIC) -c $< -o $@

$(OBJ_SHARED_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%.c | $(OBJ_SHARED_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS_SHARED) -c $< -o $@

$(BIN_DIR) $(OBJ_STATIC_DIR) $(OBJ_SHARED_DIR):
	$(MKDIR) $(MKDIRFLAGS) "$@"

-include $(OBJ_STATIC:$(OBJ_EXT)=.d)
-include $(OBJ_SHARED:$(OBJ_EXT)=.d)
