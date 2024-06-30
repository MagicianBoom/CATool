#!/bin/bash

set -e

CA_TOOL_ROOT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
BUILD_DIR="$CA_TOOL_ROOT_DIR/obj"
CONFIG_FILE="$CA_TOOL_ROOT_DIR/catool.conf"
CONFIG_INC_FILE="$CA_TOOL_ROOT_DIR/catool_conf.h"

function cfg_to_inc()
{
    echo "" > $CONFIG_INC_FILE

    echo "#ifndef __CATOOL_CONF_H" >> $CONFIG_INC_FILE
    echo "#define __CATOOL_CONF_H" >> $CONFIG_INC_FILE
    echo "" >> $CONFIG_INC_FILE

    while IFS='=' read -r key value; do
        if [[ -n "$key" && ! "$key" =~ ^# ]]; then
            key=$(echo -n "$key" | xargs)
            value=$(echo -n "$value" | xargs)
            echo "#define $key ($value)" >> $CONFIG_INC_FILE
        fi
    done < $CONFIG_FILE

    echo "" >> $CONFIG_INC_FILE
    echo "#endif" >> $CONFIG_INC_FILE
}

function build_kernel_module()
{
    if [ -f "$CA_TOOL_ROOT_DIR/catool.ko" ]; then
        rm $CA_TOOL_ROOT_DIR/catool.ko
    fi

    local CATOOL_KERNEL_MODULE=$(grep -E "^CATOOL_KERNEL_MODULE=" "$CONFIG_FILE" | cut -d '=' -f 2)

    if [ "$CATOOL_KERNEL_MODULE" == "y" ]; then
        cd $CA_TOOL_ROOT_DIR/kernel_module
        make -s
        cp catool.ko $CA_TOOL_ROOT_DIR
        make -s clean
    fi
}

function build_catool()
{
    cd $CA_TOOL_ROOT_DIR

    cfg_to_inc

    if [ -f "$CA_TOOL_ROOT_DIR/catool" ]; then
        rm $CA_TOOL_ROOT_DIR/catool
    fi

    make -s
    cp $BUILD_DIR/catool $CA_TOOL_ROOT_DIR
    make -s clean
}

function build()
{
    build_kernel_module
    build_catool
    echo ""
    echo "Generate the catool.ko in the $CA_TOOL_ROOT_DIR/catool.ko"
    echo "Generate the catool in the $CA_TOOL_ROOT_DIR/catool"
}

function clean()
{
    cd $CA_TOOL_ROOT_DIR

    if [ -f "$CA_TOOL_ROOT_DIR/catool" ]; then
        rm $CA_TOOL_ROOT_DIR/catool
    fi

    if [ -f "$CA_TOOL_ROOT_DIR/catool.ko" ]; then
        rm $CA_TOOL_ROOT_DIR/catool.ko
    fi
}

function help()
{
    echo "USAGE:"
    echo "  ./build_run.sh                      build catool"
    echo "  ./build_run.sh clean                clean catool project object"
    echo "  ./build_run.sh help                 show cmd of build_run.sh"
}


trap 'onCtrlC' INT
function onCtrlC () {
        kill -9 ${do_sth_pid} ${progress_pid}
        echo
        echo 'Ctrl+C is captured'
        exit 1
}

function progress() {
        #进度条程序
        local main_pid=$1
        local length=50
        local ratio=1
        while [ "$(ps -p ${main_pid} | wc -l)" -ne "1" ] ; do
                mark='>'
                progress_bar=
                for i in $(seq 1 "${length}"); do
                        if [ "$i" -gt "${ratio}" ] ; then
                                mark='-'
                        fi
                        progress_bar="${progress_bar}${mark}"
                done
                printf "Build: ${progress_bar}\r"
                ratio=$((ratio+1))
                if [ "${ratio}" -gt "${length}" ] ; then
                        ratio=1
                fi
                sleep 0.1
        done
}

function main()
{
    CMD=$1

    case "$CMD" in
    clean)
        clean
        ;;
    help)
        help
        ;;
    *)
        build
        ;;
    esac
}

main $@ &
do_sth_pid=$(jobs -p | tail -1)

progress "${do_sth_pid}" &
progress_pid=$(jobs -p | tail -1)

wait "${do_sth_pid}"
printf "Build: done                \n"
