#!/bin/bash

# set all environment variables here
# QT_PATH
# PREDICTOR_API_KEY
# GOOGLE_CUSTOM_SEARCH_ENGINE_ID (CSE_ID)
source env.sh

cmake -G"Ninja" -B build                       \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1        \
      -DCMAKE_PREFIX_PATH=${QT_PATH}           \
      -DPREDICTOR_API_KEY=${PREDICTOR_API_KEY} \
      -DCSE_ID=${CSE_ID}

ninja -C build
ln -sf build/compile_commands.json compile_commands.json

