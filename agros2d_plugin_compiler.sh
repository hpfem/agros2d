#!/bin/sh

pluginPath="plugins/"

# change directory
cd "$pluginPath""$1"

# run qmake
qmake

# make plugin
make clean
make -j2
