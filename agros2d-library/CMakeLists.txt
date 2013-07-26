PROJECT(${AGROS_LIBRARY})
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/dxflib)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/rapidxml)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bson)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/paralution/src)
EXECUTE_PROCESS(COMMAND "python" "-c" "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()" OUTPUT_VARIABLE PYTHON_INC)
STRING(STRIP ${PYTHON_INC} PYTHON_INC)
INCLUDE_DIRECTORIES(${PYTHON_INC})    
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF()
INCLUDE_DIRECTORIES(.)

SET(SOURCES value.cpp
    scene.cpp 
    confdialog.cpp 
    util/conf.cpp 
    util/global.cpp 
    util/xml.cpp 
    util/enums.cpp 
    util/loops.cpp 
    gui/common.cpp 
    gui/imageloader.cpp 
    gui/htmledit.cpp 
    gui/scenewidget.cpp 
    gui/groupbox.cpp 
    gui/latexviewer.cpp 
    gui/physicalfield.cpp 
    gui/valuetimedialog.cpp 
    gui/valuedatatabledialog.cpp 
    gui/valuelineedit.cpp 
    hermes2d/marker.cpp 
    hermes2d/module.cpp 
    hermes2d/solver.cpp 
    hermes2d/field.cpp 
    hermes2d/block.cpp 
    hermes2d/problem.cpp 
    hermes2d/problem_config.cpp 
    mainwindow.cpp 
    scenemarker.cpp 
    scenemarkerdialog.cpp 
    scenebasic.cpp 
    sceneview_common.cpp 
    sceneview_common2d.cpp 
    sceneview_common3d.cpp 
    sceneview_mesh.cpp 
    sceneview_geometry.cpp 
    sceneview_geometry_chart.cpp 
    sceneview_post.cpp 
    sceneview_post2d.cpp 
    sceneview_post3d.cpp 
    sceneview_particle.cpp 
    meshgenerator.cpp 
    meshgenerator_triangle.cpp 
    meshgenerator_gmsh.cpp 
    chartdialog.cpp 
    problemdialog.cpp 
    scenetransformdialog.cpp 
    reportdialog.cpp 
    videodialog.cpp 
    scenemarkerselectdialog.cpp 
    logview.cpp 
    scenebasicselectdialog.cpp 
    postprocessorview.cpp 
    collaboration.cpp 
    resultsview.cpp 
    examplesdialog.cpp 
    datatable.cpp 
    materialbrowserdialog.cpp 
    sceneedge.cpp 
    scenelabel.cpp 
    scenenode.cpp 
    hermes2d/coupling.cpp 
    hermes2d/solutiontypes.cpp 
    preprocessorview.cpp 
    infowidget.cpp 
    settings.cpp 
    hermes2d/solutionstore.cpp 
    moduledialog.cpp 
    parser/lex.cpp 
    hermes2d/bdf2.cpp 
    pythonlab/pythonengine_agros.cpp 
    pythonlab/pyproblem.cpp 
    pythonlab/pyfield.cpp 
    pythonlab/pygeometry.cpp 
    pythonlab/pyview.cpp 
    pythonlab/pyparticletracing.cpp 
    pythonlab/remotecontrol.cpp 
    particle/particle_tracing.cpp 
    particle/mesh_hash.cpp 
    util/form_interface.cpp 
    util/form_script.cpp 
    ../resources_source/classes/module_xml.cpp 
    ../resources_source/classes/coupling_xml.cpp 
    ../resources_source/classes/agros2d_structure_xml.cpp 
    ../resources_source/classes/material_xml.cpp 
    ../resources_source/classes/problem_a2d_31_xml.cpp 
    ../resources_source/classes/form_xml.cpp 
    ../hermes/hermes2d/src/mesh/subdomains_h2d_xml.cpp 
    ../hermes/hermes2d/src/mesh/mesh_h2d_xml.cpp
  )
  
  SET(HEADERS value.h 
    scene.h 
    util/global.h 
    util/constants.h 
    util/conf.h 
    util/xml.h 
    util/loops.h 
    util/enums.h 
    gui/common.h 
    gui/imageloader.h 
    gui/htmledit.h 
    gui/scenewidget.h 
    gui/latexviewer.h 
    gui/physicalfield.h 
    gui/valuetimedialog.h 
    gui/valuedatatabledialog.h 
    gui/valuelineedit.h 
    hermes2d/marker.h 
    hermes2d/module.h 
    hermes2d/problem.h 
    hermes2d/problem_config.h 
    mainwindow.h 
    scenebasic.h 
    scenemarker.h 
    scenemarkerdialog.h 
    sceneview_data.h 
    # sceneview.h 
    sceneview_common.h 
    sceneview_common2d.h 
    sceneview_common3d.h 
    sceneview_geometry.h 
    sceneview_geometry_chart.h 
    sceneview_mesh.h 
    sceneview_post.h 
    sceneview_post2d.h 
    sceneview_post3d.h 
    sceneview_particle.h 
    meshgenerator.h 
    meshgenerator_triangle.h 
    meshgenerator_gmsh.h 
    chartdialog.h 
    problemdialog.h 
    scenetransformdialog.h 
    reportdialog.h 
    videodialog.h 
    scenemarkerselectdialog.h 
    logview.h 
    scenebasicselectdialog.h 
    postprocessorview.h 
    collaboration.h 
    resultsview.h 
    examplesdialog.h 
    datatable.h 
    materialbrowserdialog.h 
    confdialog.h 
    hermes2d/solver.h 
    sceneedge.h 
    scenelabel.h 
    scenenode.h 
    hermes2d/coupling.h 
    hermes2d/solutiontypes.h 
    preprocessorview.h 
    infowidget.h 
    settings.h 
    hermes2d/field.h 
    hermes2d/block.h 
    hermes2d/solutionstore.h 
    moduledialog.h 
    parser/lex.h 
    hermes2d/bdf2.h 
    hermes2d/plugin_interface.h 
    util/form_interface.h 
    util/form_script.h 
    gui/groupbox.h 
    pythonlab/pythonengine_agros.h 
    pythonlab/pyproblem.h 
    pythonlab/pyfield.h 
    pythonlab/pygeometry.h 
    pythonlab/pyview.h 
    pythonlab/pyparticletracing.h 
    pythonlab/remotecontrol.h 
    particle/particle_tracing.h 
    particle/mesh_hash.h
    )
    
IF(MSVC)
  SET(HEADERS ${HEADERS} 
    ../util/util/checkversion.h
    ../util/gui/textedit.h
    ../util/gui/about.h
    ../util/gui/filebrowser.h
    ../util/gui/lineeditdouble.h
    ../util/gui/systemoutput.h
    ../pythonlab-library/pythonlab/pythonconsole.h
    ../pythonlab-library/pythonlab/pythonengine.h
    ../pythonlab-library/pythonlab/pythoneditor.h
    ../pythonlab-library/singleapp/qtsingleapplication.h)
ENDIF(MSVC)

QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})

SET(RESOURCES ../resources_source/resources.qrc)
QT4_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
SET(RC_FILE ../resources_source/resources.rc)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES_RCC} ${RC_FILE} ${HEADERS_MOC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${OPENGL_LIBRARIES} hermes2d hermes_common)
INSTALL(TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				LIBRARY DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				ARCHIVE DESTINATION ${CMAKE_HOME_DIRECTORY}/libs)