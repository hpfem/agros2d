# agros2d - hp-FEM multiphysics application based on Hermes2D library
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3rdparty
LIBS += -lagros2d

RC_FILE = ../src/src.rc
RESOURCES = ../src/src.qrc

SOURCES += main.cpp \
    agros_solver.cpp
HEADERS += \
    agros_solver.h

linux-g++|linux-g++-64|linux-g++-32 {
    TARGET = ../agros2d_solver
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

win32-msvc2010 {
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt

    LIBS += -L..
    TARGET = ../../agros2d
}

include(../agros2d.pri)
include(../agros2d_version.pri)
