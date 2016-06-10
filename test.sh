#!/bin/bash

mkdir -p build/control_loop_test
cd build/control_loop_test
qmake CXX=$CXX ../../test/control_loop_test/control_loop_test.pro && make && ./control_loop_test
if [[ $? -ne 0 ]] ; then
    exit 1
fi
