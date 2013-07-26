PROJECT(${AGROS_SOLVER})

SET(SOURCES main.cpp
    agros_solver.cpp)

SET(HEADERS agros_solver.h)

IF(MSVC)
  SET(HEADERS ${HEADERS} ../agros2d-library/hermes2d/problem.h
  ../agros2d-library/hermes2d/field.h
  ../agros2d-library/scene.h
  ../agros2d-library/logview.h
  ../agros2d-library/pythonlab/pythonengine_agros.h
  ../pythonlab-library/pythonlab/pythonengine.h
  ../pythonlab-library/pythonlab/pythoneditor.h)
ENDIF(MSVC)
    
ADD_LIBRARY(${PROJECT_NAME} ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
INSTALL(TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				LIBRARY DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				ARCHIVE DESTINATION ${CMAKE_HOME_DIRECTORY}/libs)