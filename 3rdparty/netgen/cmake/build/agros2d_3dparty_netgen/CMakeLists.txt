# Setup the Source files for the individual libraries which 
# together form the Netgen-mesher

# Get the actual path to the source directory
STRING (REGEX REPLACE "/cmake/build/" "/" CURRENT_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Get a list of all the source files in the directory	
FILE (GLOB INCLUDED_SRCS 
      "${CURRENT_SRC_PATH}/nglib.cpp" 
	  )

# List all the explicitly excluded files here
SET (EXCLUDED_SRCS 
	 ""
	)
LIST(LENGTH EXCLUDED_SRCS LENGTH_EXCLUDED)

# Remove all the excluded files from the above generated list
IF(LENGTH_EXCLUDED GREATER 0)
	LIST (REMOVE_ITEM INCLUDED_SRCS ${EXCLUDED_SRCS})
ENDIF(LENGTH_EXCLUDED GREATER 0)

INCLUDE_DIRECTORIES("${NETGEN_LIBSRC_DIR}/interface")
INCLUDE_DIRECTORIES("${CURRENT_SRC_PATH}")

message(${NETGEN_INT_LIBS})

# Create the nglib Base Package
ADD_LIBRARY(${NETGEN_API_NAME} SHARED ${INCLUDED_SRCS})
TARGET_LINK_LIBRARIES(${NETGEN_API_NAME} LINK_INTERFACE_LIBRARIES)
TARGET_LINK_LIBRARIES(${NETGEN_API_NAME} ${EXT_LIB_LIBRARIES} ${NETGEN_INT_LIBS})
SET_TARGET_PROPERTIES(${NETGEN_API_NAME}
                      PROPERTIES
                      SOVERSION ${NETGEN_LIB_SOVERSION}
                      VERSION ${NETGEN_LIB_VERSION}
                      )

# Set up the installation options
INSTALL (TARGETS ${NETGEN_API_NAME} 
         RUNTIME DESTINATION bin
		 LIBRARY DESTINATION lib
		 ARCHIVE DESTINATION lib
		 )
		 
INSTALL (FILES "${CURRENT_SRC_PATH}/${NETGEN_API_NAME}.h" DESTINATION include)
