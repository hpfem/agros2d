PROJECT(${PYTHONLAB_LIBRARY})
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(src)

IF(WIN32)
  EXECUTE_PROCESS(COMMAND "C:/Python27/Scripts/cython.py" "--cplus" "${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx")
  EXECUTE_PROCESS(COMMAND "C:/Python27/Scripts/cython.py" "--cplus" "${CMAKE_HOME_DIRECTORY}/resources_source/python/pythonlab.pyx")
ELSE(WIN32)
  EXECUTE_PROCESS(COMMAND "cython" "--cplus" "${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx")
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
    singleapp/qtlocalpeer.cpp
    singleapp/qtlockedfile.cpp
    singleapp/qtsingleapplication.cpp
    singleapp/qtsinglecoreapplication.cpp
  )
  
SET(HEADERS pythonlab.h
    pythonlab_functions.h
    pythonlab/pythonconsole.h
    pythonlab/pythoncompleter.h
    pythonlab/pythonhighlighter.h
    pythonlab/pythonengine.h
    pythonlab/pythonbrowser.h
    pythonlab/pythoneditor.h
    singleapp/qtlocalpeer.h
    singleapp/qtlockedfile.h
    singleapp/qtsingleapplication.h
    singleapp/qtsinglecoreapplication.h
    )
    
QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})
    
SET(RESOURCES ../resources_source/resources.qrc)
QT4_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})
SET(RC_FILE ../resources_source/resources.rc)
  
IF(MSVC)
  SET(HEADERS ${HEADERS} ../util/gui/textedit.h
    ../util/gui/about.h
    ../util/gui/filebrowser.h
  singleapp/qtlockedfile.h)
  SET(SOURCES ${SOURCES} singleapp/qtlockedfile_win.cpp)
ENDIF(MSVC)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES_RCC} ${RC_FILE} ${HEADERS_MOC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${AGROS_UTIL} ${PYTHON_MODLIBS} ${PYTHON_LIB})
INSTALL(TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				LIBRARY DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				ARCHIVE DESTINATION ${CMAKE_HOME_DIRECTORY}/libs)