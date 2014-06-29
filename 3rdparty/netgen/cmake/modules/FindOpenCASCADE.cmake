# - Try to find OpenCASCADE libraries
### Does not test what version has been found,though
### that could be done by parsing Standard_Version.hxx


# Once done, this will define
#  OpenCASCADE_FOUND - true if OCC has been found
#  OpenCASCADE_INCLUDE_DIR - the OCC include dir
#  OpenCASCADE_LIBRARIES - names of OCC libraries
#  OpenCASCADE_LINK_DIRECTORY - location of OCC libraries

# ${OpenCASCADE_FOUND} is cached, so once OCC is found this block shouldn't have to run again

IF( NOT OpenCASCADE_FOUND STREQUAL TRUE )
  IF (${CMAKE_SIZEOF_VOID_P} MATCHES "8") # It is 64bit, otherwise 32 bit systems match 4
	set( _bit_width 64)
  ELSE (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
	set( _bit_width 32)
  ENDIF(${CMAKE_SIZEOF_VOID_P} MATCHES "8")

  IF(UNIX)
    set( _platform_name ${CMAKE_SYSTEM_NAME})
    set( _incsearchpath /usr/include/opencascade /opt/occ/inc $ENV{CASROOT}/inc )
    if (APPLE)
      set( _testlibname libTKernel.dylib )
    else (APPLE)
      set( _testlibname libTKernel.so )
    endif (APPLE)
	
	IF(${_bit_width} MATCHES "64")
		set( _libsearchpath /usr/lib64 /opt/occ/ros/${_platorm_name}/lib $ENV{CASROOT}/${_platorm_name}/lib )
	ELSE(${_bit_width} MATCHES "64")
		set( _libsearchpath /usr/lib /opt/occ/ros/${_platorm_name}/lib $ENV{CASROOT}/${_platorm_name}/lib )	
	ENDIF(${_bit_width} MATCHES "64")	
  ELSE(UNIX)
    IF(WIN32)
      MESSAGE("************ FindOpenCASCADE.cmake has not been tried on windows and may or may not work! *************")
	  set( _platform_name "Win${_bit_width}")
	  IF(MSVC11)
		set( _compiler_name "vc11")
	  ELSEIF(MSVC10)
	    set( _compiler_name "vc10")
	  ELSEIF(MSVC90)
		set( _compiler_name "vc9")
	  ELSE(MSVC11)
		message( FATAL_ERROR "Unsupported Windows Compiler! Exiting." )
	  ENDIF(MSVC11)
	  
      set( _incsearchpath $ENV{CASROOT}\\inc C:\\OpenCASCADE\\ros\\inc )
      set( _testlibname TKernel.lib )
      set( _libsearchpath $ENV{CASROOT}\\${platform_name}\\lib C:\\OpenCASCADE\\ros\\${_platform_name}\\${_compiler_name}\\lib )
    ELSE(WIN32)
      message( FATAL_ERROR "Unknown system! Exiting." )
    ENDIF(WIN32)
  ENDIF(UNIX)

  #find the include dir by looking for Standard_Real.hxx
  FIND_PATH( OpenCASCADE_INCLUDE_DIR Standard_Real.hxx PATHS ${_incsearchpath} DOC "Path to OpenCASCADE includes" )
  IF( OpenCASCADE_INCLUDE_DIR STREQUAL OpenCASCADE_INCLUDE_DIR-NOTFOUND )
    SET( OpenCASCADE_FOUND FALSE CACHE INTERNAL "OpenCASCADE Libraries Found?" FORCE )
    MESSAGE( FATAL_ERROR "Cannot find OpenCASCADE include directory. Install OpenCASCADE or set CASROOT or create a symlink /opt/occ/inc (for Linux) pointing to the correct directory." )
  ENDIF( OpenCASCADE_INCLUDE_DIR STREQUAL OpenCASCADE_INCLUDE_DIR-NOTFOUND )

  # Find one lib and save its directory to OpenCASCADE_LINK_DIRECTORY. Because
  #  OCC has so many libs, there is increased risk of a name collision.
  #  Requiring that all libs be in the same directory reduces the risk.
  IF(OpenCASCADE_INCLUDE_DIR)
	GET_FILENAME_COMPONENT(_OpenCASCADE_root_dir ${OpenCASCADE_INCLUDE_DIR} PATH)
	IF(UNIX)
		LIST(APPEND _libsearchpath ${_OpenCASCADE_root_dir}/${_platform_name}/lib)
	ELSE(UNIX)
		IF(WIN32)
			LIST(APPEND _libsearchpath ${_OpenCASCADE_root_dir}\\${_platform_name}\\${_compiler_name}\\lib )
		ENDIF(WIN32)
	ENDIF(UNIX)
  ENDIF(OpenCASCADE_INCLUDE_DIR)

  FIND_PATH( OpenCASCADE_LINK_DIRECTORY ${_testlibname} PATH ${_libsearchpath} DOC "Path to OCC libs" )
  IF( OpenCASCADE_LINK_DIRECTORY STREQUAL OpenCASCADE_LINK_DIRECTORY-NOTFOUND )
    SET(OpenCASCADE_FOUND FALSE CACHE INTERNAL "OpenCASCADE Libraries Found?" FORCE)
    MESSAGE( FATAL_ERROR "Cannot find OpenCASCADE lib dir. Install OpenCASCADE or set CASROOT or create a symlink /opt/occ/lib pointing to the dir where the OpenCASCADE libs are." )
  ELSE( OpenCASCADE_LINK_DIRECTORY STREQUAL OpenCASCADE_LINK_DIRECTORY-NOTFOUND )
    SET( OpenCASCADE_FOUND TRUE CACHE INTERNAL "OpenCASCADE Libraries Found?" FORCE )
    SET( _firsttime TRUE ) #so that messages are only printed once
    MESSAGE( STATUS "Found OCC include dir: ${OpenCASCADE_INCLUDE_DIR}" )
    MESSAGE( STATUS "Found OCC lib dir: ${OpenCASCADE_LINK_DIRECTORY}" )
  ENDIF( OpenCASCADE_LINK_DIRECTORY STREQUAL OpenCASCADE_LINK_DIRECTORY-NOTFOUND )
ELSE( NOT OpenCASCADE_FOUND STREQUAL TRUE )
  SET( _firsttime FALSE ) #so that messages are only printed once
ENDIF( NOT OpenCASCADE_FOUND STREQUAL TRUE )

IF( OpenCASCADE_FOUND STREQUAL TRUE )
  IF( DEFINED OpenCASCADE_FIND_COMPONENTS )
    FOREACH( _libname ${OpenCASCADE_FIND_COMPONENTS} )
      #look for libs in OpenCASCADE_LINK_DIRECTORY
      FIND_LIBRARY( OpenCASCADE_${_libname} ${_libname} ${OpenCASCADE_LINK_DIRECTORY} NO_DEFAULT_PATH)
      SET( _foundlib ${OpenCASCADE_${_libname}} )
      IF( _foundlib STREQUAL OpenCASCADE_${_libname}-NOTFOUND )
        MESSAGE( FATAL_ERROR "Cannot find ${_libname}. Is it spelled correctly? Correct capitalization? Do you have another package with similarly-named libraries, installed at ${OpenCASCADE_LINK_DIRECTORY}? (That is where this script thinks the OCC libs are.)" )
      ENDIF( _foundlib STREQUAL OpenCASCADE_${_libname}-NOTFOUND )
      SET( OpenCASCADE_LIBRARIES ${OpenCASCADE_LIBRARIES} ${_foundlib} )
	  MARK_AS_ADVANCED(${_foundlib})
    ENDFOREACH( _libname ${OpenCASCADE_FIND_COMPONENTS} )

    IF (UNIX)
      ADD_DEFINITIONS( -DLIN -DLININTEL )
    ENDIF (UNIX)

    # 32 bit or 64 bit?
    IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
      IF( _firsttime STREQUAL TRUE )
        MESSAGE( STATUS "This is a 32-bit system." )
      ENDIF( _firsttime STREQUAL TRUE )
    ELSE( CMAKE_SIZEOF_VOID_P EQUAL 4 )
      IF( _firsttime STREQUAL TRUE )
        MESSAGE( STATUS "This is a 64-bit system. Adding appropriate compiler flags for OCC." )
      ENDIF( _firsttime STREQUAL TRUE )
      ADD_DEFINITIONS( -D_OCC64 )
      IF (UNIX)
        ADD_DEFINITIONS( -m64 )
      ENDIF (UNIX)
    ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 4 )

    ADD_DEFINITIONS( -DHAVE_CONFIG_H -DHAVE_IOSTREAM -DHAVE_FSTREAM -DHAVE_LIMITS_H -DHAVE_IOMANIP )
  ELSE( DEFINED OpenCASCADE_FIND_COMPONENTS )
    MESSAGE( AUTHOR_WARNING "Developer must specify required libraries to link against in the cmake file, i.e. find_package( OpenCASCADE REQUIRED COMPONENTS TKernel TKBRep) . Otherwise no libs will be added - linking against ALL OCC libraries is slow!")
  ENDIF( DEFINED OpenCASCADE_FIND_COMPONENTS )
ENDIF( OpenCASCADE_FOUND STREQUAL TRUE )
