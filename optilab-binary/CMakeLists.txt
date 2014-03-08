PROJECT(${OPTILAB_BINARY})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bson)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)

# Allow to override the default values in CMake.vars:

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../hermes/cmake)

find_package(XSD REQUIRED)
find_package(XERCES REQUIRED)

SET(SOURCES
    main.cpp
    optilab/optilab.cpp
    optilab/optilab_data.cpp
    optilab/pythonengine_optilab.cpp
    ../3rdparty/qcustomplot/qcustomplot.cpp)
SET(HEADERS
    optilab/optilab.h
    optilab/optilab_data.h
    optilab/pythonengine_optilab.cpp)

SET(RESOURCES ../resources_source/resources.qrc)
IF(WITH_QT5)
    QT5_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
ELSE(WITH_QT5)
    QT4_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
ENDIF(WITH_QT5)

ADD_EXECUTABLE(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${RESOURCES_RCC} ../resources_source/resources.rc)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${AGROS_UTIL} ${PYTHONLAB_LIBRARY} ${CTEMPLATE_LIBRARY} ${XERCES_LIBRARY})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

