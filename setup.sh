#!/bin/sh

SCRIPT_PARENT="$(dirname "$0")"

# functions
install_bin() {
    cp "$SCRIPT_PARENT/bin/luag-console" "$1"
}

uninstall_bin() {
    rm -f "$1"
}

install_res() {
    cp -r "$SCRIPT_PARENT/res" "$1"
}

uninstall_res() {
    rm -rf "$1"
}

install_config() {
    mkdir "$1"
}

uninstall_config() {
    rm -rf "$1"
}

INSTALL_LOCALLY=0
ACTION=0

# parse arguments and options
for i in "$@"; do
    case $i in
        install)
            if [ "$ACTION" -eq "0" ]; then
                ACTION="install"
            fi
            ;;
        uninstall)
            if [ "$ACTION" -eq "0" ]; then
                ACTION="uninstall"
            fi
            ;;

        -l|--local)
            INSTALL_LOCALLY=1
            ;;
    esac
done

# install/uninstall
if [ "$ACTION" = "install" ]; then
    echo Installing LuaG Console
elif [ "$ACTION" = "uninstall" ]; then
    echo Uninstalling LuaG Console
else
    echo "Usage: $0 <install/uninstall> [--local]"
    exit
fi

if [ "$INSTALL_LOCALLY" -eq 1 ]; then
    ${ACTION}_bin "$HOME/.local/bin/luag-console"

    if [ -z "$XDG_DATA_HOME" ]; then
        ${ACTION}_res "$HOME/.local/share/luag-console"
    else
        ${ACTION}_res "$XDG_DATA_HOME/luag-console"
    fi

    if [ -z "$XDG_CONFIG_HOME" ]; then
        ${ACTION}_config "$HOME/.config/luag-console"
    else
        ${ACTION}_config "$XDG_CONFIG_HOME/luag-console"
    fi
else
    ${ACTION}_bin "/usr/local/bin/luag-console"

    ${ACTION}_res "/usr/local/share/luag-console"
    ${ACTION}_config "/etc/luag-console"
fi
