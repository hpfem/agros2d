PROJECT(${MATIO_LIBRARY})

IF(WIN32)
ADD_CUSTOM_COMMAND(COMMAND copy ARGS "matioConfigWin.h" "matioConfig.h" OUTPUT "matioConfig.h" DEPENDS "matioConfig.h")
ELSE()
ADD_CUSTOM_COMMAND(COMMAND cp ARGS "matioConfigLinux.h" "matioConfig.h" OUTPUT "matioConfig.h" DEPENDS "matioConfig.h")
ENDIF()

SET(SOURCES endian.c
    inflate.c
    io.c
    mat4.c
    mat5.c
    mat73.c
    mat.c
    matvar_cell.c
    matvar_struct.c
    read_data.c
    snprintf.c
    matioConfig.h
)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
