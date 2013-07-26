PROJECT(${AGROS_UTIL})

SET(SOURCES util.cpp 
    util/checkversion.cpp 
    util/point.cpp 
    util/system_utils.cpp 
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
    gui/about.h 
    gui/filebrowser.h 
    gui/lineeditdouble.h 
    gui/systemoutput.h 
    gui/textedit.h 
    indicators/indicators.h 
    indicators/indicator_unity.h)
    
QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${HEADERS_MOC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				LIBRARY DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				ARCHIVE DESTINATION ${CMAKE_HOME_DIRECTORY}/libs)