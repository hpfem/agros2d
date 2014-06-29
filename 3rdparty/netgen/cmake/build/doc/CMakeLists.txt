# Setup the Documentation files for the Netgen-mesher

# Get the actual path to the source directory
STRING (REGEX REPLACE "/cmake/build/" "/" CURRENT_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Get a list of all the files to be included	
FILE (GLOB INCLUDED_DOCS 
      "${CURRENT_SRC_PATH}/*.pdf" 
	  )

SET (EXCLUDED_DOCS
     ""
	 )
LIST(LENGTH EXCLUDED_DOCS LENGTH_EXCLUDED)

# Remove all the excluded files from the above generated list
IF(LENGTH_EXCLUDED GREATER 0)
	LIST (REMOVE_ITEM INCLUDED_DOCS ${EXCLUDED_DOCS})
ENDIF(LENGTH_EXCLUDED GREATER 0) 

INSTALL (FILES ${INCLUDED_DOCS} DESTINATION share/doc)
