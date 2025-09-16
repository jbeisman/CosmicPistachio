#!/usr/bin/env bash

ORIGIN_DIR=`pwd`
if [ -d "build" ]; then 
  rm -rf "build"
fi
mkdir -p "install"
mkdir "build" ; cd "build"
cmake ..\
    -DCMAKE_INSTALL_PREFIX:FILEPATH=${ORIGIN_DIR}/install \
    -DENABLE_CUDA:BOOL=ON \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DCMAKE_CXX_FLAGS_INIT:STRING="-Wall -Wextra -Wpedantic"
make -j4 VERBOSE=1
make install
cd ${ORIGIN_DIR}
