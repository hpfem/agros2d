#-------------------------------------------------
#
# Project created by QtCreator 2011-08-25T12:50:07
#
#-------------------------------------------------

QT += core gui network xml webkit xmlpatterns

TEMPLATE = lib
TARGET = build/lib/weakform
OBJECTS_DIR = build
CONFIG += staticlib
CONFIG += warn_off

INCLUDEPATH += ../hermes2d/include \
    ../hermes_common/include \
    ../lib/muparser \
    ../lib/dxflib \
    ../src/ \
    ../src/hermes2d

linux-g++|linux-g++-64|linux-g++-32 {
    system(python ./xml_parser.py)
}

OTHER_FILES += \
    xml_parser.py


win32-msvc2010 {
    # QMAKE_LFLAGS += /MD /openmp
    QMAKE_CXXFLAGS += /MD /MP /openmp /Zc:wchar_t

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../../qwt-6.0.1/src
    INCLUDEPATH += ../lib/ctemplate/windows
	INCLUDEPATH += d:/hpfem/hermes/dependencies/include
}
	
include(src/weakform.pri)