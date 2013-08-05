PROJECT(${POLY2TRI_LIBRARY})

SET(SOURCES common/shapes.cc 
    sweep/advancing_front.cc 
    sweep/cdt.cc 
    sweep/sweep.cc 
    sweep/sweep_context.cc
  )

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
