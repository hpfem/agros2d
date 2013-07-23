# agros2d - hp-FEM multiphysics application based on Hermes2D library
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros_pythonlab_library
LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_library

RC_FILE = ../resources_source/resources.rc
RESOURCES = ../resources_source/resources.qrc

INCLUDEPATH += ../util \
       ../agros2d-library \
       ../pythonlab-library

SOURCES += main.cpp \
    agros_solver.cpp
    
HEADERS += \
    agros_solver.h

linux-clang {
    INCLUDEPATH += ../hermes2d/omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../agros2d_solver
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

win32-msvc2010|win32-msvc2012 {
    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd
    CONFIG += console

    DEFINES += _WINDOWS

    HEADERS      += ../src/hermes2d/problem.h
    HEADERS      += ../src/hermes2d/field.h
    HEADERS      += ../src/scene.h
    HEADERS      += ../src/logview.h
    HEADERS      += ../src/pythonlab/pythonengine_agros.h
    HEADERS      += ../pythonlab/pythonlab/pythonengine.h
    HEADERS      += ../pythonlab/pythonlab/pythoneditor.h

    LIBS += -L..
    TARGET = ../../agros2d_solver
}

include(../agros2d.pri)
include(../agros2d_version.pri)
