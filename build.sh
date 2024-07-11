#!/bin/bash

# set all environment variables here
# QT_PATH
source env.sh

cmake -G"Ninja" -B build                \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -DCMAKE_PREFIX_PATH=${QT_PATH}    \
      -DPREDICTOR_API_KEY=${PREDICTOR_API_KEY}


ninja -C build
ln -sf build/compile_commands.json compile_commands.json

