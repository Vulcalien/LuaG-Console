# Download and prepare dependencies needed
# for 'UNIX to WINDOWS' cross-compilation.

PREFIX := lib

LIBS := SDL2 SDL2_image SDL2_mixer lua54 libarchive dlfcn-win32

LIB_PATHS := $(foreach LIB,$(LIBS),$(PREFIX)/$(LIB))

.PHONY: all clean

all: $(LIB_PATHS)

$(PREFIX)/SDL2:
	mkdir -p $@
	curl -s -L "https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-mingw.tar.gz"\
		| tar -xvz --strip-components 1 -C $@

$(PREFIX)/SDL2_image:
	mkdir -p $@
	curl -s -L "https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-mingw.tar.gz"\
		| tar -xvz --strip-components 1 -C $@

$(PREFIX)/SDL2_mixer:
	mkdir -p $@
	curl -s -L "https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-mingw.tar.gz"\
		| tar -xvz --strip-components 1 -C $@

$(PREFIX)/lua54:
	mkdir -p $@
	wget -O lua54_tmp.zip "https://sourceforge.net/projects/luabinaries/files/5.4.2/Windows%20Libraries/Dynamic/lua-5.4.2_Win64_dll17_lib.zip/download"
	unzip lua54_tmp.zip -d $@
	rm -f lua54_tmp.zip

$(PREFIX)/libarchive:
	mkdir -p $@
	curl -s -L -o libarchive_tmp.zip "https://libarchive.org/downloads/libarchive-v3.7.2-amd64.zip"
	unzip libarchive_tmp.zip -d $(PREFIX)
	rm -f libarchive_tmp.zip

$(PREFIX)/dlfcn-win32:
	mkdir -p $@ $@/source
	curl -s -L "https://github.com/dlfcn-win32/dlfcn-win32/archive/refs/tags/v1.4.1.tar.gz"\
		| tar -xvz --strip-components 1 -C $@/source
	x86_64-w64-mingw32-gcc -fPIC -shared $@/source/src/dlfcn.c -o $@/dlfcn.dll
	mv $@/source/src/dlfcn.h $@/dlfcn.h

clean:
	rm -rfv $(LIB_PATHS)
