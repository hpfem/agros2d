#!/bin/sh

docPath="./doc"
docReleasePath="./doc/help"
docBuildPath="./doc/build/html"
reportPath="./doc/report"
langPath="./lang"
temporaryDirectory="./tmp"
debianizedFiles="./data ./debian ./lang ./src ./hermes_common ./src-remote ./agros2d.desktop ./agros2d.iss ./agros2d.pro ./COPYING ./functions.py ./problem-agros2d.xml ./README ./hermes2d"

version="1.9"

case "$1" in
	help )
		cd $docPath
		make html

		cd ../
		if [ -e $docReleasePath ] ; then rm -r $docReleasePath ; fi
		mkdir $docReleasePath
		cp -r $docBuildPath/* $docReleasePath
		;;
	lang )
		case "$2" in
			release )
				lrelease $langPath/*.ts
				;;
			update )
				lupdate src/src.pro -ts $langPath/cs_CZ.ts $langPath/en_US.ts $langPath/pl_PL.ts $langPath/de_DE.ts
				;;
			* )
				echo "Usage: agros2d.sh lang\n  [release - release language files]\n  [update - update language files]"
				;;
		esac
		;;
	comp )
		if qmake ./agros2d.pro ; then make ; fi
		;;
	build )
		case "$2" in
			binary )
				if [ -e ../agros2d_* ] ; then rm ../agros2d_* ; fi
				dpkg-buildpackage -sgpg -rfakeroot
				;;
			source )
				./agros2d.sh help
				./agros2d.sh lang release

				rm -r $temporaryDirectory

				if [ -e $temporaryDirectory ]
				then
					if [ -e $temporaryDirectory/agros2d-$version ]
					then
						rm -r $temporaryDirectory/agros2d-$version
					fi
				else
					mkdir $temporaryDirectory
				fi

				mkdir $temporaryDirectory/agros2d-$version
				for i in $debianizedFiles
				do
					cp -r $i $temporaryDirectory/agros2d-$version
				done

				# documentation
				mkdir $temporaryDirectory/agros2d-$version/doc
				mkdir $temporaryDirectory/agros2d-$version/doc/help
				cp -r $docReleasePath/* $temporaryDirectory/agros2d-$version/doc/help
				cp -r $reportPath $temporaryDirectory/agros2d-$version/doc

				rm -f $temporaryDirectory/agros2d-$version/src/Doxyfile
				rm -f $temporaryDirectory/agros2d-$version/src/Makefile
				rm -f $temporaryDirectory/agros2d-$version/src/build/*
				rm -f $temporaryDirectory/agros2d-$version/hermes2d/build/*
				rm -f $temporaryDirectory/agros2d-$version/hermes2d/lib/*
				rm -f $temporaryDirectory/agros2d-$version/src-remote/build/*
				rm -f $temporaryDirectory/agros2d-$version/src-remote/agros2d-remote
				rm -f $temporaryDirectory/agros2d-$version/data/data/*
				rmdir $temporaryDirectory/agros2d-$version/data/data

				cd $temporaryDirectory/agros2d-$version
				echo "Run 'debuild -S -sa'"
				echo "Run 'dput ppa:pkarban/agros2d *.changes' for upload"
				;;
			* )
				echo "Usage: agros2d.sh build\n  [binary - build binary package]\n  [source - build source package]"
				;;
		esac
		;;
	upload )
		./agros2d.sh build source
		cd $temporaryDirectory/agros2d-$version
		debuild -S -sa
		echo "Run 'dput ppa:pkarban/agros2dunstable *.changes' for upload unstable"
		echo "Run 'dput ppa:pkarban/agros2d *.changes' for upload"
		;;
	* )
		echo "Usage: agros2d.sh\n  [help - build and generate documentation]\n  [lang release - release language files]\n  [lang update - update language files]\n  [comp - compile]\n  [build binary - build binary package]\n  [build source - build source package]\n  [upload - prepare package for upload]"
		;;
esac
