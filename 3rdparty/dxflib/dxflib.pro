QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += dl_writer_ascii.cpp \
    dl_dxf.cpp

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../../libs/agros_3rdparty_dxflib
}

macx-g++ {    
    TARGET = ../../libs/agros_3rdparty_dxflib
}

win32-msvc2010 {
    TARGET = ../../../libs/agros_3rdparty_dxflib

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd
}

macx-g++ {
}
