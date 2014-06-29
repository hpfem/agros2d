# - Find Tix includes and libraries.
# This module finds if Tix is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#  TIX_FOUND              = Tix was found
#  TIX_LIBRARY            = path to Tix library (tix8x)
#  TCL_INCLUDE_PATH       = path to where tix include files can be found
#
# Adapted from the CMake FindPackage "FindTCL.cmake"


INCLUDE(CMakeFindFrameworks)
INCLUDE(FindTCL)

SET(TIX_POSSIBLE_LIB_PATHS
  "${TCL_INCLUDE_PATH_PARENT}/lib"
  "${TCL_LIBRARY_PATH}"
  "${TCL_TCLSH_PATH_PARENT}/lib"
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
	"C:/Program Files/Tix/lib" 
	"C:/Tix/lib" 
    )
ENDIF(WIN32)

FIND_LIBRARY(TIX_LIBRARY
  NAMES 
  tix   
  tix86 tix8.6 
  tix85 tix8.5 
  tix84 tix8.4 
  tix83 tix8.3 
  tix82 tix8.2 
  tix80 tix8.0
  PATHS ${TIX_POSSIBLE_LIB_PATHS}
  )

CMAKE_FIND_FRAMEWORKS(Tix)

SET(TIX_FRAMEWORK_INCLUDES)
IF(Tix_FRAMEWORKS)
  IF(NOT TIX_INCLUDE_PATH)
    FOREACH(dir ${Tix_FRAMEWORKS})
      SET(TIX_FRAMEWORK_INCLUDES ${TIX_FRAMEWORK_INCLUDES} ${dir}/Headers)
    ENDFOREACH(dir)
  ENDIF(NOT TIX_INCLUDE_PATH)
ENDIF(Tix_FRAMEWORKS)

SET(TIX_POSSIBLE_INCLUDE_PATHS
  "${TCL_LIBRARY_PATH_PARENT}/include"
  "${TCL_INCLUDE_PATH}"
  ${TCL_FRAMEWORK_INCLUDES} 
  /usr/include
  /usr/local/include
  /usr/include/tcl${TK_LIBRARY_VERSION}
  /usr/include/tcl${TCL_LIBRARY_VERSION}
  /usr/include/tcl8.6
  /usr/include/tcl8.5
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
  )

IF(WIN32)
  SET(TIX_POSSIBLE_INCLUDE_PATHS ${TIX_POSSIBLE_INCLUDE_PATHS}
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
	"C:/Program Files/Tix/lib" 
	"C:/Tix/lib" 	
    )
ENDIF(WIN32)

FIND_PATH(TIX_INCLUDE_PATH 
  NAMES tix.h
  HINTS ${TIX_POSSIBLE_INCLUDE_PATHS}
  )

# handle the QUIETLY and REQUIRED arguments and set TIX_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Tix DEFAULT_MSG TIX_LIBRARY TIX_INCLUDE_PATH)
SET(TIX_FIND_REQUIRED ${TIX_FIND_REQUIRED})
SET(TIX_FIND_QUIETLY  ${TIX_FIND_QUIETLY})

MARK_AS_ADVANCED(
  TIX_INCLUDE_PATH
  TIX_LIBRARY
  )
