#!/bin/sh

case "$1" in
	help )
		sphinx-build -b qthelp ./source ./help
		qcollectiongenerator ./help/Agros2D.qhcp -o ./help/Agros2D.qhc
		;;
	latex )
		sphinx-build -b latex ./source ./latex
		make ./latex/Makefile
		;;
	web )
		sphinx-build -D html_theme_options.nosidebar=False -b html ./source ./web
		;;
  all )
		# sphinx-build -b qthelp ./source ./help
		# qhelpgenerator ./doc/help/agros2d.qhp -o ./doc/help/agros2d.qch
		# sphinx-build -b latex ./source ./latex
		sphinx-build -b html ./source ./web
	;;
	* )
		echo "Usage: agros2d-help.sh  [help - generate help source files] [web - generate web files (for online help)] [latex - generate latex files] [all - all operations]"
		;;
esac
