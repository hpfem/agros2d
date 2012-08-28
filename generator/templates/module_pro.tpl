QT += core gui network xml webkit xmlpatterns

OBJECTS_DIR = ../build
MOC_DIR = ../build

TEMPLATE = lib
CONFIG += plugin 

INCLUDEPATH += ../../hermes2d/include \
    ../../hermes_common/include \
    ../../src \
    ../../src/hermes2d \
    ../../3dparty/muparser \
    ../../3dparty/dxflib

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3dparty
LIBS += -lagros2d

linux-g++|linux-g++-64|linux-g++-32 {
    LIBS += -L../../libs
}

win32-msvc2010 {
    LIBS += -L../../hermes2d/debug/build/lib
    LIBS += -L../../hermes2d/release/build/lib
    LIBS += -L../../lib/debug/build/lib
    LIBS += -L../../lib/release/build/lib
}

# interface
HEADERS      += {{ID}}_interface.h
SOURCES      += {{ID}}_interface.cpp

# headers and sources
HEADERS      += {{ID}}_weakform.h
SOURCES      += {{ID}}_weakform.cpp
HEADERS      += {{ID}}_filter.h
SOURCES      += {{ID}}_filter.cpp
HEADERS      += {{ID}}_localvalue.h
SOURCES      += {{ID}}_localvalue.cpp

TARGET        = $$qtLibraryTarget(agros2d_plugin_{{ID}})
DESTDIR       = ../../libs
