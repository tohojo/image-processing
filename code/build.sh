#!/bin/bash

dir=$(dirname "$0")
builddir="$dir/build"
CMAKEARGS="-Wdev -UTESTS -USEGMENTING -UFEATUREPOINTS -UCALIBRATION -UDISTORTION -URECTIFICATION -USTEREO -UPCA_773 -UFACE_NORMAL"

if [[ "$1" == "--debug" ]]; then
    CMAKEARGS="${CMAKEARGS} -DCMAKE_BUILD_TYPE=Debug"
    shift
fi

if [ ! -d "$builddir" ]; then
    mkdir "$builddir"
fi

cd "$builddir"
cmake $CMAKEARGS .. || exit
make "$@" || exit

RES=$?

[ ! -f image-processing ] && ln -sf src/image-processing image-processing

exit $RES
