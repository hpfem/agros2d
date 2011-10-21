# agros2d - hp-FEM multiphysics application based on Hermes2D library
QT += opengl \
    xml \
    network \
    webkit \
    svg

DEFINES += VERSION_MAJOR=1
DEFINES += VERSION_MINOR=9
DEFINES += VERSION_SUB=5
DEFINES += VERSION_GIT=899
DEFINES += VERSION_YEAR=2011
DEFINES += VERSION_MONTH=9
DEFINES += VERSION_DAY=26

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
QMAKE_CXXFLAGS += -fpermissive
OBJECTS_DIR = build
MOC_DIR = build
SUBDIRS += src
SOURCES += util.cpp \
    value.cpp \
    scene.cpp \
    gui.cpp \
    hermes2d/hermes_field.cpp \
    hermes2d/hermes_general.cpp \
    hermes2d/hermes_electrostatic.cpp \
    hermes2d/hermes_heat.cpp \
    hermes2d/hermes_magnetic.cpp \
    hermes2d/hermes_magnetic_integrals.cpp \
    hermes2d/hermes_current.cpp \
    hermes2d/hermes_elasticity.cpp \
    #hermes2d/hermes_flow.cpp \
    hermes2d/hermes_rf.cpp \
    hermes2d/hermes_acoustic.cpp \
    localvalueview.cpp \
    surfaceintegralview.cpp \
    volumeintegralview.cpp \
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
    dxflib/dl_writer_ascii.cpp \
    dxflib/dl_dxf.cpp \
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
    collaboration.cpp
HEADERS += util.h \
    value.h \
    scene.h \
    gui.h \
    hermes2d/hermes_field.h \
    hermes2d/hermes_general.h \
    hermes2d/hermes_electrostatic.h \
    hermes2d/hermes_heat.h \
    hermes2d/hermes_magnetic.h \
    hermes2d/hermes_current.h \
    hermes2d/hermes_elasticity.h \
    #hermes2d/hermes_flow.h \
    hermes2d/hermes_rf.h \
    hermes2d/hermes_acoustic.h \
    localvalueview.h \
    surfaceintegralview.h \
    volumeintegralview.h \
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
    collaboration.h
INCLUDEPATH += . \
    dxflib \
    ../hermes_common
OTHER_FILES += python/agros2d.pyx \
    functions.py \
    version.xml
linux-g++|linux-g++-64|linux-g++-32 {
    # DEFINES += WITH_MUMPS
    # DEFINES += WITH_SUPERLU
    # DEFINES += WITH_UNITY

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")
    INCLUDEPATH += ../hermes2d/src
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
    #ICON = src/images/agros2d.icns

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
