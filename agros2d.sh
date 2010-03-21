#!/bin/sh

helpPath="./doc/help"
helpSourcePath="./doc/source"
webPath="./doc/web"
latexPath="./doc/latex"
langPath="./lang"
temporaryDirectory="./tmp"
debianizedFiles="./data ./debian ./doc ./images ./lang ./src ./src-remote ./agros2d.desktop ./agros2d.iss ./agros2d.pro ./agros2d.qrc ./agros2d.qrc ./agros2d.rc ./COPYING ./functions.py ./problem-agros2d.xml ./README"

version="1.0"

case "$1" in
	help )
		if sphinx-build -b qthelp $helpSourcePath $helpPath
		then
			qcollectiongenerator $helpPath/Agros2D.qhcp -o $helpPath/Agros2D.qhc
		fi
		;;
	help.build-web )
		sphinx-build -b html $helpSourcePath $webPath
		;;
	help.build-latex )
		sphinx-build -b latex $helpSourcePath $latexPath
		;;
	lang )
		lrelease $langPath/*.ts
		;;
	lang.update )
		lupdate agros2d.pro -ts $langPath/cs_CZ.ts $langPath/en_US.ts
		;;
	lang.update-noobsolete )
		lupdate agros2d.pro -noobsolete -ts $langPath/cs_CZ.ts $langPath/en_US.ts
		;;
	comp )
		if qmake ./agros2d.pro ; then make ; fi
		;;
	pack.build-binary )
		if [ -e ../agros2d_* ] ; then rm ../agros2d_* ; fi
		dpkg-buildpackage -sgpg -rfakeroot
		;;
	pack.build-source )
		if [ -e $temporaryDirectory ]
		then
			if [ -e $temporaryDirectory/agros2d_$version ]
			then
				rm -r $temporaryDirectory/agros2d_$version
			fi
			if [ -e $temporaryDirectory/hermes2d ]
			then
				rm -r $temporaryDirectory/hermes2d
			fi
		else
			mkdir $temporaryDirectory
		fi

		mkdir $temporaryDirectory/agros2d_$version
		for i in $debianizedFiles
		do
			cp -r $i $temporaryDirectory/agros2d_$version
		done

		mkdir $temporaryDirectory/hermes2d
		#git clone http://hpfem.org/git/agros2d.git $temporaryDirectory/hermes2d

		./agros2d.sh help
		./agros2d.sh lang
		;;
	* )
		echo "Usage: agros2d.sh\n  [help - build and generate help]\n  [help.build-web - build online help]\n  [help.build-latex - build latex documentation]\n  [lang - release language files]\n  [lang.update - update language files]\n  [lang.update-noobsolete - update language files without obsolete translations]\n  [comp - compile]\n  [pack.build-binary - build binary package]\n  [pack.build-source - build source package]"
		;;
esac
