QT += core opengl svg gui network xml webkit xmlpatterns

TEMPLATE = lib
CONFIG += plugin

OBJECTS_DIR = ../build
MOC_DIR = ../build

INCLUDEPATH += ../../hermes2d/include \
    ../../hermes_common/include \
    ../../src \
    ../../src/hermes2d \
    ../../3rdparty/dxflib

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3rdparty
LIBS += -lagros2d

linux-g++|linux-g++-64|linux-g++-32 {
    CONFIG += warn_off

    LIBS += -L../../libs

    DESTDIR = ../../libs
}

win32-msvc2010 {
    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmt
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -L../../libs
    LIBS += -L../..

    LIBS += -Lc:/hpfem/hermes/dependencies/lib
    LIBS += -Ld:/hpfem/hermes/dependencies/lib
    LIBS += -Lc:/Python27/libs
    LIBS += -L../../../qwt-6.0.1/lib

    LIBS += -lvcomp
    LIBS += -lqwt
    LIBS += -lpython27
    LIBS += -llibumfpack
    LIBS += -llibamd
    LIBS += -lpthreadVCE2
    LIBS += -lxerces-c_static_3
    LIBS += -ladvapi32
    LIBS += -lws2_32

    DESTDIR = ../..
}

# interface
HEADERS      += {{ID}}_interface.h
SOURCES      += {{ID}}_interface.cpp

# headers and sources
HEADERS      += {{ID}}_weakform.h
SOURCES      += {{ID}}_weakform.cpp

TARGET = $$qtLibraryTarget(agros2d_plugin_{{ID}})
DESTDIR = ../../libs
