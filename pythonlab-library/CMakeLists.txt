PROJECT(${PYTHONLAB_LIBRARY})
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/qtsingleapplication)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(.)

IF(WIN32)
  EXECUTE_PROCESS(COMMAND "C:/Python27/Scripts/cython.py" "--cplus" "${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx")
ELSE(WIN32)
  EXECUTE_PROCESS(COMMAND "cython" "--cplus" "${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx")
ENDIF(WIN32)

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
  
IF(MSVC)
  SET(HEADERS ${HEADERS} ../util/gui/textedit.h
    ../util/gui/about.h
    ../util/gui/filebrowser.h)  
ENDIF(MSVC)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${RESOURCES_RCC} ${RC_FILE})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${PYTHON_LIBRARIES} ${AGROS_UTIL} ${QTSINGLEAPPLICATION_LIBRARY})
INSTALL(TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				LIBRARY DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				ARCHIVE DESTINATION ${CMAKE_HOME_DIRECTORY}/libs)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libs)
