# Set project's name
PROJECT( agros2d )
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Debug yes / no.
SET(AGROS_DEBUG NO)
SET(WITH_QT5 NO)
SET(BUILD_CYTHON YES)

SET(CMAKE_INSTALL_PREFIX "/usr/local")

# Allow to override the default values in CMake.vars:
include(CMake.vars OPTIONAL)

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

# Hermes and Hermes common
IF(AGROS_DEBUG)
    SET(HERMES_COMMON_LIBRARY hermes_common-debug)
    SET(HERMES_LIBRARY hermes2d-debug)
ElSE(AGROS_DEBUG)
    SET(HERMES_COMMON_LIBRARY hermes_common)
    SET(HERMES_LIBRARY hermes2d)
ENDIF(AGROS_DEBUG)


# Add QT.
SET(CMAKE_AUTOMOC TRUE)
IF(WITH_QT5)
    IF(WIN32)
        SET(CMAKE_PREFIX_PATH "C:\\Qt\\32\\Qt5.0.2\\5.0.2\\msvc2010_opengl\\")
    ENDIF(WIN32)
    FIND_PACKAGE(Qt5Widgets)
ELSE(WITH_QT5)
    SET(QT_USE_QTOPENGL TRUE)
    SET(QT_USE_QTUITOOLS TRUE)
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTOPENGL TRUE)
    SET(QT_USE_QTSQL TRUE)
    SET(QT_USE_QTXML TRUE)
    SET(QT_USE_QTSVG TRUE)
    SET(QT_USE_QTTEST TRUE)
    SET(QT_USE_QTDBUS TRUE)
    SET(QT_USE_QTSCRIPT TRUE)
    SET(QT_USE_QTWEBKIT TRUE)
    SET(QT_USE_QTXMLPATTERNS TRUE)
    FIND_PACKAGE(Qt4 REQUIRED)
    # This has to be here, otherwise CMake will not do the magic and will not link to OpenGL
    FIND_PACKAGE(OpenGL REQUIRED)
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
ENDIF(WITH_QT5)

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
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /openmp /Zc:wchar_t")
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt")
ENDIF(MSVC)

# Python
FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIR})

# Include current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable debugging symbols for all files all the time:
# set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
IF(AGROS_DEBUG)
    SET(CMAKE_BUILD_TYPE Debug)
    IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Disable all warnings and turn on only important ones:
        SET(CMAKE_CXX_FLAGS "-w ${CMAKE_CXX_FLAGS} -fPIC")
        SET(CMAKE_CXX_FLAGS "-Wuninitialized -Wvla -Wsign-compare ${CMAKE_CXX_FLAGS}")

        SET(DEBUG_FLAGS   "-g")
        SET(FLAGS ${DEBUG_FLAGS})
    ENDIF()
ElSE(AGROS_DEBUG)
    SET(CMAKE_BUILD_TYPE Release)
    SET(RELEASE_FLAGS "-DNDEBUG -O3")
    SET(FLAGS ${RELEASE_FLAGS})
ENDIF(AGROS_DEBUG)

# Create file with preprocessor definitions exposing the build settings to the source code.
configure_file(
  util/config.h.cmake
  util/config.h
)

# Include OUR header files location
include(IncludeSubdirs.cmake)

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

# Installation
SET(BINARY_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/bin)
SET(LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib)
SET(SHARE_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/share/agros2d)

# libs
FILE(GLOB LIB_FILES "libs/*.so")
# pixmap
FILE(GLOB PIXMAP_FILES "resources/images/agros2d.xpm")
# desktop
FILE(GLOB DESKTOP_FILES "agros2d.desktop")
# python functions
FILE(GLOB PYTHON_FUNCTION_FILES "*.py")
# resources
SET(RESOURCES_FILES "resources")

IF(WIN32)
  FILE(GLOB BINARY_AGROS2D "agros2d_binary.exe")
  #FILE(RENAME "agros2d_binary.exe" "agros2d.exe")
ELSE(WIN32)
  FILE(GLOB BINARY_AGROS2D "agros2d_binary")
  #FILE(RENAME "agros2d_binary" "agros2d")
ENDIF(WIN32)

IF(WIN32)
  FILE(GLOB BINARY_SOLVER "agros2d_solver.exe")
ELSE(WIN32)
  FILE(GLOB BINARY_SOLVER "agros2d_solver")
ENDIF(WIN32)

IF(WIN32)
  FILE(GLOB BINARY_PYTHONLAB "pythonlab_binary.exe")
  #FILE(RENAME "pythonlab_binary.exe" "pythonlab.exe")
ELSE(WIN32)
  FILE(GLOB BINARY_PYTHONLAB "pythonlab_binary")
  #FILE(RENAME "pythonlab_binary" "pythonlab")
ENDIF(WIN32)

# bin
INSTALL(PROGRAMS ${BINARY_AGROS2D} DESTINATION ${BINARY_INSTALL_PATH} RENAME agros2d)
INSTALL(PROGRAMS ${BINARY_SOLVER} DESTINATION ${BINARY_INSTALL_PATH})
INSTALL(PROGRAMS ${BINARY_PYTHONLAB} DESTINATION ${BINARY_INSTALL_PATH} RENAME pythonlab)
# lib
INSTALL(FILES ${LIB_FILES} DESTINATION ${LIB_INSTALL_PATH})
# share
INSTALL(FILES ${PIXMAP_FILES} DESTINATION ${SHARE_INSTALL_PATH}/pixmaps)
INSTALL(FILES ${DESKTOP_FILES} DESTINATION ${SHARE_INSTALL_PATH}/applications)
INSTALL(FILES ${PYTHON_FUNCTION_FILES} DESTINATION ${SHARE_INSTALL_PATH})
INSTALL(DIRECTORY ${RESOURCES_FILES} DESTINATION ${SHARE_INSTALL_PATH})
