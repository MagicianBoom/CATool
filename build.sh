#!/bin/bash

set -e

CA_TOOL_ROOT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
BUILD_DIR="$CA_TOOL_ROOT_DIR/obj"
CONFIG_FILE="$CA_TOOL_ROOT_DIR/catool.conf"
CONFIG_INC_FILE="$CA_TOOL_ROOT_DIR/catool_conf.h"

cfg_to_inc()
{
    echo "" > $CONFIG_INC_FILE

    echo "#ifndef __CATOOL_CONF_H" >> $CONFIG_INC_FILE
    echo "#define __CATOOL_CONF_H" >> $CONFIG_INC_FILE
    echo "" >> $CONFIG_INC_FILE

    while IFS='=' read -r key value; do
        if [[ -n "$key" && ! "$key" =~ ^# ]]; then
            key=$(echo -n "$key" | xargs)
            value=$(echo -n "$value" | xargs)
            echo "#define $key $value" >> $CONFIG_INC_FILE
        fi
    done < $CONFIG_FILE

    echo "" >> $CONFIG_INC_FILE
    echo "#endif" >> $CONFIG_INC_FILE
}

build_kernel_module()
{
    local CATOOL_KERNEL_MODULE=$(grep -E "^CATOOL_KERNEL_MODULE=" "$CONFIG_FILE" | cut -d '=' -f 2)

    if [ -f "$CA_TOOL_ROOT_DIR/catool.ko" ]; then
        rm $CA_TOOL_ROOT_DIR/catool.ko
    fi

    if [ "$CATOOL_KERNEL_MODULE" == "y" ]; then
        cd $CA_TOOL_ROOT_DIR/kernel_module
        make -s
        cp catool.ko $CA_TOOL_ROOT_DIR
        make -s clean
    fi
}

build_catool()
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

mv_catool_to_sys()
{
    if [[ -d "/usr/local/bin" ]]; then
        local bin_target_dir=/usr/local/bin
    elif [[ -d "/usr/bin" ]]; then
        local bin_target_dir=/usr/bin
    fi

    if [[ -d "/usr/local/sbin" ]]; then
        local sbin_target_dir=/usr/local/sbin
    elif [[-d "/usr/sbin" ]]; then
        local sbin_target_dir=/usr/sbin
    fi

    if [[ -n "${bin_target_dir}" ]]; then
        if [[ -f "${bin_target_dir}/catool" ]]; then
            rm ${bin_target_dir}/catool
        fi

        ln -s $CA_TOOL_ROOT_DIR/catool ${bin_target_dir}/catool
        echo "Create a catool software link to ${bin_target_dir}"
    fi

    if [[ -n "${sbin_target_dir}" ]]; then
        if [[ -f "${sbin_target_dir}/catool" ]]; then
            rm ${sbin_target_dir}/catool
        fi

        ln -s $CA_TOOL_ROOT_DIR/catool ${sbin_target_dir}/catool
        echo "Create a catool software link to ${sbin_target_dir}"
    fi
}

build()
{
    build_kernel_module
    build_catool
    echo ""
    echo "Generate the files 'catool' and 'catool.ko' (if available) in the project root directory [$CA_TOOL_ROOT_DIR]."
    mv_catool_to_sys
}

clean()
{
    cd $CA_TOOL_ROOT_DIR

    if [ -f "$CA_TOOL_ROOT_DIR/catool" ]; then
        rm $CA_TOOL_ROOT_DIR/catool
    fi

    if [ -f "$CA_TOOL_ROOT_DIR/catool.ko" ]; then
        rm $CA_TOOL_ROOT_DIR/catool.ko
    fi
}

help()
{
    echo "USAGE:"
    echo "  ./build_run.sh                      build catool"
    echo "  ./build_run.sh clean                clean catool project object"
    echo "  ./build_run.sh help                 show cmd of build_run.sh"
}


trap 'onCtrlC' INT
onCtrlC () {
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

main()
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
