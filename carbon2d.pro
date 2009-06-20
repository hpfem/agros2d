# -------------------------------------------------
# Project created by QtCreator 2009-04-05T11:57:16
# -------------------------------------------------
QT += opengl \
    xml \
    script
TRANSLATIONS = lang/cs_CZ.ts \
    lang/en_US.ts
CODECFORTR = UTF-8
RC_FILE = carbon2d.rc
RESOURCES = carbon2d.qrc
TARGET = carbon2d
TEMPLATE = app
QMAKE_CXXFLAGS_DEBUG += -w
QMAKE_CXXFLAGS += -w
OBJECTS_DIR = build
MOC_DIR = build
SUBDIRS += src
SOURCES += src/main.cpp \
    src/util.cpp \
    src/gui.cpp \
    src/scripteditordialog.cpp \
    src/scripteditorhighlighter.cpp \
    src/solverdialog.cpp \
    src/hermes2d/hermes_electrostatic.cpp \
    src/hermes2d/hermes_heat.cpp \
    src/hermes2d/hermes_magnetostatic.cpp \
    src/hermes2d/hermes_current.cpp \
    src/hermes2d/hermes_elasticity.cpp \
    src/mainwindow.cpp \
    src/scenemarker.cpp \
    src/scenebasic.cpp \
    src/sceneinfo.cpp \
    src/sceneview.cpp \
    src/scene.cpp \
    src/chartdialog.cpp \
    src/projectdialog.cpp \
    src/scenetransformdialog.cpp \
    src/sceneviewdialog.cpp \
    src/optionsdialog.cpp \
    src/scenesolution.cpp \
    src/localvalueview.cpp \
    src/volumeintegralview.cpp \
    src/surfaceintegralview.cpp \
    src/dxflib/dl_writer_ascii.cpp \
    src/dxflib/dl_dxf.cpp
HEADERS += src/mainwindow.h \
    src/util.h \
    src/gui.h \
    src/scripteditordialog.h \
    src/scripteditorhighlighter.h \
    src/solverdialog.h \
    src/hermes2d/hermes_electrostatic.h \
    src/hermes2d/hermes_heat.h \
    src/hermes2d/hermes_magnetostatic.h \
    src/hermes2d/hermes_current.h \
    src/hermes2d/hermes_elasticity.h \
    src/scenebasic.h \
    src/sceneinfo.h \
    src/scenemarker.h \
    src/sceneview_data.h \
    src/sceneview.h \
    src/scene.h \
    src/chartdialog.h \
    src/projectdialog.h \
    src/scenetransformdialog.h \
    src/sceneviewdialog.h \
    src/optionsdialog.h \
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
FORMS +=
OTHER_FILES += TODO.txt
