#!/bin/sh

case "$1" in
	help )
		qcollectiongenerator ./doc/help/Agros2D.qhcp -o ./doc/help/Agros2D.qhc
		;;
	lang )
		lrelease ./lang/*.ts
		;;
	comp )
		if qmake ./agros2d.pro ; then make ; fi
		;;
	pack )
		rm ../agros2d_*
		dpkg-buildpackage -sgpg -rfakeroot
		;;
	all )
		qcollectiongenerator ./doc/help/Agros2D.qhcp -o ./doc/help/Agros2D.qhc
		lrelease ./lang/*.ts ./lang/*.qm
		if qmake ./agros2d.pro ; then make ; fi
		rm ../agros2d_*
		dpkg-buildpackage -sgpg -rfakeroot
		;;
	* )
		echo "Usage: agros2d.sh  [help - generate help files] [lang - generate language files] [comp - compile] [pack - build package] [all - all operations]"
		;;
esac
