# agros2d - hp-FEM multiphysics application based on Hermes2D library
OBJECTS_DIR = build
MOC_DIR = build

TEMPLATE = lib

# backup
# VERSION_GIT=$$system(git log --pretty=format:%h | wc -l)
# run cython for python extensions
linux-g++|linux-g++-64|linux-g++-32: CONFIG(release) {
    system(cython --cplus ../resources_source/python/agros2d.pyx)
    system(cython --cplus ../resources_source/python/pythonlab.pyx)
    #system(cd ../ && ./agros2d.sh lang release)
    #system(cd ../ && ./agros2d.sh help)
}

CODECFORTR = UTF-8

RC_FILE = ../resources_source/resources.rc
RESOURCES = ../resources_source/resources.qrc

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes2d/include/mesh/
INCLUDEPATH += ../hermes_common/include
INCLUDEPATH += ../pythonlab-library
INCLUDEPATH += ../util

LIBS += -lagros_3rdparty
LIBS += -lagros_util
LIBS += -lagros_pythonlab_library
LIBS += -lagros2d_hermes2d

SOURCES += value.cpp \
    scene.cpp \
    confdialog.cpp \
    util/conf.cpp \
    util/global.cpp \
    util/xml.cpp \
    util/enums.cpp \
    util/loops.cpp \
    gui/common.cpp \
    gui/imageloader.cpp \
    gui/htmledit.cpp \
    gui/scenewidget.cpp \
    gui/groupbox.cpp \
    gui/latexviewer.cpp \
    gui/physicalfield.cpp \
    gui/valuetimedialog.cpp \
    gui/valuedatatabledialog.cpp \
    gui/valuelineedit.cpp \
    hermes2d/marker.cpp \
    hermes2d/module.cpp \
    hermes2d/solver.cpp \
    hermes2d/field.cpp \
    hermes2d/block.cpp \
    hermes2d/problem.cpp \
    hermes2d/problem_config.cpp \
    mainwindow.cpp \
    scenemarker.cpp \
    scenemarkerdialog.cpp \
    scenebasic.cpp \
    sceneview_common.cpp \
    sceneview_common2d.cpp \
    sceneview_common3d.cpp \
    sceneview_mesh.cpp \
    sceneview_geometry.cpp \
    sceneview_geometry_chart.cpp \
    sceneview_post.cpp \
    sceneview_post2d.cpp \
    sceneview_post3d.cpp \
    sceneview_particle.cpp \
    meshgenerator.cpp \
    meshgenerator_triangle.cpp \
    meshgenerator_gmsh.cpp \
    chartdialog.cpp \
    problemdialog.cpp \
    scenetransformdialog.cpp \
    reportdialog.cpp \
    videodialog.cpp \
    scenemarkerselectdialog.cpp \
    logview.cpp \
    scenebasicselectdialog.cpp \
    postprocessorview.cpp \
    collaboration.cpp \
    resultsview.cpp \
    examplesdialog.cpp \
    datatable.cpp \
    materialbrowserdialog.cpp \
    sceneedge.cpp \
    scenelabel.cpp \
    scenenode.cpp \
    hermes2d/coupling.cpp \
    hermes2d/solutiontypes.cpp \
    preprocessorview.cpp \
    infowidget.cpp \
    settings.cpp \
    hermes2d/solutionstore.cpp \
    moduledialog.cpp \
    parser/lex.cpp \
    hermes2d/bdf2.cpp \
    pythonlab/pythonengine_agros.cpp \
    pythonlab/pyproblem.cpp \
    pythonlab/pyfield.cpp \
    pythonlab/pygeometry.cpp \
    pythonlab/pyview.cpp \
    pythonlab/pyparticletracing.cpp \
    pythonlab/remotecontrol.cpp \
    particle/particle_tracing.cpp \
    particle/mesh_hash.cpp \
    ../resources_source/classes/module_xml.cpp \
    ../resources_source/classes/coupling_xml.cpp \
    ../resources_source/classes/agros2d_structure_xml.cpp \
    ../resources_source/classes/material_xml.cpp \
    ../resources_source/classes/problem_a2d_30_xml.cpp \
    ../hermes2d/src/mesh/subdomains_h2d_xml.cpp \
    ../hermes2d/src/mesh/mesh_h2d_xml.cpp

HEADERS += value.h \
    scene.h \
    util/global.h \
    util/constants.h \
    util/conf.h \
    util/xml.h \
    util/loops.h \
    util/enums.h \
    gui/common.h \
    gui/imageloader.h \
    gui/htmledit.h \
    gui/scenewidget.h \
    gui/latexviewer.h \
    gui/physicalfield.h \
    gui/valuetimedialog.h \
    gui/valuedatatabledialog.h \
    gui/valuelineedit.h \
    hermes2d/marker.h \
    hermes2d/module.h \
    hermes2d/problem.h \
    hermes2d/problem_config.h \
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
    sceneview_geometry_chart.h \
    sceneview_mesh.h \
    sceneview_post.h \
    sceneview_post2d.h \
    sceneview_post3d.h \
    sceneview_particle.h \
    meshgenerator.h \
    meshgenerator_triangle.h \
    meshgenerator_gmsh.h \
    chartdialog.h \
    problemdialog.h \
    scenetransformdialog.h \
    reportdialog.h \
    videodialog.h \
    scenemarkerselectdialog.h \
    logview.h \
    scenebasicselectdialog.h \
    postprocessorview.h \
    collaboration.h \
    resultsview.h \
    examplesdialog.h \
    datatable.h \
    materialbrowserdialog.h \
    confdialog.h \
    hermes2d/solver.h \
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
    pythonlab/remotecontrol.h \
    particle/particle_tracing.h \
    particle/mesh_hash.h

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
    ../resources/xsd/problem_a2d_21_xml.xsd \
    ../resources/xsd/problem_a2d_30_xml.xsd \
    ../resources/xslt/problem_a2d_21_xml.xsl \
    ../resources/xslt/problem_a2d_30_xml.xsl\
    ../resources/xslt/problem_a2d_31_xml.xsl\
    ../resources/modules/rf.xml \
    ../resources_source/python/agros2d.pyx \
    ../resources_source/python/pyproblem.pxi \
    ../resources_source/python/pyfield.pxi \
    ../resources_source/python/pygeometry.pxi \
    ../resources_source/python/pyview.pxi \
    ../resources_source/python/pyparticletracing.pxi \
    ../resources/xsd/subdomains_h2d_xml.xsd \
    ../resources/xsd/module_xml.xsd \
    ../resources/xsd/mesh_h2d_xml.xsd \
    ../resources/xsd/coupling_xml.xsd

linux-clang {
    INCLUDEPATH += ../hermes2d/omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    # DEFINES += WITH_UNITY
    TARGET = ../libs/agros2d_library

    CONFIG += warn_off
    # QMAKE_CXXFLAGS += -Wun
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    QMAKE_CXXFLAGS += -fpermissive
    #target.path = /Applications
    #INSTALLS += target

    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.7

    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/3rdparty/python2.7/config
    LIBS += -lpthread
    LIBS += -lpython2.7
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}

win32-msvc2010 {
    TARGET = ../../agros2d_library

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    DEFINES += WITH_SUPERLU
    DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += AGROS_DLL
    DEFINES += _WINDOWS
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"

    HEADERS += ../util/util/checkversion.h
    HEADERS += ../util/gui/textedit.h
    HEADERS += ../util/gui/about.h
    HEADERS += ../util/gui/filebrowser.h
    HEADERS += ../util/gui/lineeditdouble.h
    HEADERS += ../util/gui/systemoutput.h
    HEADERS += ../pythonlab-library/pythonlab/pythonconsole.h
    HEADERS += ../pythonlab-library/pythonlab/pythonengine.h
    HEADERS += ../pythonlab-library/pythonlab/pythoneditor.h
    HEADERS += ../pythonlab-library/singleapp/qtsingleapplication.h

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += ../3rdparty/ctemplate/windows
    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include
}

include(../agros2d.pri)
include(../agros2d_version.pri)
