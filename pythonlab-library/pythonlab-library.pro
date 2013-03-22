QT += opengl xml network webkit svg xmlpatterns widgets printsupport webkitwidgets

OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib

RC_FILE = ../resources_source/resources.rc
RESOURCES = ../resources_source/resources.qrc

INCLUDEPATH += ./ \
    ../util

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros2d_pythonlab_library
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

win32-msvc2010 {
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt

    LIBS += -L..
    TARGET = ../../agros2d_pythonlab_library
}

SOURCES += pythonlab.cpp \
    pythonlab_functions.cpp \
    pythonlab/pythonconsole.cpp \
    pythonlab/pythoncompleter.cpp \
    pythonlab/pythonhighlighter.cpp \
    pythonlab/pythonengine.cpp \
    pythonlab/pythonbrowser.cpp \
    pythonlab/pythoneditor.cpp \
    singleapp/qtlocalpeer.cpp \
    singleapp/qtlockedfile.cpp \
    singleapp/qtsingleapplication.cpp \
    singleapp/qtsinglecoreapplication.cpp

HEADERS  += pythonlab.h \
    pythonlab_functions.h \
    pythonlab/pythonconsole.h \
    pythonlab/pythoncompleter.h \
    pythonlab/pythonhighlighter.h \
    pythonlab/pythonengine.h \
    pythonlab/pythonbrowser.h \
    pythonlab/pythoneditor.h \
    singleapp/qtlocalpeer.h \
    singleapp/qtlockedfile.h \
    singleapp/qtsingleapplication.h \
    singleapp/qtsinglecoreapplication.h

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    # DEFINES += WITH_UNITY

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")

    LIBS += -L../libs

    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")

    # unity
    contains(CONFIG, WITH_UNITY) {
        DEFINES += WITH_UNITY
        INCLUDEPATH += /usr/include/unity/unity
        INCLUDEPATH += /usr/include/glib-2.0
        INCLUDEPATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include
        INCLUDEPATH += /usr/include/dee-1.0
        INCLUDEPATH += /usr/include/libdbusmenu-0.4
        LIBS += -lunity
    }
}

macx-g++ {
    HEADERS += singleapp/qtlockedfile_unix.h
    SOURCES += singleapp/qtlockedfile_unix.cpp

    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += ../3rdparty/ctemplate/osx
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.7
    INCLUDEPATH += ../../qwt-6.0.1/src

    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config
    LIBS += -lpthread
    LIBS += -lpython2.7
}

win32-msvc2010 {
    # /openmp
    QMAKE_CXXFLAGS += /MP  /Zc:wchar_t
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    #DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += AGROS_PYTHONLAB_API
    #DEFINES += XML_LIBRARY
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"

    HEADERS += singleapp/qtlockedfile_win.h
    SOURCES += singleapp/qtlockedfile_win.cpp

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../3rdparty/ctemplate/windows
    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -L../libs
    LIBS += -L../3rdparty/libs
    LIBS += -Lc:/hpfem/hermes/dependencies/lib
    LIBS += -Ld:/hpfem/hermes/dependencies/lib

    LIBS += -Lc:/Python27/libs
    LIBS += -lvcomp
    LIBS += -lpthreadVCE2
    LIBS += -ladvapi32
    LIBS += -lws2_32
    
    CONFIG(release, debug|release) {
        LIBS += -lpython27
    }
    CONFIG(debug, debug|release) {
        LIBS += -lpython27_d
    }
}

include(../agros2d_version.pri)
