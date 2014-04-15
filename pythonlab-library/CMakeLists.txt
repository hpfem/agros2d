PROJECT(${PYTHONLAB_LIBRARY})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAGROS_PYTHONLAB_DLL")

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/qtsingleapplication)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)

# cython
# message("Cython files - PythonLab")
# message("\tcython -3 pythonlab.pyx")
add_custom_target(CYTHON_PYTHONLAB ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.cpp)

IF(WIN32)
  # In W64 Python, for some reason calling the script (.py) does not work - an error on the cython's side probably
  IF(WIN64)
    ADD_CUSTOM_COMMAND(
        COMMAND C:/Python34/Scripts/cython ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx
        DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx
        OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.cpp)
  ELSE()
    ADD_CUSTOM_COMMAND(
        COMMAND C:/Python34/Scripts/cython.py ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx
        DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx
        OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.cpp)
  ENDIF()
ELSE()
  ADD_CUSTOM_COMMAND(
        COMMAND cython ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx
        DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx
        OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.cpp)
ENDIF()

SET(SOURCES pythonlab.cpp
    pythonlab_functions.cpp
    pythonlab/pythonconsole.cpp
    pythonlab/pythoncompleter.cpp
    pythonlab/pythonhighlighter.cpp
    pythonlab/pythonengine.cpp
    pythonlab/pythonbrowser.cpp
    pythonlab/pythoneditor.cpp
  )
  
SET(HEADERS pythonlab.h
    pythonlab_functions.h
    pythonlab/pythonconsole.h
    pythonlab/pythoncompleter.h
    pythonlab/pythonhighlighter.h
    pythonlab/pythonengine.h
    pythonlab/pythonbrowser.h
    pythonlab/pythoneditor.h
   )

SET(RESOURCES ../resources_source/resources.qrc)
IF(WITH_QT5)
    QT5_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
ELSE(WITH_QT5)
    QT4_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
ENDIF(WITH_QT5)
SET(RC_FILE ../resources_source/resources.rc)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${RESOURCES_RCC} ${RC_FILE})
ADD_DEPENDENCIES(${PROJECT_NAME} CYTHON_PYTHONLAB)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${PYTHON_LIBRARIES} ${AGROS_UTIL} ${QTSINGLEAPPLICATION_LIBRARY})
IF(WITH_QT5)
    QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets OpenGL)
ENDIF(WITH_QT5)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
