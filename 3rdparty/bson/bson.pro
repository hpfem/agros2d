QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += bcon.c \
    bson.c \
    encoding.c \
    env.c \
    gridfs.c \
    md5.c \
    # mongo.c \
    numbers.c

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty_bson
}

macx-g++ {    
    TARGET = ../../libs/agros_3rdparty_bson
}

win32-msvc2010 {
    TARGET = ../../libs/agros_3rdparty_bson

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd
}

macx-g++ {
}
