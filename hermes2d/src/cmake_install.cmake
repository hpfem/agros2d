# Install script for directory: /home/karban/Projects/programovani/c/agros2d/hermes2d/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/libhermes2d-real.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/hermes2d" TYPE FILE FILES
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/norm.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/mesh_parser.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset_l2_all.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/space_l2.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/mesh_lexer.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solution.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/adapt_ortho_h1.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/space_h1.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/precond_ml.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/filter.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/linear.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/exodusii.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/refmap.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset_hc_all.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/space_hcurl.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/matrix.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/feproblem.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solver_umfpack.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/quad.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/quad_all.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/refsystem2.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/precond.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/precond_ifpack.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset_h1_all.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/integrals_hdiv.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/h2d_reader.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/space_hdiv.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/config.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/traverse.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/auto_local_array.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/refsystem.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/weakform.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solver_aztecoo.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solver_nox.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset_h1_quad.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/hash.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/compat.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/transform.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/weakform_lexer.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/linsystem.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/refinement_type.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/adapt_h1.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/integrals_hcurl.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/precalc.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/element_to_refine.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/nonlinsystem.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/common.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/mesh_loader.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/function.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/graph.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/array.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/asmlist.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/hermes2d.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/integrals_h1.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solver_epetra.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/adapt_ortho_hcurl.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset_common.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/mesh.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/itersolver.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/forms.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/curved.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/space.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/adapt_ortho_l2.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solver_pardiso.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/assoc.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/shapeset_hd_all.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/solver.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/hermes2d/compat" TYPE FILE FILES "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/compat/c99_functions.h")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/hermes2d/views" TYPE FILE FILES
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/vector_view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/vector_base_view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/stream_view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/order_view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/base_view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/view_support.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/mesh_view.h"
    "/home/karban/Projects/programovani/c/agros2d/hermes2d/src/views/scalar_view.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

