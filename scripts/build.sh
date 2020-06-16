#!/bin/bash

[[ -z "${ARCH}" ]] && export ARCH=$( uname )

if [[ $NAME == "Ubuntu" ]]; then
    sudo apt update
    sudo apt install -y git build-essential clang llvm cmake gcc libboost-all-dev libssl-dev libgmp-dev libicu-dev
fi

mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
make -j8