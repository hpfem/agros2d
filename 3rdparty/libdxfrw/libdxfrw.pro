QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += drw_entities.cpp \
    drw_objects.cpp \
    drw_textcodec.cpp \
    dxfreader.cpp \
    dxfwriter.cpp \
    libdxfrw.cpp

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty_libdxfrw
}

macx-g++ {    
    TARGET = ../../libs/agros_3rdparty_libdxfrw
}

win32-msvc2010 {
    TARGET = ../../libs/agros_3rdparty_libdxfrw

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd
}

macx-g++ {
}
