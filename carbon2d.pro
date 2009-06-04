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
TARGET = carbon2d
TEMPLATE = app
QMAKE_CXXFLAGS_DEBUG += -w
QMAKE_CXXFLAGS += -w
SOURCES += main.cpp \
    util.cpp \
    gui.cpp \
    scenehermes.cpp \
    hermes2d/hermes_electrostatic.cpp \
    hermes2d/hermes_heat.cpp \
    hermes2d/hermes_magnetostatic.cpp \
    hermes2d/hermes_elasticity.cpp \
    mainwindow.cpp \
    scenemarker.cpp \
    scenebasic.cpp \
    sceneinfo.cpp \
    sceneview.cpp \
    scene.cpp \
    # chartdialog.cpp \
    projectdialog.cpp \
    scenetransformdialog.cpp \
    sceneviewdialog.cpp \
    optionsdialog.cpp \
    scenesolution.cpp \
    localvalueview.cpp \
    volumeintegralview.cpp \
    surfaceintegralview.cpp
HEADERS += mainwindow.h \
    util.h \
    gui.h \
    scenehermes.h \
    hermes2d/hermes_electrostatic.h \
    hermes2d/hermes_heat.h \
    hermes2d/hermes_magnetostatic.h \
    hermes2d/hermes_elasticity.h \
    scenebasic.h \
    sceneinfo.h \
    scenemarker.h \
    sceneview_data.h \
    sceneview.h \
    scene.h \
    # chartdialog.h \
    projectdialog.h \
    scenetransformdialog.h \
    sceneviewdialog.h \
    optionsdialog.h \
    scenesolution.h \
    localvalueview.h \
    volumeintegralview.h \
    surfaceintegralview.h
INCLUDEPATH += include/hermes2d \
    include/dxflib \
    /usr/include \
    /usr/include/suitesparse
LIBS += lib/liblibdxf.a \
    -lhermes2d-real \
    -lumfpack \
    -lamd \
    -lblas \
    -lJudy \
    -lpthread

FORMS += 
OTHER_FILES += TODO.txt
