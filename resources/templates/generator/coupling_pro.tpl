QT += core opengl svg gui network xml webkit xmlpatterns widgets webkitwidgets

TEMPLATE = lib
CONFIG += plugin

OBJECTS_DIR = ../build
MOC_DIR = ../build

DEFINES += WITH_PARALUTION
INCLUDEPATH += ../../3rdparty/paralution/src
DEFINES += WITH_BSON
INCLUDEPATH += ../../3rdparty/bson

INCLUDEPATH += ../../hermes2d/include \
    ../../hermes_common/include \
    ../../3rdparty/libdxfrw

INCLUDEPATH += ../../util
INCLUDEPATH += ../../hermes2d/include
INCLUDEPATH += ../../hermes_common/include
INCLUDEPATH += ../../agros2d-library

LIBS += -lagros_util
LIBS += -lagros_pythonlab_library
LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_library

linux-clang {
    INCLUDEPATH += ../../hermes2d/omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    CONFIG += warn_off

    INCLUDEPATH += /usr/include/google
    INCLUDEPATH += /usr/include/suitesparse

    LIBS += -L../../libs

    DESTDIR = ../../libs
}

win32-msvc2010|win32-msvc2012 {
    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += _WINDOWS

    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    SOURCES      += ../../resources_source/classes/coupling_xml.cpp
    HEADERS      += ../../resources_source/classes/coupling_xml.h

    LIBS += -L../../libs
    LIBS += -L../..

    LIBS += -Lc:/hpfem/hermes/dependencies/lib
    LIBS += -Ld:/hpfem/hermes/dependencies/lib
    LIBS += -Lc:/Python27/libs

    LIBS += -lvcomp
    LIBS += -lpython27
    LIBS += -llibumfpack
    LIBS += -llibamd
    LIBS += -lpthreadVCE2
    LIBS += -ladvapi32
    LIBS += -lws2_32
    LIBS += -lpsapi
    
    CONFIG(release, debug|release) {
        LIBS += -lxerces-c_static_3
    }
    CONFIG(debug, debug|release) {
        LIBS += -lxerces-c_static_3D
    }
}

# interface
HEADERS      += {{ID}}_interface.h
SOURCES      += {{ID}}_interface.cpp

# headers and sources
HEADERS      += {{ID}}_weakform.h
SOURCES      += {{ID}}_weakform.cpp

TARGET = agros2d_plugin_{{ID}}
DESTDIR = ../../libs
