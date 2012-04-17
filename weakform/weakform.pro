#-------------------------------------------------
#
# Project created by QtCreator 2011-08-25T12:50:07
#
#-------------------------------------------------

QT += core gui network xml webkit xmlpatterns

TEMPLATE = lib
TARGET = ../build/weakform/lib/weakform
OBJECTS_DIR = ../build/weakform
CONFIG += staticlib
CONFIG += warn_off

INCLUDEPATH += ../hermes2d/include \
    ../hermes_common/include \
    ../lib/muparser \
    ../lib/dxflib \
    ../lib/rapidxml \
    ../src/ \
    ../src/hermes2d

linux-g++|linux-g++-64|linux-g++-32 {
    system(python ./gen_weakforms.py)
}

OTHER_FILES += \
    gen_weakforms.py

include(src/weakform.pri)






