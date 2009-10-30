#!/bin/sh

case "$1" in
	help )
		if sphinx-build -b qthelp ./doc/source ./doc/help
		then
			qcollectiongenerator ./doc/help/Agros2D.qhcp -o ./doc/help/Agros2D.qhc
		fi
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
		if sphinx-build -b qthelp ./doc/source ./doc/help
		then
			qcollectiongenerator ./doc/help/Agros2D.qhcp -o ./doc/help/Agros2D.qhc
		fi
		lrelease ./lang/*.ts ./lang/*.qm
		if qmake ./agros2d.pro ; then make ; fi
		rm ../agros2d_*
		dpkg-buildpackage -sgpg -rfakeroot
		;;
	* )
		echo "Usage: agros2d.sh  [help - generate help files] [lang - generate language files] [comp - compile] [pack - build package] [all - all operations]"
		;;
esac
