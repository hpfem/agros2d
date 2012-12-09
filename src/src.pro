# agros2d - hp-FEM multiphysics application based on Hermes2D library

OBJECTS_DIR = build
MOC_DIR = build

TEMPLATE = lib

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++|linux-g++-64|linux-g++-32 : CONFIG(release) {
    system(cython --cplus ../resources_source/python/agros2d.pyx)
    system(cython --cplus ../resources_source/python/pythonlab.pyx)
    system(cd ../ && ./agros2d.sh lang release)
    #system(cd ../ && ./agros2d.sh help)
}

TRANSLATIONS = lang/cs_CZ.ts \
    lang/pl_PL.ts \
    lang/de_DE.ts
CODECFORTR = UTF-8
RC_FILE = src.rc
RESOURCES = src.qrc

SOURCES += util/fonts/ubuntu-10.cpp \
    util/fonts/ubuntu-12.cpp \
    util/fonts/ubuntu-14.cpp \
    util/fonts/ubuntu-16.cpp \
    util/fonts/ubuntu-18.cpp \
    util/fonts/ubuntu-20.cpp \
    util/fonts/ubuntu-22.cpp \
    util/fonts/ubuntu-24.cpp \
    util/fonts/liberation-10.cpp \
    util/fonts/liberation-12.cpp \
    util/fonts/liberation-14.cpp \
    util/fonts/liberation-16.cpp \
    util/fonts/liberation-18.cpp \
    util/fonts/liberation-20.cpp \
    util/fonts/liberation-22.cpp \
    util/fonts/liberation-24.cpp \
    util/fonts/google-droid-10.cpp \
    util/fonts/google-droid-12.cpp \
    util/fonts/google-droid-14.cpp \
    util/fonts/google-droid-16.cpp \
    util/fonts/google-droid-18.cpp \
    util/fonts/google-droid-20.cpp \
    util/fonts/google-droid-22.cpp \
    util/fonts/google-droid-24.cpp \
    util/fonts/computer-modern-10.cpp \
    util/fonts/computer-modern-12.cpp \
    util/fonts/computer-modern-14.cpp \
    util/fonts/computer-modern-16.cpp \
    util/fonts/computer-modern-18.cpp \
    util/fonts/computer-modern-20.cpp \
    util/fonts/computer-modern-22.cpp \
    util/fonts/computer-modern-24.cpp \
    util.cpp \
    value.cpp \
    scene.cpp \
    util/checkversion.cpp \
    util/point.cpp \
    util/xml.cpp \
    util/enums.cpp \
    util/glfont.cpp \
    gui/common.cpp \
    gui/chart.cpp \
    gui/filebrowser.cpp \
    gui/imageloader.cpp \
    gui/about.cpp \
    gui/lineeditdouble.cpp \
    gui/htmledit.cpp \
    gui/textedit.cpp \
    gui/systemoutput.cpp \
    gui/scenewidget.cpp \
    gui/groupbox.cpp \
    gui/latexviewer.cpp \
    hermes2d/marker.cpp \
    hermes2d/module.cpp \
    hermes2d/solver.cpp \
    hermes2d/field.cpp \
    hermes2d/block.cpp \
    hermes2d/problem.cpp \
    ../pythonlab/pythonlab/pythonconsole.cpp \
    ../pythonlab/pythonlab/pythoncompleter.cpp \
    ../pythonlab/pythonlab/pythonhighlighter.cpp \
    ../pythonlab/pythonlab/pythonengine.cpp \
    ../pythonlab/pythonlab/pythonbrowser.cpp \
    ../pythonlab/pythonlab/pythoneditor.cpp \
    mainwindow.cpp \
    scenemarker.cpp \
    scenemarkerdialog.cpp \
    scenebasic.cpp \
    sceneview_common.cpp \
    sceneview_common2d.cpp \
    sceneview_common3d.cpp \
    sceneview_mesh.cpp \
    sceneview_geometry.cpp \
    sceneview_post.cpp \
    sceneview_post2d.cpp \
    sceneview_post3d.cpp \
    meshgenerator.cpp \
    meshgenerator_triangle.cpp \
    meshgenerator_gmsh.cpp \
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
    indicators/indicators.cpp \
    indicators/indicator_unity.cpp \
    collaboration.cpp \
    resultsview.cpp \
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
    ../resources_source/classes/coupling_xml.cpp \
    moduledialog.cpp \
    parser/lex.cpp \
    hermes2d/bdf2.cpp \
    pythonlab/pythonengine_agros.cpp \
    pythonlab/pyproblem.cpp \
    pythonlab/pyfield.cpp \
    pythonlab/pygeometry.cpp \
    pythonlab/pyview.cpp \
    pythonlab/pyparticletracing.cpp \
    ../pythonlab/pythonlab_functions.cpp \
    particle/particle_tracing.cpp

HEADERS += util.h \
    value.h \
    scene.h \
    util/constants.h \
    util/checkversion.h \
    util/point.h \
    util/xml.h \
    util/enums.h \
    util/glfont.h \
    gui/common.h \
    gui/chart.h \
    gui/filebrowser.h \
    gui/imageloader.h \
    gui/about.h \
    gui/lineeditdouble.h \
    gui/htmledit.h \
    gui/textedit.h \
    gui/systemoutput.h \
    gui/scenewidget.h \
    gui/latexviewer.h \
    hermes2d/marker.h \
    hermes2d/module.h \
    hermes2d/problem.h \
    ../pythonlab/pythonlab/pythonconsole.h \
    ../pythonlab/pythonlab/pythoncompleter.h \
    ../pythonlab/pythonlab/pythonhighlighter.h \
    ../pythonlab/pythonlab/pythonengine.h \
    ../pythonlab/pythonlab/pythonbrowser.h \
    ../pythonlab/pythonlab/pythoneditor.h \
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
    meshgenerator_triangle.h \
    meshgenerator_gmsh.h \
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
    indicators/indicators.h \
    indicators/indicator_unity.h \
    collaboration.h \
    resultsview.h \
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
    hermes2d/solutionstore.h \
    moduledialog.h \
    parser/lex.h \
    hermes2d/bdf2.h \
    hermes2d/plugin_interface.h \
    gui/groupbox.h \
    pythonlab/pythonengine_agros.h \
    pythonlab/pyproblem.h \
    pythonlab/pyfield.h \
    pythonlab/pygeometry.h \
    pythonlab/pyview.h \
    pythonlab/pyparticletracing.h \
    particle/particle_tracing.h

OTHER_FILES += functions.py \
    version.xml \
    ../resources/modules/rf.xml \
    ../resources/modules/magnetic.xml \
    ../resources/modules/heat.xml \
    ../resources/modules/electrostatic.xml \
    ../resources/modules/elasticity.xml \
    ../resources/modules/current.xml \
    ../resources/modules/acoustic.xml \
    ../resources/modules/flow.xml \
    ../resources/couplings/heat-elasticity.xml \
    ../resources/couplings/current-heat.xml \
    ../resources/couplings/magnetic-heat.xml \
    ../resources/couplings/flow-heat.xml \
    ../resources/custom.xml \
    ../resources/a2d_convert_table.xml \
    ../resources/materials/Copper.xml \
    ../resources/materials/Aluminum.xml \
    ../resources/materials/Iron.xml \
    ../resources/xsd/problem_a2d_xml.xsd \
    ../resources/xslt/problem_a2d_xml.xsl \
    ../resources/modules/rf.xml \
    ../resources_source/python/agros2d.pyx \
    ../resources_source/python/pyproblem.pxi \
    ../resources_source/python/pyfield.pxi \
    ../resources_source/python/pyview.pxi \
    ../resources_source/python/pyparticletracing.pxi \
    ../resources/xsd/subdomains_h2d_xml.xsd \
    ../resources/xsd/module_xml.xsd \
    ../resources/xsd/mesh_h2d_xml.xsd \
    ../resources/xsd/coupling_xml.xsd

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include
LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3rdparty

linux-g++|linux-g++-64|linux-g++-32 {
    # DEFINES += WITH_UNITY
    TARGET = ../libs/agros2d

    CONFIG += warn_off
    # QMAKE_CXXFLAGS += -Wun
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
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/3rdparty/python2.7/config
    LIBS += -L../../qwt-6.0.1/lib
    LIBS += -lpthread
    LIBS += -lpython2.7
    LIBS += -lqwt
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}

win32-msvc2010 {
    CONFIG += staticlib
    TARGET = ../../libs/agros2d

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += AGROS
    #DEFINES += XML_LIBRARY
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../../qwt-6.0.1/src
    INCLUDEPATH += ../3rdparty/ctemplate/windows
    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include
}

include(../agros2d.pri)
