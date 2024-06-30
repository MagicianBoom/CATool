#!/bin/bash

set -e

LY_TOOL_ROOT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
BUILD_DIR="$LY_TOOL_ROOT_DIR/obj"
CONFIG_FILE="$LY_TOOL_ROOT_DIR/catool.conf"
CONFIG_INC_FILE="$LY_TOOL_ROOT_DIR/catool_conf.h"

function cfg_to_inc()
{
    echo "" > $CONFIG_INC_FILE

    echo "#ifndef __CATOOL_CONF_H" >> $CONFIG_INC_FILE
    echo "#define __CATOOL_CONF_H" >> $CONFIG_INC_FILE
    echo "" >> $CONFIG_INC_FILE

    # 读取配置文件并生成宏定义
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
    if [ -f "$LY_TOOL_ROOT_DIR/catool.ko" ]; then
        rm $LY_TOOL_ROOT_DIR/catool.ko
    fi

    cd $LY_TOOL_ROOT_DIR/kernel_module
    make -s
    cp catool.ko $LY_TOOL_ROOT_DIR
    make -s clean
}

function build_catool()
{
    cd $LY_TOOL_ROOT_DIR

    cfg_to_inc

    if [ -f "$LY_TOOL_ROOT_DIR/catool" ]; then
        rm $LY_TOOL_ROOT_DIR/catool
    fi

    make -s
    cp $BUILD_DIR/catool $LY_TOOL_ROOT_DIR
    make -s clean
}

function build()
{
    # build_kernel_module
    build_catool
    echo ""
    echo "Generate the catool.ko in the $LY_TOOL_ROOT_DIR/catool.ko"
    echo "Generate the catool in the $LY_TOOL_ROOT_DIR/catool"
}

function clean()
{
    cd $LY_TOOL_ROOT_DIR

    if [ -f "$LY_TOOL_ROOT_DIR/catool" ]; then
        rm $LY_TOOL_ROOT_DIR/catool
    fi

    if [ -f "$LY_TOOL_ROOT_DIR/catool.ko" ]; then
        rm $LY_TOOL_ROOT_DIR/catool.ko
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
        #捕获CTRL+C，当脚本被ctrl+c的形式终止时同时终止程序的后台进程
        kill -9 ${do_sth_pid} ${progress_pid}
        echo
        echo 'Ctrl+C is captured'
        exit 1
}

progress() {
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
