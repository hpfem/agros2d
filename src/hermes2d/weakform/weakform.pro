#-------------------------------------------------
#
# Project created by QtCreator 2011-08-25T12:50:07
#
#-------------------------------------------------

QT += core gui network xml webkit

TEMPLATE = lib
TARGET = lib/weakform
OBJECTS_DIR = build
# CONFIG = += staticlib

INCLUDEPATH += ../../../hermes2d/include \
    ../../../hermes_common/include \
    ../../../lib/muparser \
    ../../../lib/dxflib \
    ../../../lib/rapidxml \
    ../../ \
    ../

SOURCES += electrostatic_steadystate_planar.cpp
# SOURCES += electrostatic_steadystate_axi.cpp


linux-g++|linux-g++-64|linux-g++-32 {
    INCLUDEPATH += /usr/include
}
