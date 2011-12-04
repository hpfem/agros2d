# agros2d - hp-FEM multiphysics application based on Hermes2D library
QT += opengl \
    xml \
    network \
    webkit \
    svg

DEFINES += VERSION_MAJOR=1
DEFINES += VERSION_MINOR=9
DEFINES += VERSION_SUB=2
DEFINES += VERSION_GIT=823
DEFINES += VERSION_YEAR=2011
DEFINES += VERSION_MONTH=5
DEFINES += VERSION_DAY=23

DEFINES += NO_THREADS
DEFINES += WEAKFORM_FACTORY

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++|linux-g++-64|linux-g++-32 :CONFIG(release) {
    system(cython ../resources_source/python/agros2d.pyx)
    system(lrelease ../resources_source/lang/*.ts)
    system(mkdir ../resources/lang)
    system(mv ../resources_source/lang/*.qm ../resources/lang)
}
TRANSLATIONS = lang/cs_CZ.ts \
    lang/pl_PL.ts \
    lang/de_DE.ts
CODECFORTR = UTF-8
RC_FILE = src.rc
RESOURCES = src.qrc
TARGET = agros2d
DESTDIR = ../
TEMPLATE = app
CONFIG += warn_off
# QMAKE_CXXFLAGS_DEBUG += -Wno-builtin-macro-redefined -Wunused-variable -Wreturn-type
# QMAKE_CXXFLAGS += -fno-strict-aliasing -Wno-builtin-macro-redefined
# QMAKE_CXXFLAGS_DEBUG += -w
# QMAKE_CXXFLAGS += -w
# QMAKE_CXXFLAGS_DEBUG += -Wuninitialized
# QMAKE_CXXFLAGS += -Wuninitialized
OBJECTS_DIR = build
MOC_DIR = build
SUBDIRS += src
SOURCES += util.cpp \
    value.cpp \
    scene.cpp \
    gui.cpp \
    hermes2d/localpoint.cpp \
    hermes2d/volumeintegral.cpp \
    hermes2d/surfaceintegral.cpp \
    hermes2d/marker.cpp \
    hermes2d/module.cpp \
    hermes2d/solver.cpp \
###    hermes2d/hermes_flow.cpp \
    main.cpp \
    scripteditordialog.cpp \
    scripteditorcommandpython.cpp \
    scripteditorhighlighter.cpp \
    mainwindow.cpp \
    scenemarker.cpp \
    scenemarkerdialog.cpp \
    scenebasic.cpp \
###    scenefunction.cpp \
    sceneinfoview.cpp \
    sceneview.cpp \
    progressdialog.cpp \
    chartdialog.cpp \
    problemdialog.cpp \
    scenetransformdialog.cpp \
    scenesolution.cpp \
    reportdialog.cpp \
    videodialog.cpp \
    terminalview.cpp \
    scenemarkerselectdialog.cpp \
    tooltipview.cpp \
    scenebasicselectdialog.cpp \
    logdialog.cpp \
    postprocessorview.cpp \
    style/stylehelper.cpp \
    style/styleanimator.cpp \
    style/manhattanstyle.cpp \
    indicators/indicators.cpp \
    indicators/indicator_unity.cpp \
    collaboration.cpp \
    resultsview.cpp \
    hermes2d/weakform_parser.cpp\
    alter_newton_solver.cpp \
    datatable.cpp \
    datatabledialog.cpp \
    materialbrowserdialog.cpp \
    conf.cpp \
    confdialog.cpp \
    hermes2d/module_agros.cpp \
    sceneedge.cpp \
    scenelabel.cpp \
    scenenode.cpp

HEADERS += util.h \
    value.h \
    scene.h \
    gui.h \
    hermes2d/localpoint.h \
    hermes2d/volumeintegral.h \
    hermes2d/surfaceintegral.h \
    hermes2d/marker.h \
    hermes2d/module.h \
    #hermes2d/hermes_flow.h \
    mainwindow.h \
    scripteditordialog.h \
    scripteditorcommandpython.h \
    scripteditorhighlighter.h \
    scenebasic.h \
    sceneinfoview.h \
    scenemarker.h \
    scenemarkerdialog.h \
#    scenefunction.h \
    sceneview_data.h \
    sceneview.h \
    progressdialog.h \
    chartdialog.h \
    problemdialog.h \
    scenetransformdialog.h \
    scenesolution.h \
    reportdialog.h \
    videodialog.h \
    terminalview.h \
    scenemarkerselectdialog.h \
    tooltipview.h \
    scenebasicselectdialog.h \
    logdialog.h \
    postprocessorview.h \
    style/stylehelper.h \
    style/styleanimator.h \
    style/manhattanstyle.h \
    indicators/indicators.h \
    indicators/indicator_unity.h \
    collaboration.h \
    resultsview.h \
    hermes2d/weakform_parser.h \
    alter_newton_solver.h \
    datatable.h \
    datatabledialog.h \
    materialbrowserdialog.h \
    conf.h \
    confdialog.h \
    hermes2d/solver.h \
    hermes2d/module_agros.h \
    sceneedge.h \
    scenelabel.h \
    scenenode.h

INCLUDEPATH += . \
    ../lib \
    ../lib/ctemplate \
    ../lib/muparser \
    ../lib/dxflib \
    ../lib/rapidxml \
    ../hermes_common \
    ../hermes2d

OTHER_FILES += python/agros2d.pyx \
    functions.py \
    version.xml \
    ../resources/modules/rf.xml \
    ../resources/modules/magnetic.xml \
    ../resources/modules/heat.xml \
    ../resources/modules/electrostatic.xml \
    ../resources/modules/elasticity.xml \
    ../resources/modules/current.xml \
    ../resources/modules/acoustic.xml \
    ../resources/custom.xml \
    ../resources/a2d_convert_table.xml \
    ../resources/materials/Copper.xml \
    ../resources/materials/Aluminum.xml \
    ../resources/materials/Iron.xml

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include
LIBS += -L../hermes2d/lib
LIBS += -lhermes2d
LIBS += -L../lib/lib
LIBS += -llib
LIBS += -L../weakform/lib
#LIBS += -lweakform #//TODO

linux-g++|linux-g++-64|linux-g++-32 {
    # DEFINES += WITH_MUMPS
    # DEFINES += WITH_SUPERLU
    # DEFINES += WITH_UNITY

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")
    LIBS += -lumfpack
    LIBS += -lxerces-c
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lpthread
    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")
    # qwt
    INCLUDEPATH += /usr/include/qwt-qt4
    LIBS += -lqwt-qt4

    # mumps
    contains(DEFINES, WITH_MUMPS) {
        LIBS += -ldmumps_seq
    }
    # superlu
    contains(DEFINES, WITH_SUPERLU) {
        INCLUDEPATH += /usr/include/superlu
        LIBS += -lsuperlu
    }

    # unity
    contains(DEFINES, WITH_UNITY) {
        INCLUDEPATH += /usr/include/unity/unity
        INCLUDEPATH += /usr/include/glib-2.0
        INCLUDEPATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include
        INCLUDEPATH += /usr/include/dee-1.0
        INCLUDEPATH += /usr/include/libdbusmenu-0.4
        LIBS += -lunity
    }
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target

    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.7
    INCLUDEPATH += ../../qwt-5.2.1/src

    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config
    LIBS += -L../../qwt-5.2.1/lib
    LIBS += -lpthread
    LIBS += -lpython2.7
    LIBS += -lqwt
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}

win32-msvc2008 {
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../../qwt-5.2.1/src
    LIBS += -Lc:/Python27/libs
    LIBS += -L../../qwt-5.2.1/lib
    LIBS += -lqwt
    LIBS += -lpython27
    LIBS += -llibumfpack
    LIBS += -llibamd
    LIBS += -llibpthreadVCE2
}






