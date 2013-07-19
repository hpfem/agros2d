QT += core opengl svg gui network xml webkit xmlpatterns widgets webkitwidgets

TEMPLATE = lib
CONFIG += plugin

OBJECTS_DIR = build
MOC_DIR = build

TARGET = agros2d_forms_example
DESTDIR = ../../libs

INCLUDEPATH += ../../util
INCLUDEPATH += ../../hermes2d/include
INCLUDEPATH += ../../hermes_common/include
INCLUDEPATH += ../../agros2d-library
INCLUDEPATH += ../../pythonlab-library

LIBS += -lagros_util
LIBS += -lagros_pythonlab_library
LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_library

HEADERS      += example.h
SOURCES      += example.cpp

INCLUDEPATH += ../../3rdparty/libdxfrw
INCLUDEPATH += ../../3rdparty

linux-clang {
    INCLUDEPATH += ../../hermes2d/omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    CONFIG += warn_off    

    INCLUDEPATH += /usr/include/google
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += ../../3rdparty/ctemplate/linux

    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")

    LIBS += -L../../libs
}

win32-msvc2010 {
    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    INCLUDEPATH += ../../3rdparty/ctemplate/linux

    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -L../../libs
    LIBS += -L../..

    LIBS += -Lc:/hpfem/hermes/dependencies/lib
    LIBS += -Ld:/hpfem/hermes/dependencies/lib
    LIBS += -Lc:/Python27/libs

    LIBS += -lvcomp
    LIBS += -lpython27
    LIBS += -llibumfpack
    LIBS += -llibamd
    LIBS += -lpthreadVCE2
    LIBS += -ladvapi32
    LIBS += -lws2_32
    LIBS += -lpsapi
    
    CONFIG(release, debug|release) {
        LIBS += -lxerces-c_static_3
    }
    CONFIG(debug, debug|release) {
        LIBS += -lxerces-c_static_3D
    }
}
