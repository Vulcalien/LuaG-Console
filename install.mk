ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

# directories
DATAROOTDIR := $(PREFIX)/share
DATADIR     := $(DATAROOTDIR)

EXEC_PREFIX := $(PREFIX)
BINDIR      := $(EXEC_PREFIX)/bin
LIBDIR      := $(EXEC_PREFIX)/lib

# install command
INSTALL         := install -D
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA    := $(INSTALL) -m 664

# remove command
REMOVE      := rm -fv
REMOVE_FILE := $(REMOVE)
REMOVE_DIR  := $(REMOVE) -r

# copy directory command
COPY_DIR := cp -Trv

APPID := net.vulcalien.LuagConsole

.PHONY: install uninstall

install:
	$(INSTALL_PROGRAM) bin/luag-console\
		$(DESTDIR)$(BINDIR)/luag-console
	$(INSTALL_DATA) desktop/$(APPID).desktop\
		$(DESTDIR)$(DATADIR)/applications/$(APPID).desktop
	$(COPY_DIR) res $(DESTDIR)$(DATADIR)/luag-console

uninstall:
	$(REMOVE_FILE) $(DESTDIR)$(BINDIR)/luag-console
	$(REMOVE_FILE) $(DESTDIR)$(DATADIR)/applications/$(APPID).desktop
	$(REMOVE_DIR) $(DESTDIR)$(DATADIR)/luag-console
