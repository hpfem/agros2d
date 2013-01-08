QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
TEMPLATE = lib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += dxflib/dl_writer_ascii.cpp \
    dxflib/dl_dxf.cpp \
    gl2ps/gl2ps.c \
    ctemplate/base/arena.cc \
    ctemplate/htmlparser/htmlparser.cc \
    ctemplate/htmlparser/htmlparser_cpp.h \
    ctemplate/htmlparser/jsparser.cc \
    ctemplate/htmlparser/statemachine.cc \
    ctemplate/per_expand_data.cc \
    ctemplate/template_annotator.cc \
    ctemplate/template_cache.cc \
    ctemplate/template.cc \
    ctemplate/template_dictionary.cc \
    ctemplate/template_modifiers.cc \
    ctemplate/template_namelist.cc \
    ctemplate/template_pathops.cc \
    ctemplate/template_string.cc \
    poly2tri/common/shapes.cc \
    poly2tri/sweep/advancing_front.cc \
    poly2tri/sweep/cdt.cc \
    poly2tri/sweep/sweep.cc \
    poly2tri/sweep/sweep_context.cc \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quaadler32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipnewinfo.cpp \
    quazip/unzip.c \
    quazip/zip.c

HEADERS += \
    quazip/crypt.h \
    quazip/ioapi.h \
    quazip/JlCompress.h \
    quazip/quaadler32.h \
    quazip/quachecksum32.h \
    quazip/quacrc32.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \
    quazip/quazipdir.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazip_global.h \
    quazip/quazip.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h

linux-g++|linux-g++-64|linux-g++-32 {
    TARGET = ../libs/agros2d_3rdparty

    INCLUDEPATH += ctemplate \
            ctemplate/linux \
            .
}

macx-g++ {
    INCLUDEPATH += ctemplate \
            ctemplate/osx \
            .
}

win32-msvc2010 {
    CONFIG += staticlib
    TARGET = ../../libs/agros2d_3rdparty

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    INCLUDEPATH += ctemplate
    INCLUDEPATH += ctemplate/windows

    SOURCES += ctemplate/windows/port.cc
}

macx-g++ {
}
