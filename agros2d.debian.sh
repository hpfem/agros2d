#!/bin/sh

case "$1" in
  -h )
    qhelpgenerator ./doc/help/agros2d.qhp -o ./doc/help/agros2d.qch
    ;;
  -l )
    lrelease ./lang/*.ts ./lang/*.qm
    ;;
  -c )
    if qmake ./agros2d.pro ; then make ; fi
    ;;
  -p )
    rm ../agros2d_*
    dpkg-buildpackage -sgpg -rfakeroot
    ;;
  -a )
    qhelpgenerator ./doc/help/agros2d.qhp -o ./doc/help/agros2d.qch
    lrelease ./lang/*.ts ./lang/*.qm
    if qmake ./agros2d.pro ; then make ; fi
    rm ../agros2d_*
    dpkg-buildpackage -sgpg -rfakeroot
    ;;
  * )
    echo "Usage: agros2d.sh  [-h generate help files] [-l generate language files] [-c compile] [-p bild package] [-a all operations]"
    ;;
esac
