#!/bin/bash

# set all environment variables here
# QT_PATH
# PREDICTOR_API_KEY
# DICTIONARY_API_KEY
# GOOGLE_CUSTOM_SEARCH_ENGINE_ID (CSE_ID)
# ELEVEN_LABS_API_KEY
source env.sh

function build {
    cmake -G"Ninja" -B build                         \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1            \
        -DCMAKE_PREFIX_PATH=${QT_PATH}               \
        -DPREDICTOR_API_KEY=${PREDICTOR_API_KEY}     \
        -DDICTIONARY_API_KEY=${DICTIONARY_API_KEY}   \
        -DCSE_ID=${CSE_ID}                           \
        -DELEVEN_LABS_API_KEY=${ELEVEN_LABS_API_KEY}

    ninja -C build
    ln -sf build/compile_commands.json compile_commands.json
}

function clean {
    rm -fR build
}

case $1 in
    "-c"|"--clean")
        clean
        ;;
    "-b"|"--build")
        build
        ;;
    *)
        build
        ;;
esac
