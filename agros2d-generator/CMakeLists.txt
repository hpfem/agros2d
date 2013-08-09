PROJECT(${AGROS_GENERATOR})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/paralution/src)

SET(SOURCES main.cpp 
    generator.cpp 
    generator_module.cpp 
    generator_coupling.cpp
)

SET(HEADERS generator.h
    generator_module.h
    generator_coupling.h
)

IF(WIN32)
  SET(SOURCES_XML ../resources_source/classes/module_xml.cpp
    ../resources_source/classes/coupling_xml.cpp
    ../resources_source/classes/agros2d_structure_xml.cpp
    ../resources_source/classes/material_xml.cpp
    ../resources_source/classes/problem_a2d_31_xml.cpp
    ../resources_source/classes/form_xml.cpp
)
ENDIF(WIN32)
    
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES} ${SOURCES_XML} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${AGROS_LIBRARY} ${PYTHONLAB_LIBRARY} ${CTEMPLATE_LIBRARY} ${QCUSTOMPLOT_LIBRARY} ${QUAZIP_LIBRARY} ${STB_TRUETYPE_LIBRARY})
IF(WITH_QT5)
    QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets OpenGL)
ENDIF(WITH_QT5)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY})
