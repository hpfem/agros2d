#!/bin/sh

# store pro file
cp hermes2d/hermes2d.pro .

# remove directory
rm -rf hermes2d
rm -rf hermes_common

# create directories
mkdir hermes2d
mkdir hermes2d/src
mkdir hermes2d/include
mkdir hermes_common
mkdir hermes_common/src
mkdir hermes_common/include

# copy files
cp -R ../hermes/hermes2d/src/* hermes2d/src/
cp -R ../hermes/hermes2d/include/* hermes2d/include/
cp -R ../hermes/hermes_common/src/* hermes_common/src/
cp -R ../hermes/hermes_common/include/* hermes_common/include/

# copy remove temp file
cp hermes2d.pro hermes2d
rm hermes2d.pro


