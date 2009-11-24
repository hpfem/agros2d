#!/bin/sh

sourcePath="./doc/source"
helpPath="./doc/help"
webPath="./doc/web"
latexPath="./doc/latex"
srcPath="./src"
langPath="./lang"

case "$1" in
	help )
		if sphinx-build -b qthelp $sourcePath $helpPath
		then
			qcollectiongenerator $helpPath/Agros2D.qhcp -o $helpPath/Agros2D.qhc
		fi
		;;
	help.build-web )
		sphinx-build -b html $sourcePath $webPath
		;;
	help.build-latex )
		sphinx-build -b latex $sourcePath $latexPath
		;;
	lang )
		lrelease $langPath/*.ts
		;;
	lang.update )
		lupdate $srcPath/*.cpp $srcPath/*.h -ts $langPath/cs_CZ.ts $langPath/en_US.ts
		;;
	lang.update-noobsolete )
		lupdate $srcPath/*.cpp $srcPath/*.h -noobsolete -ts $langPath/cs_CZ.ts $langPath/en_US.ts
		;;
	comp )
		if qmake ./agros2d.pro ; then make ; fi
		;;
	pack )
		rm ../agros2d_*
		dpkg-buildpackage -sgpg -rfakeroot
		;;
	all )
		if sphinx-build -b qthelp $sourcePath $helpPath
		then
			qcollectiongenerator $helpPath/Agros2D.qhcp -o $helpPath/Agros2D.qhc
		fi
		lrelease $langPath/*.ts
		if qmake ./agros2d.pro ; then make ; fi
		;;
	* )
		echo "Usage: agros2d.sh\n  [help - build and generate help]\n  [help.build-web - build online help]\n  [help.build-latex - build latex documentation]\n  [lang - release language files]\n  [lang.update - update language files]\n  [lang.update-noobsolete - update language files without obsolete translations]\n  [comp - compile]\n  [pack - build package]\n  [all - build and generate help, release language files, compile]"
		;;
esac
