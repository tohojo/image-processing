#!/bin/bash

dir=$(dirname "$0")
builddir="$dir/build"
CMAKEARGS="-Wdev"

if [[ "$1" == "--debug" ]]; then
    CMAKEARGS="${CMAKEARGS} -DCMAKE_BUILD_TYPE=Debug"
    shift
fi

if [ ! -d "$builddir" ]; then
    mkdir "$builddir"
fi

cd "$builddir"
cmake $CMAKEARGS ..
make "$@"