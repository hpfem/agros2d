#!/bin/sh

docPath="resources_source/doc"
docPathTarget="./resources/help"
langPath="resources_source/lang"
langPathTarget="./resources/lang"
tempPath="tmp"
compCores=1
version="3.0"
debianizedFiles="debian hermes2d hermes_common lib libs resources resources_source util agros2d-forms agros2d-remote agros2d-generator agros2d-binary agros2d-library agros2d-solver pythonlab-library pythonlab-binary 3rdparty agros2d.desktop CMakeList.txt agros2d.sh AUTHORS COPYING functions_agros2d.py functions_pythonlab.py README"
unwantedFiles="resources_source/test libs/* 3rdparty/build 3rdparty/Makefile 3rdparty/libs 3rdparty/ctemplate/Makefile agros2d-forms/Makefile agros2d-forms/example/build agros2d-forms/Makefile 3rdparty/ctemplate/build 3rdparty/dxflib/Makefile 3rdparty/dxflib/build 3rdparty/paralution/Makefile 3rdparty/paralution/build 3rdparty/poly2tri/Makefile 3rdparty/poly2tri/build 3rdparty/qcustomplot/Makefile 3rdparty/qcustomplot/build 3rdparty/quazip/Makefile 3rdparty/quazip/build 3rdparty/stb_truetype/Makefile 3rdparty/stb_truetype/build util/build util/Makefile hermes2d/build hermes2d/lib hermes2d/Makefile agros2d-solver/build solver/Makefile agros2d-solver/qrc_resources.cpp agros2d-remote/build agros2d-remote/Makefile agros2d-generator/build agros2d-generator/Makefile agros2d-generator/qrc_resources.cpp pythonlab-library/build pythonlab-library/Makefile pythonlab-library/qrc_resources.cpp agros2d-binary/build agros2d-binary/Makefile agros2d-binary/qrc_resources.cpp pythonlab-binary/build pythonlab-binary/Makefile pythonlab-binary/qrc_resources.cpp agros2d-library/build agros2d-library/Makefile agros2d-library/qrc_resources.cpp"

export LD_LIBRARY_PATH="libs"

case "$1" in
    help )
        make html -C $docPath

        if [ -e $docPathTarget ] ; then rm -rv $docPathTarget ; fi
        mkdir -v $docPathTarget
        cp -rv $docPath/build/html/* $docPathTarget
        ;;
    lang )
        case "$2" in
            release )
                lrelease $langPath/*.ts

                if [ -e $langPathTarget ] ; then rm -rv $langPathTarget ; fi
                mkdir -v $langPathTarget
                mv -v $langPath/*.qm $langPathTarget
                ;;
            update )
                lupdate agros2d.pro -ts $langPath/cs_CZ.ts $langPath/en_US.ts $langPath/pl_PL.ts $langPath/de_DE.ts $langPath/ru_RU.ts $langPath/fr_FR.ts
                lupdate plugins/plugins.pro -ts $langPath/plugin_cs_CZ.ts $langPath/plugin_en_US.ts $langPath/plugin_pl_PL.ts $langPath/plugin_ru_RU.ts $langPath/plugin_fr_FR.ts
                ;;
            * )
                echo "Usage: agros2d.sh lang\n\t [release - release language files]\n\t [update - update language files]"
                ;;
        esac
        ;;
    comp )
        if [ $2 -gt $compCores ]; then compCores=$2 ; fi
        if cmake . ; then make -j$compCores ; fi
        ./agros2d_generator
        if cmake plugins/CMakeList.txt ; then make -j$compCores -C ./plugins/ ; fi
        ;;
    run )
        ./agros2d
        ;;        
    equations )
        find ./plugins/ -name "*.py" -exec python {} \;
        ;;       
    cython )
        cython --cplus resources_source/python/pythonlab.pyx
        cython --cplus resources_source/python/agros2d.pyx
        ;;     
    release )
    	git clean -dxf
        ./agros2d.sh help
        ./agros2d.sh lang release
        ./agros2d.sh cython
        
        tempPathTarget=$tempPath/agros2d-$version

        if [ -e $tempPath ]
        then
            if [ -e $tempPathTarget ] ; then rm -rv $tempPathTarget ; fi
        else
            mkdir -v $tempPath
        fi
        
        mkdir -v $tempPathTarget
        cp -rv * $tempPathTarget
        rm -rv $tempPathTarget/resources_source/test
        rmdir -v $tempPathTarget/resources_source/test
        
        cd $tempPathTarget        
        debuild -S -sa >/dev/null
        
        ;;             
    build )
        case "$2" in
            binary )
                if [ -e ../agros2d_* ] ; then rm -v ../agros2d_* ; fi
                dpkg-buildpackage -sgpg -rfakeroot
                ;;
            source )
                tempPathTarget=$tempPath/agros2d-$version

                ./agros2d.sh help
                ./agros2d.sh lang release

                if [ -e $tempPath ]
                then
                    if [ -e $tempPathTarget ] ; then rm -rv $tempPathTarget ; fi
                else
                    mkdir -v $tempPath
                fi

                mkdir -v $tempPathTarget

                for file in $debianizedFiles
                do
                    cp -rv ./$file $tempPathTarget
                done

                for file in $unwantedFiles
                do
                    rm -rv ./$tempPathTarget/$file
                done
                rm `find ./$tempPathTarget/resources/python -name "*.pyc"`

                echo "Run 'debuild -S -sa'"
                echo "Run 'dput ppa:pkarban/agros2d *.changes' for upload stable version"
                echo "Run 'dput ppa:pkarban/agros2dunstable *.changes' for upload unstable version"
                ;;
            * )
                echo "Usage: agros2d.sh build\n\t [binary - build binary package]\n\t [source - build source package]"
                ;;
        esac
        ;;
    * )
        echo "Usage: agros2d.sh\n\t [equations - build equations]\n\t [help - build and generate documentation]\n\t [cython - run cython]\n\t [lang release - release language files]\n\t [lang update - update language files]\n\t [comp - compile]\n\t [run - run]\n\t [build binary - build binary package]\n\t [build source - build source package]"
        ;;
esac
