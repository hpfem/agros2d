# Setup the Source files for the individual libraries which 
# together form the Netgen-mesher

# Get the actual path to the source directory
STRING (REGEX REPLACE "/cmake/build/" "/" CURRENT_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Get a list of all the source files in the directory	
FILE (GLOB INCLUDED_SRCS "${CURRENT_SRC_PATH}/*.cpp")

# List all the explicitly excluded files here
SET (EXCLUDED_SRCS 
	 ""
	)
LIST(LENGTH EXCLUDED_SRCS LENGTH_EXCLUDED)

# Remove all the excluded files from the above generated list
IF(LENGTH_EXCLUDED GREATER 0)
	LIST (REMOVE_ITEM INCLUDED_SRCS ${EXCLUDED_SRCS})
ENDIF(LENGTH_EXCLUDED GREATER 0)


INCLUDE_DIRECTORIES("${CURRENT_SRC_PATH}")


# Create the Base Package
GET_FILENAME_COMPONENT(_curr_libName ${CURRENT_SRC_PATH} NAME)
ADD_LIBRARY (${_curr_libName} ${${PROJECT_NAME}_LIBRARY_TYPE} ${INCLUDED_SRCS})
TARGET_LINK_LIBRARIES(${_curr_libName} LINK_INTERFACE_LIBRARIES)
TARGET_LINK_LIBRARIES(${_curr_libName} gen)
SET_TARGET_PROPERTIES(${_curr_libName}
                      PROPERTIES
					  SOVERSION ${NETGEN_LIB_SOVERSION}
					  VERSION ${NETGEN_LIB_VERSION}
					  )

SET(NETGEN_INT_LIBS ${NETGEN_INT_LIBS} ${_curr_libName} CACHE INTERNAL "Internal Libraries to be linked to the executable")


IF(${${PROJECT_NAME}_LIBRARY_TYPE} STREQUAL "SHARED")
	INSTALL(TARGETS ${_curr_libName}
			RUNTIME DESTINATION bin
			LIBRARY DESTINATION lib
			)
ENDIF(${${PROJECT_NAME}_LIBRARY_TYPE} STREQUAL "SHARED")

