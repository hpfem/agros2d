# - Try to find OpenGL
# Once done this will define
#
#  OPENGL_FOUND        - system has OpenGL
#  OPENGL_XMESA_FOUND  - system has XMESA
#  OPENGL_GLU_FOUND    - system has GLU
#  OPENGL_INCLUDE_DIR  - the GL include directory
#  OPENGL_LIBRARIES    - Link these to use OpenGL and GLU
#
# If you want to use just GL you can use these values
#  OPENGL_gl_LIBRARY   - Path to OpenGL Library
#  OPENGL_glu_LIBRARY  - Path to GLU Library
#
# On OSX default to using the framework version of opengl
# People will have to change the cache values of OPENGL_glu_LIBRARY
# and OPENGL_gl_LIBRARY to use OpenGL with X11 on OSX

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (WIN32)
  if (WIN64)
      if(DEFINED OPENGL_LIB_DIR)
        find_library(OPENGL_gl_LIBRARY opengl32 ${OPENGL_LIB_DIR})
        find_library(OPENGL_glu_LIBRARY glu32  ${OPENGL_LIB_DIR})
      else()
        set (OPENGL_gl_LIBRARY opengl32 CACHE STRING "OpenGL library for win32")
        set (OPENGL_glu_LIBRARY glu32 CACHE STRING "GLU library for win32")
      endif()
    endif()
endif()