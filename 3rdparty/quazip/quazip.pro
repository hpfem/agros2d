QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += JlCompress.cpp \
    qioapi.cpp \
    quaadler32.cpp \
    quacrc32.cpp \
    quagzipfile.cpp \
    quaziodevice.cpp \
    quazip.cpp \
    quazipdir.cpp \
    quazipfile.cpp \
    quazipnewinfo.cpp \
    unzip.c \
    zip.c

HEADERS += \
    crypt.h \
    ioapi.h \
    JlCompress.h \
    quaadler32.h \
    quachecksum32.h \
    quacrc32.h \
    quagzipfile.h \
    quaziodevice.h \
    quazipdir.h \
    quazipfile.h \
    quazipfileinfo.h \
    quazip_global.h \
    quazip.h \
    quazipnewinfo.h

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty_quazip
}

macx-g++ {
    TARGET = ../libs/agros_3rdparty_quazip
}

win32-msvc2010|win32-msvc2012 {
    TARGET = ../../libs/agros_3rdparty_quazip

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include
}

macx-g++ {
}
