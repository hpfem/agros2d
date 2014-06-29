# - Find Togl-1.7 includes and libraries.
# This module finds if Togl-1.7 is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#  TOGL1.7_FOUND              = Togl-1.7 was found
#  TOGL1.7_LIBRARY            = path to Togl-1.7 library (togl-1.7)
#  TOGL1.7_INCLUDE_PATH       = path to where Togl-1.7 include files can be found
#
# Adapted from the CMake FindPackage "FindTCL.cmake"


INCLUDE(CMakeFindFrameworks)
INCLUDE(FindTCL)

SET(TOGL1.7_POSSIBLE_LIB_PATHS
  "${TCL_INCLUDE_PATH_PARENT}/lib"
  "${TCL_LIBRARY_PATH}/Togl1.7"
  "${TCL_LIBRARY_PATH}/Togl-1.7"  
  "${TCL_LIBRARY_PATH}/togl1.7"
  "${TCL_LIBRARY_PATH}/togl-1.7"    
  "${TCL_TCLSH_PATH_PARENT}/lib"
  "${TCL_INCLUDE_PATH_PARENT}/lib/Togl1.7"  
  "${TCL_INCLUDE_PATH_PARENT}/lib/Togl-1.7"    
  "${TCL_INCLUDE_PATH_PARENT}/lib/togl1.7"  
  "${TCL_INCLUDE_PATH_PARENT}/lib/togl-1.7"      
  "/opt/togl1.7/lib"
  "/opt/togl-1.7/lib"
  "/opt/Togl1.7/lib"
  "/opt/Togl-1.7/lib"  
  /usr/lib 
  /usr/local/lib
  )

IF(WIN32)
  GET_FILENAME_COMPONENT(
    ActiveTcl_CurrentVersion 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl;CurrentVersion]" 
    NAME)
  SET(TIX_POSSIBLE_LIB_PATHS ${TIX_POSSIBLE_LIB_PATHS}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.6;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/lib"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/lib"
    "$ENV{ProgramFiles}/Tcl/Lib"
    "C:/Program Files/Tcl/lib" 
    "C:/Tcl/lib" 
	"C:/Program Files/Togl1.7/lib" 
	"C:/Togl1.7/lib" 
	"C:/Program Files/Togl-1.7/lib" 
	"C:/Togl-1.7/lib" 	
    )
ENDIF(WIN32)

FIND_LIBRARY(TOGL1.7_LIBRARY
  NAMES 
  togl-1.7   
  togl1.7
  Togl-1.7
  Togl1.7
  PATHS ${TOGL1.7_POSSIBLE_LIB_PATHS}
  )

CMAKE_FIND_FRAMEWORKS(Togl1.7)

SET(TOGL1.7_FRAMEWORK_INCLUDES)
IF(Togl1.7_FRAMEWORKS)
  IF(NOT TOGL1.7_INCLUDE_PATH)
    FOREACH(dir ${Togl1.7_FRAMEWORKS})
      SET(TOGL1.7_FRAMEWORK_INCLUDES ${TOGL1.7_FRAMEWORK_INCLUDES} ${dir}/Headers)
    ENDFOREACH(dir)
  ENDIF(NOT TOGL1.7_INCLUDE_PATH)
ENDIF(Togl1.7_FRAMEWORKS)

SET(TOGL1.7_POSSIBLE_INCLUDE_PATHS
  "${TCL_LIBRARY_PATH_PARENT}/include"
  "${TCL_INCLUDE_PATH}"
  ${TOGL1.7_FRAMEWORK_INCLUDES} 
  /usr/include
  /usr/local/include
  /usr/include/Togl1.7
  /usr/include/Togl-1.7
  /usr/include/togl1.7
  /usr/include/togl-1.7
  /usr/include/tcl${TCL_LIBRARY_VERSION}
  /usr/include/tcl8.6
  /usr/include/tcl8.5
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
  )

IF(WIN32)
  SET(TOGL1.7_POSSIBLE_INCLUDE_PATHS ${TOGL1.7_POSSIBLE_INCLUDE_PATHS}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.6;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include"
    "$ENV{ProgramFiles}/Tcl/include"
    "C:/Program Files/Tcl/include"
    "C:/Tcl/include"
	"C:/Program Files/Togl-1.7/lib" 
	"C:/Togl-1.7/lib" 	
	"C:/Program Files/Togl1.7/lib" 
	"C:/Togl1.7/lib" 		
    )
ENDIF(WIN32)

FIND_PATH(TOGL1.7_INCLUDE_PATH 
  NAMES togl.h togl_ws.h
  HINTS ${TOGL1.7_POSSIBLE_INCLUDE_PATHS}
  )

# handle the QUIETLY and REQUIRED arguments and set TIX_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Togl1.7 DEFAULT_MSG TOGL1.7_LIBRARY TOGL1.7_INCLUDE_PATH)
SET(TOGL1.7_FIND_REQUIRED ${TOGL1.7_FIND_REQUIRED})
SET(TOGL1.7_FIND_QUIETLY  ${TOGL1.7_FIND_QUIETLY})

MARK_AS_ADVANCED(
  TOGL1.7_INCLUDE_PATH
  TOGL1.7_LIBRARY
  )
