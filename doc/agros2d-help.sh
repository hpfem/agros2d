#!/bin/sh

case "$1" in
	help )
		if sphinx-build -b qthelp ./source ./help
		then
			qcollectiongenerator ./help/Agros2D.qhcp -o ./help/Agros2D.qhc
		fi
		;;
	latex )
		if sphinx-build -b latex ./source ./latex
		then
			make ./latex/Makefile
		fi
		;;
	web )
		echo "To corectly generate online help is need to edit the configuration file."
		sphinx-build -b html ./source ./web
		;;
	all )
		if sphinx-build -b qthelp ./source ./help
		then
			qcollectiongenerator ./help/Agros2D.qhcp -o ./help/Agros2D.qhc
		fi
		if sphinx-build -b latex ./source ./latex
		then
			make ./latex/Makefile
		fi
		#sphinx-build -b html ./source ./web
	;;
	* )
		echo "Usage: agros2d-help.sh  [help - generate help source files] [web - generate web files (for online help)] [latex - generate latex files] [all - all operations]"
		;;
esac
