# Vulcalien's Library Makefile
# version 0.3.0

# === Detect OS ===
ifeq ($(OS),Windows_NT)
    CURRENT_OS := WINDOWS
else
    CURRENT_OS := UNIX
endif
TARGET_OS := $(CURRENT_OS)

# === Basic Info ===
ifeq ($(OUT_FILENAME),)
    $(error OUT_FILENAME must be definend)
endif

SRC_DIRS := .

OBJ_DIR := obj
BIN_DIR := bin

LIBS := sdl2 SDL2_image SDL2_mixer lua5.4 libarchive

# === C Flags ===
CPPFLAGS := -I../../include -MMD -MP

CFLAGS_STATIC := -Wall -pedantic
CFLAGS_SHARED := -Wall -pedantic -fPIC

# list libraries and add C flags for dependencies
ifeq ($(TARGET_OS),UNIX)
    # UNIX
    LIBS := sdl2 SDL2_image SDL2_mixer lua5.4 libarchive

    CPPFLAGS      += `pkg-config --cflags-only-I $(LIBS)`
    CFLAGS_SHARED += `pkg-config --cflags-only-other $(LIBS)`
else ifeq ($(TARGET_OS),WINDOWS)
    ifeq ($(CURRENT_OS),WINDOWS)
        # WINDOWS
        CPPFLAGS      +=
        CFLAGS_SHARED +=
    else ifeq ($(CURRENT_OS),UNIX)
        # UNIX to WINDOWS cross-compile
        WIN_CC_DIR := ../../win-cc/lib

        CPPFLAGS += -I$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/include/SDL2 \
                    -I$(WIN_CC_DIR)/SDL2_image/x86_64-w64-mingw32/include/SDL2 \
                    -I$(WIN_CC_DIR)/SDL2_mixer/x86_64-w64-mingw32/include/SDL2 \
                    -I$(WIN_CC_DIR)/libarchive/include \
                    -I$(WIN_CC_DIR)/lua54/include \
                    -I$(WIN_CC_DIR)/dlfcn-win32
        CFLAGS_SHARED += `$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/bin/sdl2-config --cflags`
    endif
endif

# === Linker Flags ===
ifeq ($(TARGET_OS),UNIX)
    # UNIX
    LDFLAGS := -shared \
               `pkg-config --libs-only-L $(LIBS)`
    LDLIBS  := `pkg-config --libs-only-l $(LIBS)`
else ifeq ($(TARGET_OS),WINDOWS)
    ifeq ($(CURRENT_OS),WINDOWS)
        # WINDOWS
        LDFLAGS := -shared
        LDLIBS  :=
    else ifeq ($(CURRENT_OS),UNIX)
        # UNIX to WINDOWS cross-compile
        LDFLAGS := -shared -L../../bin \
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
        LDLIBS := -l:luag-console.lib -lpthread \
                  -lSDL2_image -lSDL2_mixer -ldlfcn -llua54 -larchive \
                  `$(WIN_CC_DIR)/SDL2/x86_64-w64-mingw32/bin/sdl2-config --libs`
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
    STATIC_EXT := a
    SHARED_EXT := so
else ifeq ($(TARGET_OS),WINDOWS)
    OBJ_EXT    := obj
    STATIC_EXT := win.a
    SHARED_EXT := dll
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

OBJ_STATIC_DIR := $(OBJ_DIR)/static
OBJ_SHARED_DIR := $(OBJ_DIR)/shared

# list of source files
SRC := $(foreach DIR,$(SRC_DIRS),\
         $(foreach EXT,$(SRC_EXT),\
           $(wildcard $(DIR)/*.$(EXT))))

# list of object files
OBJ_STATIC := $(SRC:%=$(OBJ_STATIC_DIR)/%.$(OBJ_EXT))
OBJ_SHARED := $(SRC:%=$(OBJ_SHARED_DIR)/%.$(OBJ_EXT))

# list of object directories
OBJ_STATIC_DIRS := $(SRC_DIRS:%=$(OBJ_STATIC_DIR)/%)
OBJ_SHARED_DIRS := $(SRC_DIRS:%=$(OBJ_SHARED_DIR)/%)

# output files
OUT_STATIC := $(BIN_DIR)/$(OUT_FILENAME).$(STATIC_EXT)
OUT_SHARED := $(BIN_DIR)/$(OUT_FILENAME).$(SHARED_EXT)

# === Targets ===

.PHONY: all build-static build-shared clean

all: build-shared

build-static: $(OUT_STATIC)
build-shared: $(OUT_SHARED)

clean:
	@$(RM) $(RMFLAGS) $(BIN_DIR) $(OBJ_DIR)

# generate static library file
$(OUT_STATIC): $(OBJ_STATIC) | $(BIN_DIR)
	$(AR) rcs $@ $^

# generate shared library file
$(OUT_SHARED): $(OBJ_SHARED) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# compile .c files for static library
$(OBJ_STATIC_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_STATIC_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS_STATIC) -c $< -o $@

# compile .c files for shared library
$(OBJ_SHARED_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_SHARED_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS_SHARED) -c $< -o $@

# create directories
$(BIN_DIR) $(OBJ_STATIC_DIRS) $(OBJ_SHARED_DIRS):
	$(MKDIR) $(MKDIRFLAGS) "$@"

-include $(OBJ_STATIC:.$(OBJ_EXT)=.d)
-include $(OBJ_SHARED:.$(OBJ_EXT)=.d)
