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

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++:CONFIG(release) system(cython python/agros2d.pyx)
linux-g++:CONFIG(release) system(lrelease ../lang/*.ts)
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
SOURCES += ../lib/dxflib/dl_writer_ascii.cpp \
    ../lib/dxflib/dl_dxf.cpp \
    ../lib/muparser/muParserBase.cpp \
    ../lib/muparser/muParserBytecode.cpp \
    ../lib/muparser/muParserCallback.cpp \
    ../lib/muparser/muParser.cpp \
    ../lib/muparser/muParserError.cpp \
    ../lib/muparser/muParserInt.cpp \
    ../lib/muparser/muParserTest.cpp \
    ../lib/muparser/muParserTokenReader.cpp \
    ../lib/rapidxml/rapidxml.cpp \
    ../lib/rapidxml/rapidxml_utils.cpp \
    util.cpp \
    value.cpp \
    scene.cpp \
    gui.cpp \
    hermes2d/localpoint.cpp \
    hermes2d/volumeintegral.cpp \
    hermes2d/surfaceintegral.cpp \
    hermes2d/boundary.cpp \
    hermes2d/material.cpp \
    hermes2d/module.cpp \
    #hermes2d/hermes_general.cpp \
    hermes2d/hermes_electrostatic.cpp \
    #hermes2d/hermes_heat.cpp \
    #hermes2d/hermes_magnetic.cpp \
    #hermes2d/hermes_current.cpp \
    #hermes2d/hermes_elasticity.cpp \
    #hermes2d/hermes_rf.cpp \
    #hermes2d/hermes_acoustic.cpp \
    #hermes2d/hermes_flow.cpp \
    main.cpp \
    scripteditordialog.cpp \
    scripteditorcommandpython.cpp \
    scripteditorhighlighter.cpp \
    mainwindow.cpp \
    scenemarker.cpp \
    scenebasic.cpp \
    scenefunction.cpp \
    sceneinfoview.cpp \
    sceneview.cpp \
    progressdialog.cpp \
    chartdialog.cpp \
    problemdialog.cpp \
    scenetransformdialog.cpp \
    config.cpp \
    configdialog.cpp \
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
    hermes2d/weakform_parser.cpp
HEADERS += util.h \
    value.h \
    scene.h \
    gui.h \
    hermes2d/localpoint.h \
    hermes2d/volumeintegral.h \
    hermes2d/surfaceintegral.h \
    hermes2d/boundary.h \
    hermes2d/material.h \
    hermes2d/module.h \
    #hermes2d/hermes_general.h \
    hermes2d/hermes_electrostatic.h \
#    hermes2d/hermes_heat.h \
#    hermes2d/hermes_magnetic.h \
#    hermes2d/hermes_current.h \
#    hermes2d/hermes_elasticity.h \
#    hermes2d/hermes_rf.h \
#    hermes2d/hermes_acoustic.h \
    #hermes2d/hermes_flow.h \
    mainwindow.h \
    scripteditordialog.h \
    scripteditorcommandpython.h \
    scripteditorhighlighter.h \
    scenebasic.h \
    sceneinfoview.h \
    scenemarker.h \
    scenefunction.h \
    sceneview_data.h \
    sceneview.h \
    progressdialog.h \
    chartdialog.h \
    problemdialog.h \
    scenetransformdialog.h \
    config.h \
    configdialog.h \
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
    hermes2d/weakform_parser.h

INCLUDEPATH += . \
    ../lib/muparser \
    ../lib/dxflib \
    ../lib/rapidxml \
    ../hermes_common \
    ../hermes2d

OTHER_FILES += python/agros2d.pyx \
    functions.py \
    version.xml \
    ../modules/rf.xml \
    ../modules/magnetic.xml \
    ../modules/heat.xml \
    ../modules/electrostatic.xml \
    ../modules/elasticity.xml \
    ../modules/current.xml \
    ../modules/acoustic.xml

linux-g++ {
    # DEFINES += WITH_MUMPS
    # DEFINES += WITH_SUPERLU
    # DEFINES += WITH_UNITY

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/python2.6
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")
    INCLUDEPATH += ../hermes2d/include
    INCLUDEPATH += ../hermes_common/include
    LIBS += -L../hermes2d/lib
    LIBS += -lhermes2d
    LIBS += -lumfpack
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
    INCLUDEPATH += ../hermes2d/src

    LIBS += -L../hermes2d/lib
    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config
    LIBS += -L../../qwt-5.2.1/lib
    LIBS += -lhermes2d
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
    INCLUDEPATH += ../hermes2d/src
    INCLUDEPATH += ../../qwt-5.2.1/src
    LIBS += -Lc:/Python27/libs
    LIBS += -L../hermes2d/lib
    LIBS += -L../../qwt-5.2.1/lib
    LIBS += -lhermes2d
    LIBS += -lqwt
    LIBS += -lpython27
    LIBS += -llibumfpack
    LIBS += -llibamd
    LIBS += -llibpthreadVCE2
}
