QT -= GUI
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
    poly2tri/sweep/sweep_context.cc

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
