#!/bin/sh

case "$1" in
	help )
		qhelpgenerator ./doc/help/agros2d.qhp -o ./doc/help/agros2d.qch
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
		qhelpgenerator ./doc/help/agros2d.qhp -o ./doc/help/agros2d.qch
		lrelease ./lang/*.ts ./lang/*.qm
		if qmake ./agros2d.pro ; then make ; fi
		rm ../agros2d_*
		dpkg-buildpackage -sgpg -rfakeroot
		;;
	* )
		echo "Usage: agros2d.sh  [help - generate help files] [lang - generate language files] [comp - compile] [pack - build package] [all - all operations]"
		;;
esac
