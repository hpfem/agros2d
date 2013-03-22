QT += opengl xml network webkit svg xmlpatterns widgets printsupport webkitwidgets

QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += util.cpp \
    util/checkversion.cpp \
    util/point.cpp \
    util/system_utils.cpp \
    gui/about.cpp \
    gui/filebrowser.cpp \
    gui/lineeditdouble.cpp \
    gui/systemoutput.cpp \
    gui/textedit.cpp \
    indicators/indicators.cpp \
    indicators/indicator_unity.cpp

HEADERS += util.h \
    util/checkversion.h \
    util/point.h \
    util/system_utils.h \
    gui/about.h \
    gui/filebrowser.h \
    gui/lineeditdouble.h \
    gui/systemoutput.h \
    gui/textedit.h \
    indicators/indicators.h \
    indicators/indicator_unity.h

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros2d_util
}

macx-g++ {
    CONFIG += staticlib
    TARGET = ../libs/agros2d_util
}

win32-msvc2010 {
    TARGET = ../../agros2d_util

    # /openmp
    QMAKE_CXXFLAGS += /MP /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    DEFINES += AGROS_DLL

    LIBS += -lpsapi # process memory usage (system_utils.h)
}

macx-g++ {
}

include(../agros2d_version.pri)
