QT += opengl xml network webkit svg xmlpatterns

OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

RC_FILE = ../src/src.rc
RESOURCES = ../src/src.qrc

INCLUDEPATH += ./ \
    ../src

linux-g++|linux-g++-64|linux-g++-32 {
    TARGET = ../agros2d_pythonlab
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
    TARGET = ../../agros2d_pythonlab
}

SOURCES += main.cpp \
    pythonlab.cpp \
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
    singleapp/qtsinglecoreapplication.cpp \
    ../src/util.cpp \
    ../src/gui/textedit.cpp \
    ../src/gui/filebrowser.cpp \
    ../src/gui/about.cpp \
    ../src/util/checkversion.cpp

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
    singleapp/qtsinglecoreapplication.h \
    ../src/util.h \
    ../src/gui/textedit.h \
    ../src/gui/filebrowser.h \
    ../src/gui/about.h \
    ../src/util/checkversion.h

linux-g++|linux-g++-64|linux-g++-32 {
    QMAKE_LFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    # DEFINES += WITH_UNITY

    HEADERS += singleapp/qtlockedfile_unix.h
    SOURCES += singleapp/qtlockedfile_unix.cpp

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
    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    #DEFINES += XERCES_STATIC_LIBRARY
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
    LIBS += -lpython27
    LIBS += -lpthreadVCE2
    LIBS += -ladvapi32
    LIBS += -lws2_32
}

include(../agros2d_version.pri)
