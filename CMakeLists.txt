# Set project's name
PROJECT( agros2d )
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Debug yes / no.
SET(AGROS_DEBUG NO)
SET(WITH_QT5 NO)

# Allow to override the default values in CMake.vars:
INCLUDE(CMake.vars OPTIONAL)

# versions
# SET(VERSION_BETA)
SET(VERSION_MAJOR 3)
SET(VERSION_MINOR 0)
SET(VERSION_SUB 0)
SET(VERSION_GIT 4214)
SET(VERSION_YEAR 2013)
SET(VERSION_MONTH 7)
SET(VERSION_DAY 13)

# Naming of projects built.
SET(AGROS_UTIL agros2d_util)
SET(PYTHONLAB_LIBRARY agros2d_pythonlab_library)
SET(PYTHONLAB_BINARY pythonlab_binary)
SET(AGROS_LIBRARY agros2d_library)
SET(AGROS_SOLVER agros2d_solver)
SET(AGROS_GENERATOR agros2d_generator)
SET(AGROS_BINARY agros2d_binary)
SET(PARALUTION_LIBRARY agros2d_3dparty_paralution)
SET(BSON_LIBRARY agros2d_3dparty_bson)
SET(CTEMPLATE_LIBRARY agros2d_3dparty_ctemplate)
SET(DXFLIB_LIBRARY agros2d_3dparty_dxflib)
SET(POLY2TRI_LIBRARY agros2d_3dparty_poly2tri)
SET(QCUSTOMPLOT_LIBRARY agros2d_3dparty_qcustomplot)
SET(QUAZIP_LIBRARY agros2d_3dparty_quazip)
SET(STB_TRUETYPE_LIBRARY agros2d_3dparty_stb_truetype)
SET(QTSINGLEAPPLICATION_LIBRARY agros2d_3dparty_qtsingleapplication)
SET(MATIO_LIBRARY agros2d_3dparty_matio)

# Hermes and Hermes common
IF(AGROS_DEBUG)
    SET(HERMES_COMMON_LIBRARY hermes_common-debug)
    SET(HERMES_LIBRARY hermes2d-debug)
ElSE(AGROS_DEBUG)
    SET(HERMES_COMMON_LIBRARY hermes_common)
    SET(HERMES_LIBRARY hermes2d)
ENDIF(AGROS_DEBUG)

# Output paths.
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/libs)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libs)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libs)
  
IF(WIN32)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_HOME_DIRECTORY})
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_HOME_DIRECTORY})
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_HOME_DIRECTORY}/libs)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_HOME_DIRECTORY}/libs)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_HOME_DIRECTORY}/libs)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_HOME_DIRECTORY}/libs)
ENDIF(WIN32)

# Add QT.
INCLUDE(CMakeQt.cmake)

# Set global compiler parameters.
find_package(OpenMP REQUIRED)
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  INCLUDE_DIRECTORIES(/usr/include/google)
  IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  INCLUDE_DIRECTORIES(omp)
  ENDIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
ENDIF()

IF(MSVC)
  INCLUDE_DIRECTORIES(c:/hpfem/hermes/dependencies/include)
  INCLUDE_DIRECTORIES(d:/hpfem/hermes/dependencies/include)
  INCLUDE_DIRECTORIES(c:/Python27/include)
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt")
ENDIF(MSVC)

# Python
FIND_PACKAGE(PythonLibs 2.7 REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIR})

# Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable debugging symbols for all files all the time:
IF(AGROS_DEBUG)
    IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Disable all warnings and turn on only important ones:
        # SET(CMAKE_CXX_FLAGS "-w ${CMAKE_CXX_FLAGS} -fPIC")
        # SET(CMAKE_CXX_FLAGS "-Wuninitialized -Wvla -Wsign-compare ${CMAKE_CXX_FLAGS}")
        SET(CMAKE_BUILD_TYPE Debug)
        SET(DEBUG_FLAGS   "-g")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")
    ENDIF()
ElSE(AGROS_DEBUG)
    IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      SET(CMAKE_BUILD_TYPE Release)
      SET(RELEASE_FLAGS "-DNDEBUG -O3 -Ofast")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
    ENDIF()
ENDIF(AGROS_DEBUG)

# This overrides CXX flags for MSVC
IF(MSVC)
  SET(MSVC_DEFINES "/DWIN32 /D_WINDOWS /Dpopen=_popen /Dpclose=_pclose /D__value=_value /Dfinite=_finite /Dhypot=_hypot /Disatty=_isatty /Dfileno=_fileno /D_CRT_SECURE_NO_WARNINGS /MP")
  SET(CMAKE_CXX_FLAGS_DEBUG_INIT          "/D_DEBUG /Od /Ob2 /MDd /Zi ${MSVC_DEFINES}")
  SET(CMAKE_CXX_FLAGS_RELEASE_INIT        "/DNDEBUG /O2 /Ob2 /MD ${MSVC_DEFINES}")
  SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
  SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
  SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
ENDIF(MSVC)
  
# Create file with preprocessor definitions exposing the build settings to the source code.
configure_file(
  util/config.h.cmake
  util/config.h
)

# Include OUR header files location
INCLUDE(IncludeSubdirs.cmake)

# Subdirectories.
ADD_SUBDIRECTORY(3rdparty)
# Also looks for Xerces, UMFPACK, etc.
ADD_SUBDIRECTORY(hermes)
# Include those
#INCLUDE_DIRECTORIES(${XSD_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(${XERCES_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(${TCMALLOC_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(${BSON_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${UMFPACK_INCLUDE_DIR})


ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(pythonlab-library)
ADD_SUBDIRECTORY(pythonlab-binary)
ADD_SUBDIRECTORY(agros2d-library)
ADD_SUBDIRECTORY(agros2d-binary)
ADD_SUBDIRECTORY(agros2d-solver)
ADD_SUBDIRECTORY(agros2d-generator)
#ADD_SUBDIRECTORY(agros2d-remote)
#ADD_SUBDIRECTORY(agros2d-forms)

# TODO - better!
message("\nConfiguration results - Agros2D")
message("---------------------")
IF(WITH_QT5)
    message("Build with QT5")
ELSE(WITH_QT5)
    message("Build with QT4")
ENDIF(WITH_QT5)
IF(AGROS_DEBUG)
  message("Build version: Debug")
ELSE(AGROS_DEBUG)
    message("Build version: Release")
ENDIF(AGROS_DEBUG)
message("\n")

# pixmap
FILE(GLOB PIXMAP_FILES "resources/images/agros2d.xpm")
# desktop
FILE(GLOB DESKTOP_FILES "agros2d.desktop")
# resources
SET(RESOURCES_FILES "resources")

# share
INSTALL(FILES ${PIXMAP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps)
INSTALL(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
INSTALL(DIRECTORY ${RESOURCES_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/agros2d)

# other files
MACRO(ADD_INFO_FILES)
     FOREACH(_it ${ARGN})
         IF(NOT IS_DIRECTORY ${_it})
             GET_FILENAME_COMPONENT(name ${_it} NAME)
             if(NOT ${_it} MATCHES "^/\\\\..*$;~$")
                 SET_SOURCE_FILES_PROPERTIES(${_it} PROPERTIES HEADER_FILE_ONLY TRUE)
             ENDIF()
         ENDIF()
     ENDFOREACH()
ENDMACRO()

set(info resources/modules/acoustic.xml
         resources/modules/current.xml
         resources/modules/elasticity.xml
         resources/modules/electrostatic.xml
         resources/modules/flow.xml
         resources/modules/heat.xml
         resources/modules/magnetic.xml
         resources/modules/rf_te.xml
         resources/modules/rf_tm.xml
         resources/couplings/current-heat.xml
         resources/couplings/heat-elasticity.xml
         resources/couplings/magnetic-heat.xml
         resources/python/functions_agros2d.py
         resources/python/functions_pythonlab.py
         resources_source/python/agros2d.pyx
         resources_source/python/pyfield.pxi
         resources_source/python/pygeometry.pxi
         resources_source/python/pyparticletracing.pxi
         resources_source/python/pyproblem.pxi
         resources_source/python/pythonlab.pyx
         resources_source/python/pyview.pxi
)
add_info_files(${info})
add_custom_target(other SOURCES ${info})
