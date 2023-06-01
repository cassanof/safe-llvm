#!/bin/bash

# get by $1 the number of jobs to spawn and by $2 the build type
if [ $# -ne 2 ]; then
  echo "Usage: $0 <number of jobs (more = more speed, too much = crash computer)> <build type. Debug or Release>"
  exit 1
fi
# this script does not really use $1, but other scripts do, soooooo...


cd build
cmake -G "Ninja" -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_BUILD_TYPE="$2" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DLLVM_ENABLE_ASSERTIONS=On ../llvm

