QT += network
OBJECTS_DIR = build

TEMPLATE = app

HEADERS = client.h
SOURCES = main.cpp \
          client.cpp

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../agros2d_remote
}

win32-msvc2010 {
    TARGET = ../../agros2d_remote
}
