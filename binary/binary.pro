# agros2d - hp-FEM multiphysics application based on Hermes2D library
TARGET = ../agros2d
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3dparty
LIBS += -lagros2d

RC_FILE = ../src/src.rc
RESOURCES = ../src/src.qrc

SOURCES += main.cpp

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

include(../agros2d.pri)
