# agros2d - hp-FEM multiphysics application based on Hermes2D library
QT += opengl \
    xml \
    webkit \
    network
SUBDIRS = src-remote

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++:CONFIG(release) system(cython src/python/agros2d.pyx)

# DEFINES += BETA
# DEFINES += VERSION_BETA
DEFINES += VERSION_MAJOR=1
DEFINES += VERSION_MINOR=0
DEFINES += VERSION_SUB=1
DEFINES += VERSION_GIT=336
DEFINES += VERSION_YEAR=2010
DEFINES += VERSION_MONTH=03
DEFINES += VERSION_DAY=23
CONFIG += help
TRANSLATIONS = lang/cs_CZ.ts \
    lang/en_US.ts
CODECFORTR = UTF-8
RC_FILE = agros2d.rc
RESOURCES = agros2d.qrc
TARGET = agros2d
TEMPLATE = app
QMAKE_CXXFLAGS_DEBUG += -w
QMAKE_CXXFLAGS += -w
OBJECTS_DIR = build
MOC_DIR = build
SUBDIRS += src
SOURCES += src/util.cpp \
    src/scene.cpp \
    src/gui.cpp \
    src/hermes2d/hermes_field.cpp \
    src/hermes2d/hermes_general.cpp \
    src/hermes2d/hermes_electrostatic.cpp \
    src/hermes2d/hermes_heat.cpp \
    src/hermes2d/hermes_magnetic.cpp \
    src/hermes2d/hermes_current.cpp \
    src/hermes2d/hermes_elasticity.cpp \
    src/localvalueview.cpp \
    src/surfaceintegralview.cpp \
    src/volumeintegralview.cpp \
    src/main.cpp \
    src/scripteditordialog.cpp \
    src/scripteditorcommandpython.cpp \
    src/scripteditorhighlighter.cpp \
    src/solverdialog.cpp \
    src/mainwindow.cpp \
    src/scenemarker.cpp \
    src/scenebasic.cpp \
    src/scenefunction.cpp \
    src/sceneinfoview.cpp \
    src/sceneview.cpp \
    src/chartdialog.cpp \
    src/problemdialog.cpp \
    src/scenetransformdialog.cpp \
    src/sceneviewdialog.cpp \
    src/optionsdialog.cpp \
    src/helpdialog.cpp \
    src/scenesolution.cpp \
    src/dxflib/dl_writer_ascii.cpp \
    src/dxflib/dl_dxf.cpp \
    src/reportdialog.cpp \
    src/videodialog.cpp \
    src/terminalview.cpp \
    src/scenemarkerselectdialog.cpp \
    src/tooltipview.cpp
HEADERS += src/util.h \
    src/scene.h \
    src/gui.h \
    src/hermes2d/hermes_field.h \
    src/hermes2d/hermes_general.h \
    src/hermes2d/hermes_electrostatic.h \
    src/hermes2d/hermes_heat.h \
    src/hermes2d/hermes_magnetic.h \
    src/hermes2d/hermes_current.h \
    src/hermes2d/hermes_elasticity.h \
    src/localvalueview.h \
    src/surfaceintegralview.h \
    src/volumeintegralview.h \
    src/mainwindow.h \
    src/scripteditordialog.h \
    src/scripteditorcommandpython.h \
    src/scripteditorhighlighter.h \
    src/solverdialog.h \
    src/scenebasic.h \
    src/sceneinfoview.h \
    src/scenemarker.h \
    src/scenefunction.h \
    src/sceneview_data.h \
    src/sceneview.h \
    src/chartdialog.h \
    src/problemdialog.h \
    src/scenetransformdialog.h \
    src/sceneviewdialog.h \
    src/optionsdialog.h \
    src/helpdialog.h \
    src/scenesolution.h \
    src/reportdialog.h \
    src/videodialog.h \
    src/terminalview.h \
    src/scenemarkerselectdialog.h \
    src/tooltipview.h
INCLUDEPATH += src \
    src/dxflib
OTHER_FILES += src/python/agros2d.pyx \
    functions.py \
    version.xml
linux-g++ { 
    # use qmake PREFIX=... to customize your installation
    isEmpty(PREFIX):PREFIX = /usr/local
    
    # install examples
    examples.path = $${PREFIX}/share/agros2d/data
    examples.files = data/*.a2d
    
    # install script
    script.path = $${PREFIX}/share/agros2d/data/script
    script.files = data/script/*.py
    
    # install help
    help.path = $${PREFIX}/share/agros2d/doc/help
    help.files = doc/help/Agros2D.qch \
        doc/help/Agros2D.qhc
    
    # install report
    report.path = $${PREFIX}/share/agros2d/doc/report/template
    report.files = doc/report/template/template.html \
        doc/report/template/default.css
    
    # install translations
    lang.path = $${PREFIX}/share/agros2d/lang
    lang.files = lang/*.qm
    
    # install script
    script.path = $${PREFIX}/share/agros2d
    script.files = *.py
    
    # install pixmap
    pixmap.path = $${PREFIX}/share/pixmaps
    pixmap.files = images/agros2d.xpm
    
    # install desktop
    desktop.path = $${PREFIX}/share/applications
    desktop.files = agros2d.desktop
    
    # install binary
    target.path = $${PREFIX}/bin
    target-remote.path = $${PREFIX}/bin
    target-remote.files = src-remote/agros2d-remote
    
    # "make install" configuration options
    INSTALLS *= target \
        target-remote \
        examples \
        help \
        lang \
        script \
        pixmap \
        report \
        desktop
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/qwt-qt4
    INCLUDEPATH += /usr/include/python2.6
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")
    INCLUDEPATH += ../hermes2d/src
    LIBS += -L../hermes2d/src
    LIBS += -lhermes2d-real
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lJudy
    LIBS += -lpthread
    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")
    LIBS += -lqwt-qt4
}
win32-g++ { 
    INCLUDEPATH += c:/qt/mingw/include
    INCLUDEPATH += c:/Python26/include
    INCLUDEPATH += ../hermes2d/src
    LIBS += -L../hermes2d/src
    LIBS += -lhermes2d-real
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lJudy
    LIBS += -lpthread
    LIBS += -lqwt
    LIBS += -lpython26
}
macx-g++ { 
    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.6
    INCLUDEPATH += ../qwt-5.2.0/src
    INCLUDEPATH += ../hermes2d/src
    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.6/lib/python2.6/config
    LIBS += -L../hermes2d/src
    LIBS += -L../qwt-5.2.0/lib
    LIBS += -lpthread
    LIBS += -lhermes2d-real
    LIBS += -lpython2.6
    LIBS += -lqwt
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lJudy
}
