QT += core gui network xml webkit xmlpatterns

TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../../hermes2d/include \
    ../../../hermes_common/include \
    ../../../src \
    ../../../lib/muparser \
    ../../../lib/dxflib \
    ../../../src/hermes2d \
    ../../interface

LIBS += -lhermes2d

linux-g++|linux-g++-64|linux-g++-32 {
    LIBS += -L../../hermes2d/build/lib
}

win32-msvc2010 {
    LIBS += -L../hermes2d/debug/build/lib
    LIBS += -L../hermes2d/release/build/lib
}

HEADERS      += electrostatic_interface.h
SOURCES      += electrostatic_interface.cpp

HEADERS      += electrostatic_steadystate_axi.h
HEADERS      += electrostatic_steadystate_planar.h
SOURCES      += electrostatic_steadystate_axi.cpp
SOURCES      += electrostatic_steadystate_planar.cpp

TARGET        = $$qtLibraryTarget(electrostatic)
DESTDIR       = ../../../resources/plugins
