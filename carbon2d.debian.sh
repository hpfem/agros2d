#!/bin/bash

rm ../carbon2d_*
dpkg-buildpackage -sgpg -rfakeroot

