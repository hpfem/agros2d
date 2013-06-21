QT -= GUI
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += base/arena.cc \
    htmlparser/htmlparser.cc \
    htmlparser/htmlparser_cpp.h \
    htmlparser/jsparser.cc \
    htmlparser/statemachine.cc \
    per_expand_data.cc \
    template_annotator.cc \
    template_cache.cc \
    template.cc \
    template_dictionary.cc \
    template_modifiers.cc \
    template_namelist.cc \
    template_pathops.cc \
    template_string.cc

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty_ctemplate

    INCLUDEPATH += ctemplate \
            linux \
            .
}

macx-g++ {
    TARGET = ../../libs/agros_3rdparty_ctemplate
    INCLUDEPATH += ctemplate \
            osx \
            .
}

win32-msvc2010 {
    TARGET = ../../libs/agros_3rdparty_ctemplate

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    INCLUDEPATH += ctemplate
    INCLUDEPATH += windows
    INCLUDEPATH += .
    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    SOURCES += windows/port.cc
}

macx-g++ {
}
