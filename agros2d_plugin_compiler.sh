#!/bin/sh

pluginPath="plugins/"

# generate plugin
./agros2d_generator "$1"

# change directory
cd "$pluginPath""$1"

# run qmake
qmake

# make plugin
make clean
make -j2
