set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)
project(hermes2d)

cmake_minimum_required(VERSION 2.6)

#---------------------------------------------------------
# begin of configuration section
# The values below are sane defaults to produce a build on all platforms,
# if you want to override them, set them in CMake.vars

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# build with debugging symbols
set(DEBUG NO)
set(RELEASE YES)

# real/complex version of the library (you can build either of both)
set(REAL     YES)
set(COMPLEX  YES)

# optional functionality
set(WITH_PYTHON     YES)
set(WITH_EXAMPLES   YES)
set(WITH_BENCHMARKS YES)
set(WITH_TUTORIAL   YES)
set(WITH_TESTS      YES)
set(WITH_GLUT       YES)
set(WITH_VIEWER_GUI NO)
set(WITH_UTIL       YES)
set(WITH_TRILINOS NO)
set(WITH_EXODUSII   NO)

# allow to override the default values in CMake.vars
if(EXISTS ${PROJECT_SOURCE_DIR}/CMake.vars)
	include(CMake.vars)
endif(EXISTS ${PROJECT_SOURCE_DIR}/CMake.vars)

# end of configuration section
#---------------------------------------------------------

if(WITH_TRILINOS)
  enable_language(Fortran)
  find_package(LAPACK REQUIRED)     # trilinos needs LAPACK
  find_package(TRILINOS REQUIRED)
endif(WITH_TRILINOS)

if(REAL)
    list(APPEND VERS "real")
endif(REAL)
if(COMPLEX)
    list(APPEND VERS "complex")
endif(COMPLEX)

if(MSVC)
    if(REAL)
        set(HERMES_REAL_BIN hermes2d-real)
    endif(REAL)
    if(COMPLEX)
        set(HERMES_CPLX_BIN hermes2d-cplx)
    endif(COMPLEX)
else(MSVC)
	if(DEBUG)
		if(REAL)
			set(HERMES_REAL_BIN hermes2d-real-debug)
		endif(REAL)
		if(COMPLEX)
			set(HERMES_CPLX_BIN hermes2d-cplx-debug)
		endif(COMPLEX)
	endif(DEBUG)

	if(RELEASE)
		if(REAL)
			set(HERMES_REAL_BIN hermes2d-real)
		endif(REAL)
		if(COMPLEX)
			set(HERMES_CPLX_BIN hermes2d-cplx)
		endif(COMPLEX)
	endif(RELEASE)
endif(MSVC)

if(NOT WITH_GLUT)
    add_definitions(-DNOGLUT)
endif(NOT WITH_GLUT)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(WITH_EXODUSII)
    find_package(EXODUSII REQUIRED)
endif(WITH_EXODUSII)

add_subdirectory(src)

if(WITH_EXAMPLES)
	add_subdirectory(examples)
endif(WITH_EXAMPLES)

if(WITH_BENCHMARKS)
	add_subdirectory(benchmarks)
endif(WITH_BENCHMARKS)

if(WITH_TUTORIAL)
	add_subdirectory(tutorial)
endif(WITH_TUTORIAL)

if(WITH_UTIL)
	add_subdirectory(util)
endif(WITH_UTIL)

if(WITH_TESTS)
    enable_testing()
	add_subdirectory(tests)
endif(WITH_TESTS)

if(WITH_PYTHON)
	add_subdirectory(python)
endif(WITH_PYTHON)

add_custom_target(test
    COMMAND ctest
    )
add_custom_target(test-quick
    COMMAND ctest -LE slow -j9
    )

# Documentation
# This doesn't work yet:
#add_subdirectory(doc)
#add_custom_target(doc)
#add_custom_command(
#	SOURCE	${DOXYGEN_CONFIG_FILE}
#	COMMAND	${DOXYGEN_BINARY}
#	ARGS	${DOXYGEN_CONFIG_FILE}
#	TARGET	doc
#	OUTPUTS	${PROJECT_BINARY_DIR}/doc/html
#)

#add_custom_command(
#	SOURCE	doc
#	TARGET	doc
#	DEPENDS	${PROJECT_BINARY_DIR}/doc/html
#)


message("\nConfiguration results (these options can be set on the commandline)")
message("---------------------")
message("Versions to build: ${VERS}")
message("Build with Python: ${WITH_PYTHON}")
message("Build with examples: ${WITH_EXAMPLES}")
message("Build with benchmarks: ${WITH_BENCHMARKS}")
message("Build with tutorial: ${WITH_TUTORIAL}")
message("Build with GLUT: ${WITH_GLUT}")
message("Build with viewer GUI: ${WITH_VIEWER_GUI}")
message("Build with util: ${WITH_UTIL}")
message("Build with tests: ${WITH_TESTS}")
message("Build with TRILINOS: ${WITH_TRILINOS}")
message("\n")
