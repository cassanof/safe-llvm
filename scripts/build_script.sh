#!/bin/bash

./scripts/build_common.sh $@
cd build
ninja -j $1
