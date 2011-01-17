# agros2d - hp-FEM multiphysics application based on Hermes2D library
QT += opengl \
    xml \
    webkit \
    network

CONFIG(debug): DEFINES += BETA
# DEFINES += VERSION_BETA
DEFINES += VERSION_MAJOR=1
DEFINES += VERSION_MINOR=7
DEFINES += VERSION_SUB=0
DEFINES += VERSION_GIT=580
DEFINES += VERSION_YEAR=2011
DEFINES += VERSION_MONTH=1
DEFINES += VERSION_DAY=22

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++:CONFIG(release) system(cython python/agros2d.pyx)
TRANSLATIONS = lang/cs_CZ.ts \
    lang/pl_PL.ts
CONFIG += help
CODECFORTR = UTF-8
RC_FILE = src.rc
RESOURCES = src.qrc
TARGET = ../agros2d
TEMPLATE = app
QMAKE_CXXFLAGS_DEBUG += -w
QMAKE_CXXFLAGS += -w
OBJECTS_DIR = build
MOC_DIR = build
SUBDIRS += src
SOURCES += util.cpp \
    scene.cpp \
    gui.cpp \
    hermes2d/hermes_field.cpp \
    hermes2d/hermes_general.cpp \
    hermes2d/hermes_electrostatic.cpp \
    hermes2d/hermes_heat.cpp \
    hermes2d/hermes_magnetic.cpp \
    hermes2d/hermes_current.cpp \
    hermes2d/hermes_elasticity.cpp \
    hermes2d/hermes_flow.cpp \
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
    sceneviewdialog.cpp \
    config.cpp \
    configdialog.cpp \
    helpdialog.cpp \
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
    hermes2d/hermes_rf.cpp
HEADERS += util.h \
    scene.h \
    gui.h \
    hermes2d/hermes_field.h \
    hermes2d/hermes_general.h \
    hermes2d/hermes_electrostatic.h \
    hermes2d/hermes_heat.h \
    hermes2d/hermes_magnetic.h \
    hermes2d/hermes_current.h \
    hermes2d/hermes_elasticity.h \
    hermes2d/hermes_flow.h \
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
    sceneviewdialog.h \
    config.h \
    configdialog.h \
    helpdialog.h \
    scenesolution.h \
    reportdialog.h \
    videodialog.h \
    terminalview.h \
    scenemarkerselectdialog.h \
    tooltipview.h \
    scenebasicselectdialog.h \
    logdialog.h \
    hermes2d/hermes_rf.h
INCLUDEPATH += . \
    dxflib \
    ../hermes_common
OTHER_FILES += python/agros2d.pyx \
    functions.py \
    version.xml
linux-g++ {
    DEFINES += WITH_MUMPS
    DEFINES += WITH_SUPERLU

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/superlu
    INCLUDEPATH += /usr/include/qwt-qt4
    INCLUDEPATH += /usr/include/python2.6
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")
    INCLUDEPATH += ../hermes2d/src
    LIBS += -L../hermes2d/lib
    LIBS += -lhermes2d
    LIBS += -lumfpack
    LIBS += -ldmumps_seq
    LIBS += -lsuperlu
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lpthread
    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")
    LIBS += -lqwt-qt4
}
win32-g++ {
    INCLUDEPATH += c:/qt/mingw/include
    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../hermes2d/src
    INCLUDEPATH += ../../qwt-5.2.1/src
    LIBS += -Lc:/Python27
    LIBS += -L../hermes2d/lib
    LIBS += -L../../qwt-5.2.1/lib
    LIBS += -lhermes2d
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lpthread
    LIBS += -lqwt
    LIBS += -lpython27
}
macx-g++ {
    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.6
    INCLUDEPATH += ../qwt-5.2.0/src
    INCLUDEPATH += hermes2d/src
    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.6/lib/python2.6/config
    LIBS += -L../hermes2d/src
    LIBS += -L../../qwt-5.2.0/lib
    LIBS += -lpthread
    LIBS += -lhermes2d
    LIBS += -lpython2.6
    LIBS += -lqwt
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}
