#!/bin/sh

SCRIPT_PARENT="$(dirname "$0")"

LOCATION="system"
ACTION=0

# parse arguments and options
for i in "$@"; do
    case $i in
        install)
            if [ "$ACTION" = "0" ]; then
                ACTION="install"
            else
                ACTION=0
            fi
            ;;
        uninstall)
            if [ "$ACTION" = "0" ]; then
                ACTION="uninstall"
            else
                ACTION=0
            fi
            ;;

        -l|--local)
            LOCATION="local"
            ;;
    esac
done

# set paths
if [ "$LOCATION" = "local" ]; then
    BIN_PATH="$HOME/.local/bin/luag-console"

    if [ -z "$XDG_DATA_HOME" ]; then
        RES_PATH="$HOME/.local/share/luag-console"
    else
        RES_PATH="$XDG_DATA_HOME/luag-console"
    fi

    if [ -z "$XDG_CONFIG_HOME" ]; then
        CONFIG_PATH="$HOME/.config/luag-console"
    else
        CONFIG_PATH="$XDG_CONFIG_HOME/luag-console"
    fi

    DESKTOP_FILE_PATH="$HOME/.local/share/applications/net.vulcalien.LuagConsole.desktop"
elif [ "$LOCATION" = "system" ]; then
    BIN_PATH="/usr/bin/luag-console"
    RES_PATH="/usr/share/luag-console"
    CONFIG_PATH="/etc/luag-console"
    DESKTOP_FILE_PATH="/usr/share/applications/net.vulcalien.LuagConsole.desktop"
fi

# install/uninstall
if [ "$ACTION" = "install" ]; then
    echo Installing LuaG Console

    echo "Copying file: $BIN_PATH"
    cp "$SCRIPT_PARENT/bin/luag-console" "$BIN_PATH"

    echo "Copying directory: $RES_PATH"
    cp -r "$SCRIPT_PARENT/res" "$RES_PATH"

    echo "Creating directory: $CONFIG_PATH"
    mkdir "$CONFIG_PATH"

    echo "Copying file: $DESKTOP_FILE_PATH"
    cp "$SCRIPT_PARENT/desktop/net.vulcalien.LuagConsole.desktop" "$DESKTOP_FILE_PATH"
elif [ "$ACTION" = "uninstall" ]; then
    echo Uninstalling LuaG Console

    echo "Deleting file: $BIN_PATH"
    rm "$BIN_PATH"

    echo "Deleting directory: $RES_PATH"
    rm -r "$RES_PATH"

    echo "Deleting directory: $CONFIG_PATH"
    rm -r "$CONFIG_PATH"

    echo "Deleting file: $DESKTOP_FILE_PATH"
    rm "$DESKTOP_FILE_PATH"
else
    echo "Usage: $0 <install/uninstall> [--local]"
    exit
fi
