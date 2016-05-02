#!/bin/bash

ROOT=$PWD

mkdir -p build/control_loop_lab
cd build/control_loop_lab
qmake CXX=$CXX ../../test/control_loop_lab/control_loop_lab.pro && make && ./control_loop_lab
if [[ $? -ne 0 ]] ; then
    exit 1
fi

cd $ROOT

mkdir -p build/maintenance_lab
cd build/maintenance_lab
qmake CXX=$CXX ../../test/maintenance_lab/maintenance_lab.pro && make && ./maintenance_lab
if [[ $? -ne 0 ]] ; then
    exit 1
fi

cd $ROOT

mkdir -p build/control_loop_test
cd build/control_loop_test
qmake CXX=$CXX ../../test/control_loop_test/control_loop_test.pro && make && ./control_loop_test
if [[ $? -ne 0 ]] ; then
    exit 1
fi
