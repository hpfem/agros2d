#!/bin/bash

rm ../agros2d_*
lrelease lang/*.ts
qhelpgenerator doc/help/agros2d.qhp
dpkg-buildpackage -sgpg -rfakeroot

