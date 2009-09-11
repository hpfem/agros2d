# agros2d - hp-FEM multiphysics application based on Hermes2D library
QT += opengl \
    xml \
    script \
    webkit \
    network
SUBDIRS = src-remote
# DEFINES += BETA
DEFINES += VERSION_MAJOR=0
DEFINES += VERSION_MINOR=9
DEFINES += VERSION_SUB=6
unix:DEFINES += VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
unix::DEFINES += VERSION_YEAR=$$system(date +%Y)
unix::DEFINES += VERSION_MONTH=$$system(date +%-m)
unix::DEFINES += VERSION_DAY=$$system(date +%d)
win32:DEFINES += VERSION_GIT=138
win32::DEFINES += VERSION_YEAR=2009
win32::DEFINES += VERSION_MONTH=9
win32::DEFINES += VERSION_DAY=11
CONFIG += help
TRANSLATIONS = lang/cs_CZ.ts \
    lang/en_US.ts
CODECFORTR = UTF-8
RC_FILE = agros2d.rc
RESOURCES = agros2d.qrc
TARGET = agros2d
TEMPLATE = app
QMAKE_CXXFLAGS_DEBUG += -w
QMAKE_CXXFLAGS += -w
OBJECTS_DIR = build
MOC_DIR = build
unix { 
    # use qmake PREFIX=... to customize your installation
    isEmpty(PREFIX):PREFIX = /usr/local
    
    # install examples
    examples.path = $${PREFIX}/share/agros2d/data
    examples.files = data/*.a2d
    
    # install script
    script.path = $${PREFIX}/share/agros2d/data/script
    script.files = data/script/*.qs
    
    # install help
    help.path = $${PREFIX}/share/agros2d/doc/help
    help.files = doc/help/agros2d.qch \
        doc/help/agros2d.qhc
    
    # install translations
    lang.path = $${PREFIX}/share/agros2d/lang
    lang.files = lang/*.qm
    
    # install script
    script.path = $${PREFIX}/share/agros2d
    script.files = *.js
    
    # install pixmap
    pixmap.path = $${PREFIX}/share/pixmaps
    pixmap.files = images/agros2d.xpm
    
    # install binary
    target.path = $${PREFIX}/bin

    target-remote.path = $${PREFIX}/bin
    target-remote.files = src-remote/agros2d-remote
    
    # "make install" configuration options
    INSTALLS *= target \
	target-remote \
        examples \
        help \
        lang \
        script \
        pixmap
}
SUBDIRS += src
SOURCES += src/util.cpp \
    src/scene.cpp \
    src/main.cpp \
    src/gui.cpp \
    src/scripteditordialog.cpp \
    src/scripteditorcommandsecma.cpp \
    src/scripteditorhighlighter.cpp \
    src/solverdialog.cpp \
    src/hermes2d/hermes_electrostatic.cpp \
    src/hermes2d/hermes_heat.cpp \
    src/hermes2d/hermes_magnetostatic.cpp \
    src/hermes2d/hermes_harmonicmagnetic.cpp \
    src/hermes2d/hermes_current.cpp \
    src/hermes2d/hermes_elasticity.cpp \
    src/mainwindow.cpp \
    src/scenemarker.cpp \
    src/scenebasic.cpp \
    src/scenefunction.cpp \
    src/sceneinfo.cpp \
    src/sceneview.cpp \
    src/chartdialog.cpp \
    src/problemdialog.cpp \
    src/scenetransformdialog.cpp \
    src/sceneviewdialog.cpp \
    src/optionsdialog.cpp \
    src/helpdialog.cpp \
    src/scenesolution.cpp \
    src/localvalueview.cpp \
    src/volumeintegralview.cpp \
    src/surfaceintegralview.cpp \
    src/dxflib/dl_writer_ascii.cpp \
    src/dxflib/dl_dxf.cpp
HEADERS += src/util.h \
    src/scene.h \
    src/mainwindow.h \
    src/gui.h \
    src/scripteditordialog.h \
    src/scripteditorhighlighter.h \
    src/solverdialog.h \
    src/hermes2d/hermes_electrostatic.h \
    src/hermes2d/hermes_heat.h \
    src/hermes2d/hermes_magnetostatic.h \
    src/hermes2d/hermes_harmonicmagnetic.h \
    src/hermes2d/hermes_current.h \
    src/hermes2d/hermes_elasticity.h \
    src/scenebasic.h \
    src/sceneinfo.h \
    src/scenemarker.h \
    src/scenefunction.h \
    src/sceneview_data.h \
    src/sceneview.h \
    src/chartdialog.h \
    src/problemdialog.h \
    src/scenetransformdialog.h \
    src/sceneviewdialog.h \
    src/optionsdialog.h \
    src/helpdialog.h \
    src/scenesolution.h \
    src/localvalueview.h \
    src/volumeintegralview.h \
    src/surfaceintegralview.h
INCLUDEPATH += src \
    src/dxflib
unix:INCLUDEPATH += /usr/include
unix:INCLUDEPATH += /usr/include/suitesparse
unix:INCLUDEPATH += /usr/include/qwt-qt4
unix:INCLUDEPATH += /usr/include/hermes2d
win32:INCLUDEPATH += c:/qt/mingw/include
win32:INCLUDEPATH += c:/qt/mingw/include/hermes2d
LIBS += -lhermes2d-real \
    -lumfpack \
    -lamd \
    -lblas \
    -lJudy \
    -lpthread
unix:LIBS += -lqwt-qt4
win32:LIBS += -lqwt
OTHER_FILES += 
