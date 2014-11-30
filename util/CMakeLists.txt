PROJECT(${AGROS_UTIL})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAGROS_UTIL_DLL")

set(WITH_OPENMP YES)

# Set global compiler parameters.
IF (WITH_OPENMP)
  find_package(OpenMP REQUIRED)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF()

SET(SOURCES util.cpp 
    util/checkversion.cpp 
    util/point.cpp 
    util/system_utils.cpp 
    util/spline.cpp
    util/table.cpp
    gui/about.cpp 
    gui/filebrowser.cpp 
    gui/lineeditdouble.cpp 
    gui/systemoutput.cpp 
    gui/textedit.cpp 
    indicators/indicators.cpp 
    indicators/indicator_unity.cpp)

SET(HEADERS util.h 
    util/checkversion.h 
    util/point.h 
    util/system_utils.h 
    util/spline.h
    util/table.cpp
    gui/about.h 
    gui/filebrowser.h 
    gui/lineeditdouble.h 
    gui/systemoutput.h 
    gui/textedit.h 
    indicators/indicators.h 
    indicators/indicator_unity.h)
    
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
IF(WITH_QT5)
    QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network OpenGL Xml XmlPatterns WebKit WebKitWidgets OpenGL)
ENDIF(WITH_QT5)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
