# agros2d - hp-FEM multiphysics application based on Hermes2D library
QT += opengl \
    xml \
    network \
    webkit \
    svg \
    xmlpatterns

DEFINES += VERSION_MAJOR=3
DEFINES += VERSION_MINOR=0
DEFINES += VERSION_SUB=0
DEFINES += VERSION_GIT=823
DEFINES += VERSION_YEAR=2012
DEFINES += VERSION_MONTH=2
DEFINES += VERSION_DAY=26

DEFINES += WEAKFORM_FACTORY

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++|linux-g++-64|linux-g++-32 :CONFIG(release) {
    system(cython --cplus ../resources_source/python/agros2d.pyx)
    system(cd ../ && ./agros2d.sh lang release)
    #system(cd ../ && ./agros2d.sh help)
}
TRANSLATIONS = lang/cs_CZ.ts \
    lang/pl_PL.ts \
    lang/de_DE.ts
CODECFORTR = UTF-8
# RC_FILE = src.rc
RESOURCES = src.qrc
TARGET = agros2d
DESTDIR = ../
TEMPLATE = app
CONFIG += warn_off
# QMAKE_CXXFLAGS_DEBUG += -Wno-builtin-macro-redefined -Wunused-variable -Wreturn-type
# QMAKE_CXXFLAGS += -fno-strict-aliasing -Wno-builtin-macro-redefined
# QMAKE_CXXFLAGS_DEBUG += -w
# QMAKE_CXXFLAGS += -w
# QMAKE_CXXFLAGS_DEBUG += -Wuninitialized
# QMAKE_CXXFLAGS += -Wuninitialized
OBJECTS_DIR = build
MOC_DIR = build
SUBDIRS += src
SOURCES += util.cpp \
    value.cpp \
    scene.cpp \
    gui.cpp \
    hermes2d/localpoint.cpp \
    hermes2d/volumeintegral.cpp \
    hermes2d/surfaceintegral.cpp \
    hermes2d/marker.cpp \
    hermes2d/module.cpp \
    hermes2d/post_values.cpp \
    hermes2d/solver.cpp \
    hermes2d/field.cpp \
    hermes2d/block.cpp \
    hermes2d/problem.cpp \
    pythonlab/pythonconsole.cpp \
    pythonlab/pythoncompleter.cpp \
    pythonlab/pythonhighlighter.cpp \
    pythonlab/pythonengine.cpp \
    pythonlab/pythonbrowser.cpp \
    pythonlab/pythoneditor.cpp \
    pythonlabagros.cpp \
    main.cpp \
    mainwindow.cpp \
    scenemarker.cpp \
    scenemarkerdialog.cpp \
    scenebasic.cpp \
    # sceneview.cpp \
    sceneview_common.cpp \
    sceneview_common2d.cpp \
    sceneview_common3d.cpp \
    sceneview_mesh.cpp \
    sceneview_geometry.cpp \
    sceneview_post.cpp \
    sceneview_post2d.cpp \
    sceneview_post3d.cpp \
    meshgenerator.cpp \
    chartdialog.cpp \
    problemdialog.cpp \
    scenetransformdialog.cpp \
    reportdialog.cpp \
    videodialog.cpp \
    scenemarkerselectdialog.cpp \
    tooltipview.cpp \
    logview.cpp \
    scenebasicselectdialog.cpp \
    postprocessorview.cpp \
    style/stylehelper.cpp \
    style/styleanimator.cpp \
    style/manhattanstyle.cpp \
    indicators/indicators.cpp \
    indicators/indicator_unity.cpp \
    collaboration.cpp \
    resultsview.cpp \
    hermes2d/weakform_parser.cpp\
    alter_newton_solver.cpp \
    datatable.cpp \
    datatabledialog.cpp \
    materialbrowserdialog.cpp \
    conf.cpp \
    confdialog.cpp \
    hermes2d/module_agros.cpp \
    sceneedge.cpp \
    scenelabel.cpp \
    scenenode.cpp \
    hermes2d/coupling.cpp \
    hermes2d/solutiontypes.cpp \
    preprocessorview.cpp \
    infowidget.cpp \
    settings.cpp \
    hermes2d/solutionstore.cpp \
    ../resources_source/classes/module_xml.cpp \
    ../resources_source/classes/coupling_xml.cpp

HEADERS += util.h \
    value.h \
    scene.h \
    gui.h \
    hermes2d/localpoint.h \
    hermes2d/volumeintegral.h \
    hermes2d/surfaceintegral.h \
    hermes2d/marker.h \
    hermes2d/module.h \
    hermes2d/post_values.h \
    hermes2d/problem.h \
    pythonlab/pythonconsole.h \
    pythonlab/pythoncompleter.h \
    pythonlab/pythonhighlighter.h \
    pythonlab/pythonengine.h \
    pythonlab/pythonbrowser.h \
    pythonlab/pythoneditor.h \
    pythonlabagros.h \
    mainwindow.h \
    scenebasic.h \
    scenemarker.h \
    scenemarkerdialog.h \
    sceneview_data.h \
    # sceneview.h \
    sceneview_common.h \
    sceneview_common2d.h \
    sceneview_common3d.h \
    sceneview_geometry.h \
    sceneview_mesh.h \
    sceneview_post.h \
    sceneview_post2d.h \
    sceneview_post3d.h \
    meshgenerator.h \
    chartdialog.h \
    problemdialog.h \
    scenetransformdialog.h \
    reportdialog.h \
    videodialog.h \
    scenemarkerselectdialog.h \
    tooltipview.h \
    logview.h \
    scenebasicselectdialog.h \
    postprocessorview.h \
    style/stylehelper.h \
    style/styleanimator.h \
    style/manhattanstyle.h \
    indicators/indicators.h \
    indicators/indicator_unity.h \
    collaboration.h \
    resultsview.h \
    hermes2d/weakform_parser.h \
    alter_newton_solver.h \
    datatable.h \
    datatabledialog.h \
    materialbrowserdialog.h \
    conf.h \
    confdialog.h \
    hermes2d/solver.h \
    hermes2d/module_agros.h \
    sceneedge.h \
    scenelabel.h \
    scenenode.h \
    hermes2d/coupling.h \
    hermes2d/solutiontypes.h \
    preprocessorview.h \
    infowidget.h \
    settings.h \
    hermes2d/field.h \
    hermes2d/block.h \
    hermes2d/solutionstore.h

INCLUDEPATH += . \
    ../lib \
    ../lib/ctemplate \
    ../lib/muparser \
    ../lib/dxflib \
    ../lib/rapidxml \
    ../hermes_common \
    ../hermes2d

OTHER_FILES += python/agros2d.pyx \
    functions.py \
    version.xml \
    ../resources/modules/rf.xml \
    ../resources/modules/magnetic.xml \
    ../resources/modules/heat.xml \
    ../resources/modules/electrostatic.xml \
    ../resources/modules/elasticity.xml \
    ../resources/modules/current.xml \
    ../resources/modules/acoustic.xml \
    ../resources/couplings/heat-elasticity.xml \
    ../resources/couplings/current-heat.xml \
    ../resources/couplings/magnetic-heat.xml \
    ../resources/custom.xml \
    ../resources/a2d_convert_table.xml \
    ../resources/materials/Copper.xml \
    ../resources/materials/Aluminum.xml \
    ../resources/materials/Iron.xml \
    ../resources/xsd/problem_a2d_xml.xsd \
    ../resources/xslt/problem_a2d_xml.xsl \
    ../resources/modules/rf.xml \
    ../resources_source/python/agros2d.pyx \
    ../resources/xsd/subdomains_h2d_xml.xsd \
    ../resources/xsd/module_xml.xsd \
    ../resources/xsd/mesh_h2d_xml.xsd \
    ../resources/xsd/coupling_xml.xsd

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include
LIBS += -lhermes2d
LIBS += -llib
LIBS += -lweakform

linux-g++|linux-g++-64|linux-g++-32 {
    QMAKE_LFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    # DEFINES += WITH_UNITY

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")
    INCLUDEPATH += ../lib/ctemplate/unix

    LIBS += -L../hermes2d/build/lib
    LIBS += -L../lib/build/lib
    LIBS += -L../weakform/build/lib

    LIBS += -lumfpack
    LIBS += -lxerces-c
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lpthread
    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")
    # qwt
    INCLUDEPATH += /usr/include/qwt
    LIBS += -lqwt

    # mumps
    contains(CONFIG, WITH_MUMPS) {
        DEFINES += WITH_MUMPS
        LIBS += -ldmumps_seq
        LIBS += -lzmumps_seq
    }
    # superlu
    contains(CONFIG, WITH_SUPERLU) {
        DEFINES += WITH_SUPERLU
        INCLUDEPATH += /usr/include/superlu
        LIBS += -lsuperlu
    }

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
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target

    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.7
    INCLUDEPATH += ../../qwt-6.0.1/src

    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config
    LIBS += -L../../qwt-6.0.1/lib
    LIBS += -lpthread
    LIBS += -lpython2.7
    LIBS += -lqwt
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}

win32-msvc2010 {
    # QMAKE_LFLAGS += /MD /openmp
    QMAKE_CXXFLAGS += /MD /MP /openmp /Zc:wchar_t

    #DEFINES += XERCES_STATIC_LIBRARY
    #DEFINES += XML_LIBRARY
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"
	
    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../../qwt-6.0.1/src
    INCLUDEPATH += ../lib/ctemplate/windows
	INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -L../hermes2d/debug/build/lib
    LIBS += -L../lib/debug/build/lib
    LIBS += -L../weakform/debug/build/lib

    LIBS += -Lc:/Python27/libs
    LIBS += -L../../qwt-6.0.1/lib
    LIBS += -lvcomp
    LIBS += -lqwt
    LIBS += -lpython27
    LIBS += -llibumfpack
    LIBS += -llibamd
    LIBS += -llibpthreadVCE2
    #LIBS += -lmsvcrt
    LIBS += -lxerces-c_static_3
    LIBS += -ladvapi32
    LIBS += -lws2_32
}
