QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += common/shapes.cc \
    sweep/advancing_front.cc \
    sweep/cdt.cc \
    sweep/sweep.cc \
    sweep/sweep_context.cc

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty_poly2tri
}

macx-g++ {
    TARGET = ../libs/agros_3rdparty_poly2tri
}

win32-msvc2010 {
    TARGET = ../../libs/agros_3rdparty_poly2tri

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd
}

macx-g++ {
}
