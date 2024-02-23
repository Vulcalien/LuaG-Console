# Vulcalien's Executable Makefile
# version 0.3.0

# === Detect OS ===
ifeq ($(OS),Windows_NT)
    CURRENT_OS := WINDOWS
else
    CURRENT_OS := UNIX
endif
TARGET_OS := $(CURRENT_OS)

# === Basic Info ===
OUT_FILENAME := luag-console

SRC_DIRS := src src/data-structs

OBJ_DIR := obj
BIN_DIR := bin

# === C Flags ===
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -pedantic -Wno-format-truncation

# list libraries and add C flags for dependencies
ifeq ($(TARGET_OS),UNIX)
    # UNIX
    LIBS := sdl2 SDL2_image SDL2_mixer lua5.4 libarchive

    CPPFLAGS += `pkg-config --cflags-only-I $(LIBS)`
    CFLAGS   += `pkg-config --cflags-only-other $(LIBS)`
else ifeq ($(TARGET_OS),WINDOWS)
    ifeq ($(CURRENT_OS),WINDOWS)
        # WINDOWS
        CPPFLAGS +=
        CFLAGS   +=
    else ifeq ($(CURRENT_OS),UNIX)
        # UNIX to WINDOWS cross-compile
        WIN_CC_DIR := win-cc/lib

        CPPFLAGS += -I$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/include/SDL2 \
                    -I$(WIN_CC_DIR)/SDL2_image/x86_64-w64-mingw32/include/SDL2 \
                    -I$(WIN_CC_DIR)/SDL2_mixer/x86_64-w64-mingw32/include/SDL2 \
                    -I$(WIN_CC_DIR)/libarchive/include \
                    -I$(WIN_CC_DIR)/lua54/include \
                    -I$(WIN_CC_DIR)/dlfcn-win32
        CFLAGS   += `$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/bin/sdl2-config --cflags`
    endif
endif

# === Linker Flags ===
ifeq ($(TARGET_OS),UNIX)
    ifdef LINK_STATIC
        LDFLAGS := -static -rdynamic\
                   `pkg-config --static --libs-only-L $(LIBS)`
        LDLIBS  := `pkg-config --static --libs-only-l $(LIBS)` \
                   -lpthread -ldl
    else
        LDFLAGS := -rdynamic\
                   `pkg-config --libs-only-L $(LIBS)`
        LDLIBS  := `pkg-config --libs-only-l $(LIBS)` \
                   -lpthread -ldl
    endif
else ifeq ($(TARGET_OS),WINDOWS)
    ifeq ($(CURRENT_OS),WINDOWS)
        # WINDOWS
        LDFLAGS :=
        LDLIBS  :=
    else ifeq ($(CURRENT_OS),UNIX)
        # UNIX to WINDOWS cross-compile
        LDFLAGS := -Wl,--export-all-symbols \
                   -Wl,--out-implib,$(BIN_DIR)/$(OUT_FILENAME).lib \
                   -L$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/bin \
                   -L$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/lib \
                   -L$(WIN_CC_DIR)/SDL2_image/x86_64-w64-mingw32/bin \
                   -L$(WIN_CC_DIR)/SDL2_image/x86_64-w64-mingw32/lib \
                   -L$(WIN_CC_DIR)/SDL2_mixer/x86_64-w64-mingw32/bin \
                   -L$(WIN_CC_DIR)/SDL2_mixer/x86_64-w64-mingw32/lib \
                   -L$(WIN_CC_DIR)/libarchive/bin \
                   -L$(WIN_CC_DIR)/libarchive/lib \
                   -L$(WIN_CC_DIR)/lua54 \
                   -L$(WIN_CC_DIR)/dlfcn-win32
        LDLIBS  := -lSDL2_image -lSDL2_mixer -ldlfcn -llua54 -larchive \
                   `$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/bin/sdl2-config --libs` \
                   -lpthread
    endif
endif

# === Compilers ===
ifeq ($(TARGET_OS),UNIX)
    # UNIX
    CC := gcc
else ifeq ($(TARGET_OS),WINDOWS)
    ifeq ($(CURRENT_OS),WINDOWS)
        # WINDOWS
        CC := gcc
    else ifeq ($(CURRENT_OS),UNIX)
        # UNIX to WINDOWS cross-compile
        CC := x86_64-w64-mingw32-gcc
    endif
endif

# === OS Specific ===
ifeq ($(TARGET_OS),UNIX)
    OBJ_EXT    := o
    OUT_SUFFIX :=
else ifeq ($(TARGET_OS),WINDOWS)
    OBJ_EXT    := obj
    OUT_SUFFIX := .exe
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

# === Resources ===

# list of source file extensions
SRC_EXT := c

# list of source files
SRC := $(foreach DIR,$(SRC_DIRS),\
         $(foreach EXT,$(SRC_EXT),\
           $(wildcard $(DIR)/*.$(EXT))))

# list of object files
OBJ := $(SRC:%=$(OBJ_DIR)/%.$(OBJ_EXT))

# list of object directories
OBJ_DIRS := $(SRC_DIRS:%=$(OBJ_DIR)/%)

# output file
OUT := $(BIN_DIR)/$(OUT_FILENAME)$(OUT_SUFFIX)

# === Targets ===

.PHONY: all run build clean

all: build run

run:
	./$(OUT)

build: $(OUT)

clean:
	@$(RM) $(RMFLAGS) $(BIN_DIR) $(OBJ_DIR)

# generate output file
$(OUT): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# compile .c files
$(OBJ_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# create directories
$(BIN_DIR) $(OBJ_DIRS):
	$(MKDIR) $(MKDIRFLAGS) "$@"

-include $(OBJ:.$(OBJ_EXT)=.d)
-include install.mk
