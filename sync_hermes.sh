#!/bin/sh

# store pro file
cp hermes2d/hermes2d.pro .
cp hermes2d/omp/omp.h .

# remove directory
rm -rf hermes2d
rm -rf hermes_common

# create directories
mkdir hermes2d
mkdir hermes2d/omp
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

# remove weakform library
rm -rf hermes2d/src/weakform_library
# rm -rf hermes2d/include/weakform_library

cp ../hermes/COPYING hermes2d
cp ../hermes/AUTHORS hermes2d

# copy remove temp file
cp hermes2d.pro hermes2d
rm hermes2d.pro
cp omp.h hermes2d/omp
rm omp.h
