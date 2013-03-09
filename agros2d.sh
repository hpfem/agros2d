#!/bin/sh

docPath="./resources_source/doc"
docPathTarget="./resources/help"
langPath="./resources_source/lang"
langPathTarget="./resources/lang"
tempPath="./tmp"
version="3.0"
debianizedFiles="data debian hermes2d hermes_common lib libs resources resources_source generator binary solver pythonlab src src-remote weakform 3rdparty agros2d.desktop agros2d.pro agros2d.pri agros2d_version.pri agros2d.sh AUTHORS COPYING functions_agros2d.py functions_pythonlab.py README"
unwantedFiles="data/*.py data/save data/test/*.mph libs/* 3rdparty/build 3rdparty/Makefile hermes2d/build hermes2d/lib hermes2d/Makefile solver/build solver/Makefile solver/qrc_src.cpp generator/build generator/Makefile generator/qrc_src.cpp pythonlab/build pythonlab/Makefile pythonlab/qrc_src.cpp binary/build binary/Makefile binary/qrc_src.cpp pythonlab/build pythonlab/Makefile src/build src/Makefile src/qrc_src.cpp"

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
                lupdate src/src.pro -ts $langPath/cs_CZ.ts $langPath/en_US.ts $langPath/pl_PL.ts $langPath/de_DE.ts $langPath/ru_RU.ts $langPath/fr_FR.ts
                lupdate plugins/plugins.pro -ts $langPath/plugin_cs_CZ.ts $langPath/plugin_en_US.ts $langPath/plugin_pl_PL.ts $langPath/plugin_ru_RU.ts $langPath/plugin_fr_FR.ts
                ;;
            * )
                echo "Usage: agros2d.sh lang\n\t [release - release language files]\n\t [update - update language files]"
                ;;
        esac
        ;;
    comp )
        if qmake -r ./agros2d.pro ; then make ; fi
        ./agros2d_generator
        if qmake ./plugins/plugins.pro ; then make -C ./plugins/ ; fi
        ;;
    run )
        ./agros2d
        ;;        
    equations )
        find ./plugins/ -name "*.py" -exec python {} \;
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
        echo "Usage: agros2d.sh\n\t [equations - build equations]\n\t [help - build and generate documentation]\n\t [lang release - release language files]\n\t [lang update - update language files]\n\t [comp - compile]\n\t [run - run]\n\t [build binary - build binary package]\n\t [build source - build source package]"
        ;;
esac
