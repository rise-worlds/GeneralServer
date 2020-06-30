#!/usr/bin/env bash
set -eo pipefail

[[ -z "${ARCH}" ]] && export ARCH=$( uname )
[[ -z "${OS_NAME}" ]] && export OS_NAME=$( cat /etc/os-release | grep ^NAME | cut -d'=' -f2 | sed 's/\"//gI' )

if [[ $ARCH == "Linux" ]]; then
    case "$OS_NAME" in
        "CentOS Linux")
        ;;
        "Ubuntu" | "Linux Mint")
        sudo apt update
        sudo apt install -y git build-essential clang llvm cmake gcc libboost-all-dev libssl-dev libgmp-dev libicu-dev
        ;;
    esac
fi

mkdir -p build
cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
make -j8