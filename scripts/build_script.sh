#!/bin/bash

# get by $1 the number of jobs to spawn
if [ -z "$1" ]; then
  echo "Usage: $0 <number of jobs (more = more speed, too much = crash computer)>"
    exit 1
fi

cd build
cmake -G "Ninja" -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="ARM;Lanai;RISCV" -DCMAKE_BUILD_TYPE="Debug" -DLLVM_ENABLE_ASSERTIONS=On ../llvm
ninja -j $1
