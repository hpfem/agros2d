PROJECT(${AGROS_LIBRARY})
IF(MSVC)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -DAGROS_LIBRARY_DLL")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} -DAGROS_LIBRARY_DLL")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAGROS_LIBRARY_DLL")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bson)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/paralution/src)

# languages
# message("Translations")
FILE(GLOB lang_files "${CMAKE_HOME_DIRECTORY}/resources_source/lang/*.ts")
foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  add_custom_target(${basefile} ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/lang/${basefile}.ts ${CMAKE_HOME_DIRECTORY}/resources/lang/${basefile}.qm)

  # message("\tlrelease ${basefile}")

  ADD_CUSTOM_COMMAND(
        COMMAND   lrelease ARGS ${LANGFILE} -qm ${CMAKE_HOME_DIRECTORY}/resources/lang/${basefile}.qm
        OUTPUT   ${CMAKE_HOME_DIRECTORY}/resources/lang/${basefile}.qm)
endforeach()

# XSDCXX
# message("XML cpp/h files")
FILE(GLOB xsd_files "${CMAKE_HOME_DIRECTORY}/resources/xsd/*.xsd")
foreach(XSD_FILE ${xsd_files})
  GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
  add_custom_target(${basefile} ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.cpp ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.h ${CMAKE_HOME_DIRECTORY}/resources/xsd/${basefile}.xsd)

  # message("\txsdcxx ${basefile}")

  ADD_CUSTOM_COMMAND(
        COMMAND  ${XSD_BIN} ARGS cxx-tree --generate-ostream --hxx-suffix .h --cxx-suffix .cpp --root-element-first --generate-polymorphic --output-dir ${CMAKE_HOME_DIRECTORY}/resources_source/classes --generate-serialization ${XSD_FILE}
        OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.cpp ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.h)
endforeach()

# cython
# message("Cython files - Agros2D")
# message("\tcython agros2d.pyx")
add_custom_target(CYTHON_AGROS2D ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
        COMMAND   C:/Python27/Scripts/cython.py ARGS --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
        OUTPUT   ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)
ELSE()
  ADD_CUSTOM_COMMAND(
        COMMAND   cython ARGS --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
        OUTPUT   ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)
ENDIF()

SET(SOURCES value.cpp
    scene.cpp
    confdialog.cpp
    util/conf.cpp
    util/global.cpp
    util/memory_monitor.cpp
    util/xml.cpp
    util/enums.cpp
    util/loops.cpp
    util/dxf_filter.cpp
    gui/common.cpp
    gui/imageloader.cpp
    gui/htmledit.cpp
    gui/scenewidget.cpp
    gui/groupbox.cpp
    gui/physicalfield.cpp
    gui/valuetimedialog.cpp
    gui/valuedatatabledialog.cpp
    gui/valuelineedit.cpp
    hermes2d/marker.cpp
    hermes2d/module.cpp
    hermes2d/solver.cpp
    hermes2d/solver_linear.cpp
    hermes2d/solver_newton.cpp
    hermes2d/solver_picard.cpp
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
    util/form_interface.cpp
    util/form_script.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/coupling_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/agros2d_structure_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/material_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/problem_a2d_31_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/form_xml.cpp
    # ../hermes/hermes2d/src/mesh/subdomains_h2d_xml.cpp
    # ../hermes/hermes2d/src/mesh/mesh_h2d_xml.cpp
  )
  
  SET(HEADERS value.h
    scene.h
    util/global.h
    util/memory_monitor.h
    util/constants.h
    util/conf.h
    util/xml.h
    util/loops.h
    util/enums.h
    util/dxf_filter.h
    gui/common.h
    gui/imageloader.h
    gui/htmledit.h
    gui/scenewidget.h
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
    hermes2d/solver_linear.h
    hermes2d/solver_newton.h
    hermes2d/solver_picard.h
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
    )

SET(RESOURCES ../resources_source/resources.qrc)
IF(WITH_QT5)
    QT5_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
ELSE(WITH_QT5)
    QT4_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
ENDIF(WITH_QT5)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${RESOURCES_RCC})
ADD_DEPENDENCIES(${PROJECT_NAME} CYTHON_AGROS2D)

foreach(LANGFILE ${lang_files})
  GET_FILENAME_COMPONENT(basefile ${LANGFILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

foreach(XSD_FILE ${xsd_files})
  GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

IF(WITH_QT5)
  QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets Svg UiTools OpenGL)
ENDIF(WITH_QT5)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${HERMES_LIBRARY} ${HERMES_COMMON_LIBRARY} ${PYTHONLAB_LIBRARY} ${AGROS_UTIL} ${CTEMPLATE_LIBRARY} ${DXFLIB_LIBRARY} ${POLY2TRI_LIBRARY} ${QCUSTOMPLOT_LIBRARY} ${QUAZIP_LIBRARY} ${STB_TRUETYPE_LIBRARY} ${PYTHON_LIBRARIES} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
