# Gmsh - Copyright (C) 1997-2014 C. Geuzaine, J.-F. Remacle
#
# See the LICENSE.txt file for license information. Please report all
# bugs and problems to the public mailing list <gmsh@geuz.org>.

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# do not warn about non-definition of WIN32 on cygwin
set(CMAKE_LEGACY_CYGWIN_WIN32 0) 

# if CMAKE_BUILD_TYPE is specified use it; otherwise set the default
# build type to "RelWithDebInfo" ("-O2 -g" with gcc) prior to calling
# project()
set(CMAKE_BUILD_TYPE Release)
if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose build type")
else(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose build type")
endif(DEFINED CMAKE_BUILD_TYPE)

project(${GMSH_LIBRARY} CXX C)

# this variable controls the default value of the options which are normally set
# to ON (useful if you want to configure a minimal version of Gmsh: e.g. "cmake
# -DDEFAULT=0 -DENABLE_POST=1 -DENABLE_PARSER=1")
set(DEFAULT OFF CACHE INTERNAL "Default value for enabled-by-default options")

macro(opt OPTION HELP VALUE)
  option(ENABLE_${OPTION} ${HELP} ${VALUE})
  set(OPT_TEXI "${OPT_TEXI}\n@item ENABLE_${OPTION}\n${HELP} (default: ${VALUE})")
endmacro(opt)

opt(3M "Enable proprietary 3M extension" OFF)
opt(ACIS "Enable ACIS geometrical models (experimental)" ${DEFAULT})
opt(ANN "Enable ANN (used for fast point search in mesh/post)" ${DEFAULT})
opt(BAMG "Enable Bamg 2D anisotropic mesh generator" ${DEFAULT})
opt(BFGS "Enable BFGS (used by some mesh optimizers)" ${DEFAULT})
opt(BLAS_LAPACK "Enable BLAS/Lapack for linear algebra (required for meshing)" ON)
opt(BLOSSOM "Enable Blossom algorithm (needed for full quad meshing)" ON)
opt(BUILD_LIB "Enable 'lib' target for building static Gmsh library" OFF)
opt(BUILD_SHARED "Enable 'shared' target for building shared Gmsh library" ON)
opt(BUILD_DYNAMIC "Enable dynamic Gmsh executable (linked with shared lib)" OFF)
opt(BUILD_ANDROID "Enable Android NDK library target (experimental)" OFF)
opt(BUILD_IOS "Enable iOS (ARM) library target (experimental)" OFF)
opt(CGNS "Enable CGNS mesh export (experimental)" OFF)
opt(CAIRO "Enable Cairo to render fonts (experimental)" ${DEFAULT})
opt(CHACO "Enable Chaco mesh partitioner (alternative to Metis)" ${DEFAULT})
opt(DINTEGRATION "Enable discrete integration (needed for levelsets)" ${DEFAULT})
opt(FLTK "Enable FLTK graphical user interface (requires mesh/post)" ${DEFAULT})
opt(FOURIER_MODEL "Enable Fourier geometrical models (experimental)" OFF)
opt(GMM "Enable GMM linear solvers (simple alternative to PETSc)" ${DEFAULT})
opt(GRAPHICS "Enable building graphics lib even without GUI (advanced)" OFF)
opt(KBIPACK "Enable Kbipack (neeeded by homology solver)" ${DEFAULT})
opt(MATHEX "Enable math expression parser (used by plugins and options)" ${DEFAULT})
opt(MED "Enable MED mesh and post file formats" ${DEFAULT})
opt(MESH "Enable mesh module (required by GUI)" ON)
opt(METIS "Enable Metis mesh partitioner" ${DEFAULT})
opt(MMG3D "Enable MMG3D 3D anisotropic mesh refinement" ${DEFAULT})
opt(MPEG_ENCODE "Enable built-in MPEG movie encoder" ${DEFAULT})
opt(MPI "Enable MPI (mostly for parser and solver - mesh generation is sequential)" OFF)
opt(MSVC_STATIC_RUNTIME "Enable static Visual C++ runtime" OFF)
opt(MUMPS "Enable MUMPS sparse direct linear solver" OFF)
opt(NATIVE_FILE_CHOOSER "Enable native file chooser in GUI" ${DEFAULT})
opt(NETGEN "Enable Netgen 3D frontal mesh generator" ${DEFAULT})
opt(NUMPY "Enable conversion between fullMatrix and numpy array (requires SWIG)" OFF)
opt(OCC "Enable Open CASCADE geometrical models" ${DEFAULT})
opt(ONELAB "Enable OneLab solver interface" ${DEFAULT})
opt(ONELAB_METAMODEL "Enable OneLab metamodels (experimental)" ${DEFAULT})
opt(OPENMP "Enable OpenMP (experimental)" OFF)
opt(OPTHOM "Enable high-order mesh optimization tools" ${DEFAULT})
opt(OS_SPECIFIC_INSTALL "Enable OS-specific (e.g. app bundle) installation" ${DEFAULT})
opt(OSMESA "Enable OSMesa for offscreen rendering (experimental)" OFF)
opt(PARSER "Enable GEO file parser (required for .geo/.pos files)" ON)
opt(PETSC "Enable PETSc linear solvers (required for SLEPc)" ${DEFAULT})
opt(PLUGINS "Enable post-processing plugins" ${DEFAULT})
opt(POST "Enable post-processing module (required by GUI)" ${DEFAULT})
opt(POPPLER "Enable Poppler for displaying PDF documents (experimental)" OFF)
opt(QT "Enable dummy QT graphical interface proof-of-concept (experimental)" OFF)
opt(RTREE "Enable RTREE (used for quad/hex mesh generation)" ${DEFAULT})
opt(SALOME "Enable Salome routines for CAD healing" ${DEFAULT})
opt(SGEOM "Enable SGEOM interface to OCC (experimental)" OFF)
opt(SLEPC "Enable SLEPc eigensolvers (required for conformal compounds)" ${DEFAULT})
opt(SOLVER "Enable built-in finite element solvers (required for compounds)" ${DEFAULT})
opt(TAUCS "Enable Taucs linear solver (alternative to PETSc)" ${DEFAULT})
opt(TETGEN "Enable Tetgen 3D initial mesh generator" ${DEFAULT})
opt(TETGEN_OLD "Enable older version of Tetgen" OFF)
opt(VORO3D "Enable Voro3D (for hex meshing, experimental)" ${DEFAULT})
opt(WRAP_JAVA "Enable generation of Java wrappers (experimental)" OFF)
opt(WRAP_PYTHON "Enable generation of Python wrappers" OFF)

set(GMSH_MAJOR_VERSION 2)
set(GMSH_MINOR_VERSION 8)
set(GMSH_PATCH_VERSION 5)
set(GMSH_EXTRA_VERSION "" CACHE STRING "Gmsh extra version string")

set(GMSH_VERSION "${GMSH_MAJOR_VERSION}.${GMSH_MINOR_VERSION}")
set(GMSH_VERSION "${GMSH_VERSION}.${GMSH_PATCH_VERSION}${GMSH_EXTRA_VERSION}")
set(GMSH_SHORT_LICENSE "GNU General Public License")

set(GMSH_API
  ${CMAKE_CURRENT_BINARY_DIR}/Common/GmshConfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/Common/GmshVersion.h
  Common/Gmsh.h Common/Context.h Common/GmshDefines.h Common/GmshMessage.h
    Common/VertexArray.h Common/Octree.h Common/OctreeInternals.h
    Common/OS.h Common/StringUtils.h Common/OpenFile.h
    Common/onelab.h Common/GmshSocket.h Common/onelabUtils.h
  Numeric/Numeric.h Numeric/GaussIntegration.h Numeric/polynomialBasis.h
    Numeric/JacobianBasis.h Numeric/MetricBasis.h Numeric/bezierBasis.h Numeric/fullMatrix.h
    Numeric/simpleFunction.h Numeric/cartesian.h Numeric/ElementType.h
  Geo/GModel.h Geo/GEntity.h Geo/GPoint.h Geo/GVertex.h Geo/GEdge.h
    Geo/GFace.h Geo/GRegion.h Geo/GEdgeLoop.h Geo/GEdgeCompound.h
    Geo/GFaceCompound.h Geo/GRegionCompound.h Geo/GRbf.h Geo/MVertex.h
    Geo/MVertexBoundaryLayerData.h Geo/MEdge.h
    Geo/MFace.h Geo/MElement.h Geo/MElementOctree.h Geo/MPoint.h Geo/MLine.h
    Geo/MTriangle.h Geo/MQuadrangle.h Geo/MTetrahedron.h Geo/MHexahedron.h
    Geo/MPrism.h Geo/MPyramid.h Geo/MElementCut.h Geo/MElementOctree.h
    Geo/discreteVertex.h Geo/discreteEdge.h Geo/discreteFace.h Geo/discreteRegion.h
    Geo/SPoint2.h Geo/SPoint3.h Geo/SVector3.h Geo/STensor3.h Geo/SBoundingBox3d.h
    Geo/Pair.h Geo/Range.h Geo/SOrientedBoundingBox.h
    Geo/CellComplex.h Geo/ChainComplex.h Geo/Cell.h Geo/Homology.h Geo/Chain.h
    Geo/partitionEdge.h Geo/CGNSOptions.h Geo/gmshLevelset.h Geo/boundaryLayersData.h
  Mesh/meshGEdge.h Mesh/meshGFace.h Mesh/meshGFaceOptimize.h
    Mesh/meshGFaceElliptic.h Mesh/meshPartition.h Mesh/meshGFaceDelaunayInsertion.h
    Mesh/simple3D.h Mesh/meshPartitionOptions.h Mesh/directions3D.h Mesh/yamakawa.h
    Mesh/Voronoi3D.h Mesh/Levy3D.h Mesh/periodical.h Mesh/meshMetric.h
  Numeric/mathEvaluator.h
  Solver/dofManager.h Solver/femTerm.h Solver/laplaceTerm.h Solver/elasticityTerm.h
    Solver/crossConfTerm.h Solver/orthogonalTerm.h
    Solver/linearSystem.h Solver/linearSystemGMM.h Solver/linearSystemCSR.h
    Solver/linearSystemFull.h Solver/elasticitySolver.h Solver/sparsityPattern.h
    Solver/groupOfElements.h Solver/linearSystemPETSc.h Solver/linearSystemMUMPS.h
  Post/PView.h Post/PViewData.h Plugin/PluginManager.h Post/OctreePost.h
  Post/PViewDataList.h Post/PViewDataGModel.h Post/PViewOptions.h Post/ColorTable.h
   Numeric/nodalBasis.h
  Graphics/drawContext.h
  contrib/kbipack/gmp_normal_form.h contrib/kbipack/gmp_matrix.h
    contrib/kbipack/gmp_blas.h contrib/kbipack/mpz.h
  contrib/DiscreteIntegration/Integration3D.h
  contrib/HighOrderMeshOptimizer/OptHOM.h contrib/HighOrderMeshOptimizer/OptHomMesh.h
  contrib/HighOrderMeshOptimizer/OptHomRun.h contrib/HighOrderMeshOptimizer/ParamCoord.h
  contrib/HighOrderMeshOptimizer/OptHomFastCurving.h contrib/HighOrderMeshOptimizer/SuperEl.h
  contrib/MathEx/mathex.h)

get_property(IAMCHILD DIRECTORY  PROPERTY PARENT_DIRECTORY)
if(IAMCHILD)
  set(GMSH_API ${GMSH_API} PARENT_SCOPE)
endif(IAMCHILD)

execute_process(COMMAND date "+%Y%m%d" OUTPUT_VARIABLE DATE 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND whoami OUTPUT_VARIABLE PACKAGER 
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT DATE)
  set(DATE "unknown")
endif(NOT DATE)
set(GMSH_DATE "${DATE}")

if(NOT HOSTNAME)
  set(HOSTNAME "unknown")
endif(NOT HOSTNAME)
set(GMSH_HOST "${HOSTNAME}")

if(NOT PACKAGER)
  set(PACKAGER "unknown")
endif(NOT PACKAGER)
set(GMSH_PACKAGER "${PACKAGER}")

if(APPLE)
  set(GMSH_OS "MacOSX")
elseif(CYGWIN)
  set(GMSH_OS "Windows")
else(APPLE)
  set(GMSH_OS "${CMAKE_SYSTEM_NAME}")
endif(APPLE)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCXXCompilerFlag)

if(MSVC)
  # remove annoying warning about bool/int cast performance
  set(GMSH_CONFIG_PRAGMAS "#pragma warning(disable:4800 4244 4267)")
  if(ENABLE_MSVC_STATIC_RUNTIME)
    foreach(VAR
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
      if(${VAR} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${VAR} "${${VAR}}")
      endif(${VAR} MATCHES "/MD")
    endforeach(VAR)
  endif(ENABLE_MSVC_STATIC_RUNTIME)
endif(MSVC)

if(ENABLE_OPENMP)
  find_package(OpenMP)
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif(OPENMP_FOUND)
endif(ENABLE_OPENMP)

macro(append_gmsh_src DIRNAME FILES)
  foreach(FILE ${FILES})
    list(APPEND LIST ${DIRNAME}/${FILE})
  endforeach(FILE)
  set(GMSH_SRC ${GMSH_SRC};${LIST} PARENT_SCOPE)
  set(GMSH_DIRS ${GMSH_DIRS};${DIRNAME} PARENT_SCOPE)
endmacro(append_gmsh_src)

macro(find_all_libraries VARNAME LISTNAME PATH SUFFIX)
  set(${VARNAME})
  list(LENGTH ${LISTNAME} NUM_LIST)
  foreach(LIB ${${LISTNAME}})
    find_library(FOUND_LIB ${LIB} PATHS ${PATH} PATH_SUFFIXES ${SUFFIX})
    if(FOUND_LIB)
      list(APPEND ${VARNAME} ${FOUND_LIB})
    endif(FOUND_LIB)
    unset(FOUND_LIB CACHE)
  endforeach(LIB)
  list(LENGTH ${VARNAME} NUM_FOUND_LIBRARIES)
  if(NUM_FOUND_LIBRARIES LESS NUM_LIST)
    set(${VARNAME})
  endif(NUM_FOUND_LIBRARIES LESS NUM_LIST)
endmacro(find_all_libraries)

macro(set_config_option VARNAME STRING)
  set(${VARNAME} TRUE)
  list(APPEND CONFIG_OPTIONS ${STRING})
  message(STATUS "Found " ${STRING})
endmacro(set_config_option)

# check if the machine is 64 bits (this is more reliable than using
# CMAKE_SIZEOF_VOID_P, which does not seem to work e.g. on some Suse
# machines)
check_type_size("void*" SIZEOF_VOID_P)
if(SIZEOF_VOID_P EQUAL 8)
  set_config_option(HAVE_64BIT_SIZE_T "64Bit")
endif(SIZEOF_VOID_P EQUAL 8)

if(ENABLE_BLAS_LAPACK)
  if(BLAS_LAPACK_LIBRARIES)
    # use libs as specified in the BLAS_LAPACK_LIBRARIES variable
    set_config_option(HAVE_BLAS "Blas(Custom)")
    set_config_option(HAVE_LAPACK "Lapack(Custom)")
    set(LAPACK_LIBRARIES ${BLAS_LAPACK_LIBRARIES})
  else(BLAS_LAPACK_LIBRARIES)
    if(MSVC)
      # on Windows with Visual C++ try really hard to find blas/lapack *without*
      # requiring a Fortran compiler: 1) try to find the Intel MKL libs using
      # the standard search path; if not found 2) try to get the reference
      # blas/lapack libs (useful for users with no Fortran compiler and no MKL
      # license, who can just download our precompiled "gmsh-dep" package)
      if(HAVE_64BIT_SIZE_T)
        set(MKL_PATH em64t/lib)
        set(MKL_LIBS_REQUIRED libguide40 mkl_intel_lp64 mkl_intel_thread mkl_core)
      else(HAVE_64BIT_SIZE_T)
        set(MKL_PATH ia32/lib)
        set(MKL_LIBS_REQUIRED libguide40 mkl_intel_c mkl_intel_thread mkl_core)
      endif(HAVE_64BIT_SIZE_T)
      find_all_libraries(LAPACK_LIBRARIES MKL_LIBS_REQUIRED "" ${MKL_PATH})
      if(LAPACK_LIBRARIES)
        set_config_option(HAVE_BLAS "Blas(IntelMKL)")
        set_config_option(HAVE_LAPACK "Lapack(IntelMKL)")
      else(LAPACK_LIBRARIES)
        set(REFLAPACK_LIBS_REQUIRED lapack blas g2c gcc)
        find_all_libraries(LAPACK_LIBRARIES REFLAPACK_LIBS_REQUIRED "" "")
        if(LAPACK_LIBRARIES)
          set_config_option(HAVE_BLAS "Blas(Reference)")
          set_config_option(HAVE_LAPACK "Lapack(Reference)")
        endif(LAPACK_LIBRARIES)
      endif(LAPACK_LIBRARIES)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      # on Linux try to find the Intel MKL without a Fortran compiler
      if(HAVE_64BIT_SIZE_T)
        set(MKL_PATH lib/em64t)
      else(HAVE_64BIT_SIZE_T)
        set(MKL_PATH lib/32)
      endif(HAVE_64BIT_SIZE_T)
      set(MKL_LIBS_REQUIRED mkl_gf_lp64 iomp5 mkl_gnu_thread mkl_core guide pthread)
      find_all_libraries(LAPACK_LIBRARIES MKL_LIBS_REQUIRED "" ${MKL_PATH})
      if(NOT LAPACK_LIBRARIES)
        # match lapack 9.0 on 64bit
        set(MKL_LIBS_REQUIRED mkl_lapack mkl_em64t guide)
        find_all_libraries(LAPACK_LIBRARIES MKL_LIBS_REQUIRED "" ${MKL_PATH})
      endif(NOT LAPACK_LIBRARIES)
      if(LAPACK_LIBRARIES)
        set_config_option(HAVE_BLAS "Blas(IntelMKL)")
        set_config_option(HAVE_LAPACK "Lapack(IntelMKL)")
      else(LAPACK_LIBRARIES)
        # on Linux also try to find ATLAS without a Fortran compiler, because
        # cmake ships with a buggy FindBLAS e.g. on Ubuntu Lucid Lynx
        set(ATLAS_LIBS_REQUIRED lapack f77blas cblas atlas)
        find_all_libraries(LAPACK_LIBRARIES ATLAS_LIBS_REQUIRED "" "")
        if(LAPACK_LIBRARIES)
          set_config_option(HAVE_BLAS "Blas(ATLAS)")
          set_config_option(HAVE_LAPACK "Lapack(ATLAS)")
        else(LAPACK_LIBRARIES)
          # try with generic names
          set(GENERIC_LIBS_REQUIRED lapack blas pthread)
          find_all_libraries(LAPACK_LIBRARIES GENERIC_LIBS_REQUIRED "" "")
          if(LAPACK_LIBRARIES)
            set_config_option(HAVE_BLAS "Blas(Generic)")
            set_config_option(HAVE_LAPACK "Lapack(Generic)")
            find_library(GFORTRAN_LIB gfortran)
            if(GFORTRAN_LIB)
              list(APPEND LAPACK_LIBRARIES ${GFORTRAN_LIB})
            endif(GFORTRAN_LIB)
          endif(LAPACK_LIBRARIES)
        endif(LAPACK_LIBRARIES)
      endif(LAPACK_LIBRARIES)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
      # on SunOS we know blas and lapack are available in sunperf
      set(LAPACK_FLAGS -library=sunperf)
      set_config_option(HAVE_BLAS "Blas(SunPerf)")
      set_config_option(HAVE_LAPACK "Lapack(SunPerf)")
    elseif(APPLE)
      # on Mac we also know that blas and lapack are available
      set(LAPACK_LIBRARIES "-llapack -lblas")
      set_config_option(HAVE_BLAS "Blas(VecLib)")
      set_config_option(HAVE_LAPACK "Lapack(VecLib)")
    endif(MSVC)

    if(ENABLE_BUILD_ANDROID)
      find_library(BLAS_LIB f2cblas PATH_SUFFIXES lib)
      find_library(LAPACK_LIB f2clapack PATH_SUFFIXES lib)
      if(BLAS_LIB)
        list(APPEND EXTERNAL_LIBRARIES ${BLAS_LIB})
        set_config_option(HAVE_BLAS "Blas")
      endif(BLAS_LIB)
      if(LAPACK_LIB)
        list(APPEND EXTERNAL_LIBRARIES ${LAPACK_LIB})
        set_config_option(HAVE_LAPACK "Lapack")
      endif(LAPACK_LIB)
    endif(ENABLE_BUILD_ANDROID)

    if(NOT HAVE_BLAS OR NOT HAVE_LAPACK)
      # if we haven't found blas and lapack check for OpenBlas
      set(OPENBLAS_LIBS_REQUIRED openblas)
      find_all_libraries(LAPACK_LIBRARIES OPENBLAS_LIBS_REQUIRED "" "")
      if(LAPACK_LIBRARIES)
        set_config_option(HAVE_BLAS "Blas(OpenBlas)")
        set_config_option(HAVE_LAPACK "Lapack(OpenBlas)")
        find_library(GFORTRAN_LIB gfortran)
        if(GFORTRAN_LIB)
          list(APPEND LAPACK_LIBRARIES ${GFORTRAN_LIB})
        endif(GFORTRAN_LIB)
      endif(LAPACK_LIBRARIES)  
    endif(NOT HAVE_BLAS OR NOT HAVE_LAPACK)

    if(NOT HAVE_BLAS OR NOT HAVE_LAPACK)
      # if we still haven't found blas and lapack, use the standard cmake tests,
      # which require a working Fortran compiler
      enable_language(Fortran)
      find_package(BLAS)
      if(BLAS_FOUND)
        set_config_option(HAVE_BLAS "Blas")
        find_package(LAPACK)
        if(LAPACK_FOUND)
          set_config_option(HAVE_LAPACK "Lapack")
        else(LAPACK_FOUND)
          set(LAPACK_LIBRARIES ${BLAS_LIBRARIES})
        endif(LAPACK_FOUND)
        if(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
          if(CMAKE_Fortran_COMPILER MATCHES "gfortran")
            list(APPEND LAPACK_LIBRARIES gfortran)
          elseif(CMAKE_Fortran_COMPILER MATCHES "f95")
            list(APPEND LAPACK_LIBRARIES gfortran)
          elseif(CMAKE_Fortran_COMPILER MATCHES "g77")
            list(APPEND LAPACK_LIBRARIES g2c)
          endif(CMAKE_Fortran_COMPILER MATCHES "gfortran")
        endif(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
      endif(BLAS_FOUND)
    endif(NOT HAVE_BLAS OR NOT HAVE_LAPACK)

    if(NOT HAVE_BLAS OR NOT HAVE_LAPACK)
      message(STATUS "Warning: Could not find Blas or Lapack: most meshing algorithms "
              "will not be functional")
    endif(NOT HAVE_BLAS OR NOT HAVE_LAPACK)

  endif(BLAS_LAPACK_LIBRARIES)
endif(ENABLE_BLAS_LAPACK)

add_subdirectory(Common)
add_subdirectory(Numeric)
add_subdirectory(Geo)

if(ENABLE_MESH)
  add_subdirectory(Mesh)
  set_config_option(HAVE_MESH "Mesh")
endif(ENABLE_MESH)

if(ENABLE_SOLVER)
  add_subdirectory(Solver)
  set_config_option(HAVE_SOLVER "Solver")
endif(ENABLE_SOLVER)

if(ENABLE_POST)
  add_subdirectory(Post)
  set_config_option(HAVE_POST "Post")
  if(ENABLE_PLUGINS)
    add_subdirectory(Plugin)
    set_config_option(HAVE_PLUGINS "Plugins")
  endif(ENABLE_PLUGINS)
endif(ENABLE_POST)

if(ENABLE_PARSER)
  add_subdirectory(Parser)
  set_config_option(HAVE_PARSER "Parser")
endif(ENABLE_PARSER)

if(HAVE_MESH OR HAVE_PLUGINS)
  if(ENABLE_ANN)
    find_library(ANN_LIB ann PATH_SUFFIXES lib)
    find_path(ANN_INC "ANN.h" PATH_SUFFIXES src include ANN)
    if(ANN_LIB AND ANN_INC)
      list(APPEND EXTERNAL_LIBRARIES ${ANN_LIB})
      list(APPEND EXTERNAL_INCLUDES ${ANN_INC})
    else(ANN_LIB AND ANN_INC)
      message(STATUS "System ANN not found: using contrib/ANN instead")
      add_subdirectory(contrib/ANN)
      include_directories(contrib/ANN/include)
    endif(ANN_LIB AND ANN_INC)
    set_config_option(HAVE_ANN "Ann")
  endif(ENABLE_ANN)
endif(HAVE_MESH OR HAVE_PLUGINS)

if(ENABLE_BFGS)
  add_subdirectory(contrib/lbfgs)
  include_directories(contrib/lbfgs)
  set_config_option(HAVE_BFGS "Bfgs")
endif(ENABLE_BFGS)

if(HAVE_FLTK AND ENABLE_CAIRO)
  find_library(CAIRO_LIB cairo)
  find_path(CAIRO_INC "cairo/cairo.h" PATH_SUFFIXES include)
  if(CAIRO_INC AND CAIRO_LIB)
     set_config_option(HAVE_CAIRO "Cairo")
     list(APPEND EXTERNAL_LIBRARIES ${CAIRO_LIB})
     list(APPEND EXTERNAL_INCLUDES ${CAIRO_INC})
  endif(CAIRO_INC AND CAIRO_LIB)
endif(HAVE_FLTK AND ENABLE_CAIRO)

if(ENABLE_DINTEGRATION)
  add_subdirectory(contrib/DiscreteIntegration)
  include_directories(contrib/DiscreteIntegration)
  set_config_option(HAVE_DINTEGRATION "DIntegration")
endif(ENABLE_DINTEGRATION)

if(ENABLE_OPTHOM)
  add_subdirectory(contrib/HighOrderMeshOptimizer)
  include_directories(contrib/HighOrderMeshOptimizer)
  set_config_option(HAVE_OPTHOM "OptHom")
endif(ENABLE_OPTHOM)

if(ENABLE_KBIPACK)
  set_config_option(HAVE_KBIPACK "Kbipack")
  add_subdirectory(contrib/kbipack)
  include_directories(contrib/kbipack)
  find_library(GMP_LIB gmp)
  find_path(GMP_INC "gmp.h" PATH_SUFFIXES src include)
  if(GMP_LIB AND GMP_INC)
    set_config_option(HAVE_GMP "GMP")
    list(APPEND EXTERNAL_LIBRARIES ${GMP_LIB})
    list(APPEND EXTERNAL_INCLUDES ${GMP_INC})
  else(GMP_LIB AND GMP_INC)
    message(STATUS "GMP not found: Kbipack uses long int")
  endif(GMP_LIB AND GMP_INC)
endif(ENABLE_KBIPACK)

if(ENABLE_MATHEX)
  add_subdirectory(contrib/MathEx)
  include_directories(contrib/MathEx)
  set_config_option(HAVE_MATHEX "MathEx")
endif(ENABLE_MATHEX)

if(ENABLE_MPI)
   find_package(MPI)
   if(MPI_FOUND)
     set_config_option(HAVE_MPI "MPI")
     list(APPEND EXTERNAL_INCLUDES ${MPI_INCLUDE_DIR})
     list(APPEND EXTERNAL_LIBRARIES ${MPI_LIBRARIES})
     include(CMakeForceCompiler)
     # Warning: this actually requires cmake >= 2.8.5
     cmake_force_c_compiler(${MPI_C_COMPILER} "MPI C Compiler")
     cmake_force_cxx_compiler(${MPI_CXX_COMPILER} "MPI C++ Compiler")
   endif(MPI_FOUND)
endif(ENABLE_MPI)

if(HAVE_MESH OR HAVE_SOLVER)
  if(ENABLE_METIS)
    # Gmsh currently uses Metis 4 - we should switch to Metis 5 and use the
    # system lib if available
    add_subdirectory(contrib/Metis)
    include_directories(contrib/Metis)
    set_config_option(HAVE_METIS "Metis")
    message(STATUS "Note: By including Metis you have to comply with Metis' "
            "special licensing requirements stated in contrib/Metis/README.txt.")
  endif(ENABLE_METIS)

  if(ENABLE_CHACO)
    add_subdirectory(contrib/Chaco)
    include_directories(contrib/Chaco/main)
    set_config_option(HAVE_CHACO "Chaco")
  endif(ENABLE_CHACO)
endif(HAVE_MESH OR HAVE_SOLVER)

if(HAVE_MESH)
  if(ENABLE_RTREE)
    include_directories(contrib/rtree)
    set_config_option(HAVE_RTREE "RTree")
  endif(ENABLE_RTREE)

  if(ENABLE_VORO3D)
    add_subdirectory(contrib/voro++)
    include_directories(contrib/voro++/src)
    set_config_option(HAVE_VORO3D "Voro3D")
  endif(ENABLE_VORO3D)

  if(ENABLE_BLOSSOM)
    add_subdirectory(contrib/blossom)
    include_directories(contrib/blossom/MATCH contrib/blossom/concorde97
                        contrib/blossom/concorde97/INCLUDE)
    set_config_option(HAVE_BLOSSOM "Blossom")
  endif(ENABLE_BLOSSOM)
endif(HAVE_MESH)

check_function_exists(vsnprintf HAVE_VSNPRINTF)
if(NOT HAVE_VSNPRINTF AND NOT ENABLE_BUILD_IOS)
  set_config_option(HAVE_NO_VSNPRINTF "NoVsnprintf")
endif(NOT HAVE_VSNPRINTF AND NOT ENABLE_BUILD_IOS)

check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
if(HAVE_SYS_SOCKET_H)
  set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
endif(HAVE_SYS_SOCKET_H)
check_type_size(socklen_t SOCKLEN_T_SIZE)
set(CMAKE_EXTRA_INCLUDE_FILES)
if(NOT SOCKLEN_T_SIZE AND NOT ENABLE_BUILD_IOS)
  set_config_option(HAVE_NO_SOCKLEN_T "NoSocklenT")
endif(NOT SOCKLEN_T_SIZE AND NOT ENABLE_BUILD_IOS)

check_include_file(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
  set(CMAKE_EXTRA_INCLUDE_FILES stdint.h)
else(HAVE_STDINT_H)
  set_config_option(HAVE_NO_STDINT_H "NoStdintH")
endif(HAVE_STDINT_H)
check_type_size(intptr_t INTPTR_T_SIZE)
set(CMAKE_EXTRA_INCLUDE_FILES)
if(NOT INTPTR_T_SIZE AND NOT ENABLE_BUILD_IOS)
  set_config_option(HAVE_NO_INTPTR_T "NoIntptrT")
endif(NOT INTPTR_T_SIZE AND NOT ENABLE_BUILD_IOS)

check_include_file(dlfcn.h DLFCN_H)
if(DLFCN_H)
  set_config_option(HAVE_DLOPEN "Dlopen")
  list(APPEND EXTERNAL_LIBRARIES ${CMAKE_DL_LIBS})
endif(DLFCN_H)

check_include_file(linux/joystick.h LINUX_JOYSTICK_H)
if(LINUX_JOYSTICK_H)
  set_config_option(HAVE_LINUX_JOYSTICK "LinuxJoystick")
endif(LINUX_JOYSTICK_H)

if(MSVC)
  add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX 
                  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
endif(MSVC)

if(WIN32 OR CYGWIN)
  list(APPEND EXTERNAL_LIBRARIES winmm wsock32 ws2_32 psapi)
endif(WIN32 OR CYGWIN)

# force full warnings to encourage everybody to write clean(er) code
check_cxx_compiler_flag("-Wall" WALL)
check_cxx_compiler_flag("-Wint-to-void-pointer-cast" WCAST)
check_cxx_compiler_flag("-Wdeprecated-declarations" WDEPREC)
if(WALL AND NOT MSVC)
  file(GLOB_RECURSE WALL_SRC Common/*.cpp Fltk/*.cpp FunctionSpace/*.cpp
       Geo/*.cpp Graphics/*.cpp Mesh/*.cpp Numeric/*.cpp Parser/*.cpp
       Plugin/*.cpp Post/*.cpp Qt/*.cpp Solver/*.cpp)
  set(WF "-Wall")
  if(WCAST)
    set(WF "${WF} -Wno-int-to-void-pointer-cast")
  endif(WCAST)
  if(WDEPREC)
    # FIXME: remove this when we have fixed the deprecated GLU code for OpenGL3
    set(WF "${WF} -Wno-deprecated-declarations")
  endif(WDEPREC)
  set_source_files_properties(${WALL_SRC} PROPERTIES COMPILE_FLAGS ${WF})
endif(WALL AND NOT MSVC)

# don't issue warnings for contributed libraries
check_cxx_compiler_flag("-w" NOWARN)
if(NOWARN)
  file(GLOB_RECURSE NOWARN_SRC contrib/*.cpp contrib/*.cc contrib/*.cxx
                               contrib/*.c)
  set_source_files_properties(${NOWARN_SRC} PROPERTIES COMPILE_FLAGS "-w")
endif(NOWARN)

# disable compile optimization on some known problematic files
check_cxx_compiler_flag("-O0" NOOPT)
if(NOOPT)
  file(GLOB_RECURSE NOOPT_SRC Numeric/robustPredicates.cpp Mesh/BDS.cpp
                              Parser/Gmsh.tab.cpp contrib/Tetgen*/*.cxx)
  foreach(FILE ${NOOPT_SRC})
    get_source_file_property(PROP ${FILE} COMPILE_FLAGS)
    if(PROP)
      set_source_files_properties(${FILE} PROPERTIES COMPILE_FLAGS "${PROP} -O0")
    else(PROP)
      set_source_files_properties(${FILE} PROPERTIES COMPILE_FLAGS "-O0")
    endif(PROP)
  endforeach(FILE)
endif(NOOPT)

list(SORT CONFIG_OPTIONS)
set(GMSH_CONFIG_OPTIONS "")
foreach(OPT ${CONFIG_OPTIONS})
  set(GMSH_CONFIG_OPTIONS "${GMSH_CONFIG_OPTIONS} ${OPT}")
endforeach(OPT)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Common/GmshConfig.h.in 
               ${CMAKE_CURRENT_BINARY_DIR}/Common/GmshConfig.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Common/GmshVersion.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/Common/GmshVersion.h)

# process cmake environment variables so we can append them to the -I include
# commands. This is not recommended (we should only use the cache variables) but
# it is very convenient: otherwise we have to remember providing the
# -D... options to cmake for each new build.
set(ENV_CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
set(ENV_CMAKE_INCLUDE_PATH $ENV{CMAKE_INCLUDE_PATH})
if(UNIX)
  if(ENV_CMAKE_PREFIX_PATH)
    string(REPLACE ":" ";" ENV_CMAKE_PREFIX_PATH ${ENV_CMAKE_PREFIX_PATH})
  endif(ENV_CMAKE_PREFIX_PATH)
  if(ENV_CMAKE_INCLUDE_PATH)
    string(REPLACE ":" ";" ENV_CMAKE_INCLUDE_PATH ${ENV_CMAKE_INCLUDE_PATH})
  endif(ENV_CMAKE_INCLUDE_PATH)
endif(UNIX)
list(APPEND EXTERNAL_INCLUDES ${CMAKE_INCLUDE_PATH} ${ENV_CMAKE_INCLUDE_PATH})
list(APPEND EXTERNAL_INCLUDES ${CMAKE_PREFIX_PATH} ${ENV_CMAKE_PREFIX_PATH})
foreach(DIR ${CMAKE_PREFIX_PATH} ${ENV_CMAKE_PREFIX_PATH})
  list(APPEND EXTERNAL_INCLUDES ${DIR}/include)
endforeach(DIR)

if(EXTERNAL_INCLUDES)
  list(REMOVE_DUPLICATES EXTERNAL_INCLUDES)
endif(EXTERNAL_INCLUDES)

if(HAVE_FLTK)
  set(LINK_LIBRARIES ${FLTK_LIBRARIES} ${EXTERNAL_LIBRARIES} 
                     ${OPENGL_LIBRARIES} ${LAPACK_LIBRARIES})
elseif(HAVE_QT)
  set(LINK_LIBRARIES ${QT_LIBRARIES} ${EXTERNAL_LIBRARIES}
                     ${OPENGL_LIBRARIES} ${LAPACK_LIBRARIES})
elseif(HAVE_OPENGL)
  set(LINK_LIBRARIES ${EXTERNAL_LIBRARIES} ${OPENGL_LIBRARIES}
                     ${LAPACK_LIBRARIES})
else(HAVE_FLTK)
  set(LINK_LIBRARIES ${EXTERNAL_LIBRARIES} ${LAPACK_LIBRARIES})
endif(HAVE_FLTK)

# try to use static gfortran on static Linux builds
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  if(NOT ENABLE_BUILD_DYNAMIC)
    find_library(GFORTRAN_STATIC libgfortran.a)
    if(GFORTRAN_STATIC)
      message(STATUS "Using static libgfortran")
      foreach(STR ${LINK_LIBRARIES})
        string(REPLACE "-lgfortran" ${GFORTRAN_STATIC} STR2 ${STR})
        list(APPEND LINK_LIBRARIES2 ${STR2})
      endforeach(STR)
      set(LINK_LIBRARIES ${LINK_LIBRARIES2})
    endif(GFORTRAN_STATIC)
  endif(NOT ENABLE_BUILD_DYNAMIC)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# we could specify include dirs more selectively, but this is simpler
include_directories(Common Fltk Geo Graphics Mesh Solver Numeric Parser
  Plugin Post Qt ${EXTERNAL_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/Common)

# set this for external codes that might include this CMakeList file
set(GMSH_EXTERNAL_INCLUDE_DIRS ${EXTERNAL_INCLUDES} CACHE 
    STRING "External include directories" FORCE)
set(GMSH_EXTERNAL_LIBRARIES ${LINK_LIBRARIES} CACHE 
    STRING "External libraries" FORCE)

# group sources for easier navigation in IDEs
foreach(DIR ${GMSH_DIRS})
  string(REGEX REPLACE "\\+" "\\\\+" DIR ${DIR})
  source_group(${DIR} REGULAR_EXPRESSION ${DIR}/.*)
endforeach(DIR)

# static library target
if(ENABLE_BUILD_LIB)
  add_library(lib STATIC ${GMSH_SRC})
  set_target_properties(lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
  if(MSVC)
    set_target_properties(lib PROPERTIES DEBUG_POSTFIX d) 
    if(ENABLE_MSVC_STATIC_RUNTIME)
      set_target_properties(lib PROPERTIES LINK_FLAGS_RELEASE "/nodefaultlib:LIBCMT")
    endif(ENABLE_MSVC_STATIC_RUNTIME)
  endif(MSVC)
  TARGET_LINK_LIBRARIES(lib ${PROJECT_NAME} ${LINK_LIBRARIES})
endif(ENABLE_BUILD_LIB)

# shared library target
if(ENABLE_BUILD_SHARED OR ENABLE_BUILD_DYNAMIC OR 
   ENABLE_WRAP_PYTHON OR ENABLE_WRAP_JAVA)
  add_library(${PROJECT_NAME} SHARED ${GMSH_SRC})
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
  set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${GMSH_MAJOR_VERSION}.${GMSH_MINOR_VERSION}.${GMSH_PATCH_VERSION}
    SOVERSION ${GMSH_MAJOR_VERSION}.${GMSH_MINOR_VERSION})
  if(HAVE_LAPACK AND LAPACK_FLAGS)
    set_target_properties(shared PROPERTIES LINK_FLAGS ${LAPACK_FLAGS})
  endif(HAVE_LAPACK AND LAPACK_FLAGS)
  if(MSVC AND ENABLE_MSVC_STATIC_RUNTIME)
    message(STATUS "Note: By enabling ENABLE_MSVC_STATIC_RUNTIME, shared library "
            "won't link. In MSVC change /MT to /MD in the shared project properties")
  endif(MSVC AND ENABLE_MSVC_STATIC_RUNTIME)		  
  target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})
endif(ENABLE_BUILD_SHARED OR ENABLE_BUILD_DYNAMIC OR 
      ENABLE_WRAP_PYTHON OR ENABLE_WRAP_JAVA)

# binary targets
# if(HAVE_FLTK)
#   if(ENABLE_BUILD_DYNAMIC)
#     add_executable(gmsh WIN32 Fltk/Main.cpp)
#     target_link_libraries(gmsh shared)
#   else(ENABLE_BUILD_DYNAMIC)
#     add_executable(gmsh WIN32 Fltk/Main.cpp ${GMSH_SRC})
#   endif(ENABLE_BUILD_DYNAMIC)
# elseif(HAVE_QT)
#   qt4_wrap_cpp(GMSH_MOC_SRC ${GMSH_MOC_HDR})
#   add_executable(gmsh WIN32 Qt/Main.cpp ${GMSH_SRC} ${GMSH_MOC_SRC})
# else(HAVE_FLTK)
#   if(ENABLE_BUILD_DYNAMIC)
#     add_executable(gmsh Common/Main.cpp)
#     target_link_libraries(gmsh shared)
#   else(ENABLE_BUILD_DYNAMIC)
#     add_executable(gmsh Common/Main.cpp ${GMSH_SRC})
#   endif(ENABLE_BUILD_DYNAMIC)
# endif(HAVE_FLTK)
#target_link_libraries(gmsh ${LINK_LIBRARIES})

# Windows specific linker options
if(WIN32 AND NOT MSVC OR CYGWIN)
  set(FLAGS "-Wl,--stack,16777216 -mwindows")
  if(HAVE_64BIT_SIZE_T)
    set(FLAGS "${FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/Win64Icon.res")
  else(HAVE_64BIT_SIZE_T)
    set(FLAGS "${FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/Win32Icon.res")
  endif(HAVE_64BIT_SIZE_T)
  if(ENABLE_BUILD_DYNAMIC)
    set(FLAGS "${FLAGS} -Wl,-Bstatic -lgfortran")
    set_target_properties(gmsh PROPERTIES 
      LINK_FLAGS "${FLAGS} -Wl,--enable-auto-import")
    set(LIBGMSH_DEF "libGmsh-${GMSH_MAJOR_VERSION}.${GMSH_MINOR_VERSION}.def")
    set_target_properties(shared PROPERTIES PREFIX "lib"
      LINK_FLAGS "${FLAGS} -Wl,--export-all-symbols,--output-def,${LIBGMSH_DEF}")
  else(ENABLE_BUILD_DYNAMIC)
    set_target_properties(gmsh PROPERTIES 
      LINK_FLAGS "${FLAGS} -static")
    # remove -Wl,-Bdynamic flags
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
  endif(ENABLE_BUILD_DYNAMIC)
elseif(MSVC)
  set_target_properties(${GMSH_LIBRARY} PROPERTIES LINK_FLAGS "/STACK:16777216")
endif(WIN32 AND NOT MSVC OR CYGWIN)

# parser target
find_program(BISON bison)
find_program(FLEX flex)
if(BISON AND FLEX)
  add_custom_target(parser 
                    COMMAND ${BISON} -p gmsh_yy --output Gmsh.tab.cpp -d Gmsh.y
                    COMMAND ${FLEX} -P gmsh_yy -o Gmsh.yy.cpp Gmsh.l
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Parser)
endif(BISON AND FLEX)

if(UNIX)
  # cannot use cmake's file search functions here (they would only find files
  # existing at configuration time)
  add_custom_target(purge 
                    COMMAND rm -f `find . -name *~ -o -name *~~`
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(etags 
                    COMMAND etags `find . -name *.cpp -o -name *.h -o -name *.y`
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif(UNIX)

set(WELCOME_FILE ${CMAKE_CURRENT_SOURCE_DIR}/doc/WELCOME.txt)
set(LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/doc/LICENSE.txt)
set(CREDITS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/doc/CREDITS.txt)
set(ONELAB_PY ${CMAKE_CURRENT_SOURCE_DIR}/contrib/onelab/python/onelab.py)
file(GLOB TUTORIAL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tutorial/?*.*)
file(GLOB DEMO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/demos/?*.*)
set(TEX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc/texinfo)
file(GLOB TEX_SRC ${TEX_DIR}/*.texi)
set(TEX_OBJ ${TEX_DIR}/gmsh.aux ${TEX_DIR}/gmsh.cp ${TEX_DIR}/gmsh.cps
    ${TEX_DIR}/gmsh.fn ${TEX_DIR}/gmsh.html ${TEX_DIR}/gmsh.info ${TEX_DIR}/gmsh.ky 
    ${TEX_DIR}/gmsh.log ${TEX_DIR}/gmsh.pdf ${TEX_DIR}/gmsh.pg ${TEX_DIR}/gmsh.toc 
    ${TEX_DIR}/gmsh.tp ${TEX_DIR}/gmsh.tps ${TEX_DIR}/gmsh.txt ${TEX_DIR}/gmsh.vr)

macro(unix2dos VARNAME)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unix2dos)
  set(UNIX2DOS_FILES)
  foreach(FILE ${${VARNAME}})
    file(READ ${FILE} F0)
    get_filename_component(N ${FILE} NAME)
    if(CYGWIN)
      string(REGEX REPLACE "\n" "\r\n" F1 "${F0}")
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/unix2dos/${N} "${F1}")
    else(CYGWIN) # if not in cygwin, cmake adds '\r's automatically
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/unix2dos/${N} "${F0}")
    endif(CYGWIN)
    list(APPEND UNIX2DOS_FILES ${CMAKE_CURRENT_BINARY_DIR}/unix2dos/${N})
  endforeach(FILE)
  set(${VARNAME} ${UNIX2DOS_FILES})
endmacro(unix2dos)

if(WIN32 OR CYGWIN)
  if(ENABLE_OS_SPECIFIC_INSTALL)
    set(GMSH_BIN .)
    set(GMSH_LIB .)
    set(GMSH_DOC .)
    set(GMSH_MAN .)
  else(ENABLE_OS_SPECIFIC_INSTALL)
    set(GMSH_BIN bin)
    set(GMSH_LIB lib)
    set(GMSH_DOC share/doc/gmsh)
    set(GMSH_MAN share/man/man1)
  endif(ENABLE_OS_SPECIFIC_INSTALL)
  if(CYGWIN)
    unix2dos(GMSH_API)
    unix2dos(WELCOME_FILE)
    unix2dos(LICENSE_FILE)
    unix2dos(CREDITS_FILE)
    unix2dos(TUTORIAL_FILES)
    unix2dos(DEMO_FILES)
  endif(CYGWIN)
elseif(APPLE AND ENABLE_OS_SPECIFIC_INSTALL)
  # set these so that the files get installed nicely in the MacOSX
  # .app bundle
  set(GMSH_BIN ../MacOS)
  set(GMSH_LIB ../MacOS)
  set(GMSH_DOC ../../..)
  set(GMSH_MAN ../../..)
else(WIN32 OR CYGWIN)
  set(GMSH_BIN bin)
  set(GMSH_LIB lib)
  set(GMSH_DOC share/doc/gmsh)
  set(GMSH_MAN share/man/man1)
endif(WIN32 OR CYGWIN)

# mark targets as optional so we can install them separately if needed
# (e.g. "make lib" or "make shared" followed by "make install/fast")
# install(TARGETS gmsh DESTINATION ${GMSH_BIN} OPTIONAL)
if(ENABLE_BUILD_LIB)
  install(TARGETS lib DESTINATION ${GMSH_LIB} OPTIONAL)
endif(ENABLE_BUILD_LIB)
if(ENABLE_BUILD_SHARED OR ENABLE_BUILD_DYNAMIC)
  if(WIN32 AND NOT MSVC OR CYGWIN)
    # install only the runtime (DLL); the .dll.a generated by gcc is not usable
    # by MSVC
    install(TARGETS shared RUNTIME DESTINATION ${GMSH_LIB} OPTIONAL)
    # install .def file
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIBGMSH_DEF} 
            DESTINATION ${GMSH_LIB})
    # install these DLLs until we figure out how to link them in statically
    # (libstc++ and libgcc should be linkable statically with -static-libstdc++
    # and -static-libgcc but with current mingw64 toolchain it does not
    # work). Also, when cross-compilating find_program does not work -> ask the
    # compiler instead
    find_program(GCC_DLL libgcc_s_sjlj-1.dll)
    find_program(STDC_DLL libstdc++-6.dll)
    if(NOT GCC_DLL)
      execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libgcc_s_seh-1.dll
                      OUTPUT_VARIABLE GCC_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (NOT EXISTS "${GCC_DLL}")
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libgcc_s_sjlj-1.dll
                        OUTPUT_VARIABLE GCC_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
      endif(NOT EXISTS "${GCC_DLL}")
      get_filename_component(GCC_DLL ${GCC_DLL} ABSOLUTE)
    endif(NOT GCC_DLL)
    if(NOT STDC_DLL)
      execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++-6.dll
                      OUTPUT_VARIABLE STDC_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
      get_filename_component(STD_DLL ${STDC_DLL} ABSOLUTE)
    endif(NOT STDC_DLL)
    if(GCC_DLL AND STDC_DLL)
      message(STATUS "Will install extra DLLs for Windows")
      install(FILES ${GCC_DLL} ${STDC_DLL} DESTINATION ${GMSH_LIB})
    endif(GCC_DLL AND STDC_DLL)
  else(WIN32 AND NOT MSVC OR CYGWIN)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${GMSH_LIB} OPTIONAL)
  endif(WIN32 AND NOT MSVC OR CYGWIN)
endif(ENABLE_BUILD_SHARED OR ENABLE_BUILD_DYNAMIC)

if(ENABLE_ONELAB)
  install(FILES ${ONELAB_PY} DESTINATION ${GMSH_BIN})
endif(ENABLE_ONELAB)
if(ENABLE_BUILD_LIB OR ENABLE_BUILD_SHARED OR ENABLE_BUILD_DYNAMIC)
  install(FILES ${GMSH_API} DESTINATION include/gmsh)
endif(ENABLE_BUILD_LIB OR ENABLE_BUILD_SHARED OR ENABLE_BUILD_DYNAMIC)
install(FILES ${WELCOME_FILE} DESTINATION ${GMSH_DOC} RENAME README.txt)
install(FILES ${LICENSE_FILE} DESTINATION ${GMSH_DOC})
install(FILES ${CREDITS_FILE} DESTINATION ${GMSH_DOC})
install(FILES ${TUTORIAL_FILES} DESTINATION ${GMSH_DOC}/tutorial)
install(FILES ${DEMO_FILES} DESTINATION ${GMSH_DOC}/demos)
if(UNIX AND NOT CYGWIN)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/gmsh.1 DESTINATION ${GMSH_MAN})
endif(UNIX AND NOT CYGWIN)

add_custom_target(getHeaders
  COMMAND ${CMAKE_COMMAND} -E remove_directory Headers
  COMMAND ${CMAKE_COMMAND} -E make_directory Headers/gmsh
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  foreach(FILE ${GMSH_API})
    add_custom_command(TARGET getHeaders POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
        ${FILE} ${CMAKE_CURRENT_BINARY_DIR}/Headers/gmsh/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach(FILE)

find_program(MAKEINFO makeinfo)
if(MAKEINFO)
  add_custom_command(OUTPUT ${TEX_DIR}/gmsh.info DEPENDS ${TEX_SRC} 
                     COMMAND ${MAKEINFO} --split-size 1000000
                     ARGS ${TEX_DIR}/gmsh.texi WORKING_DIRECTORY ${TEX_DIR})
  add_custom_target(info DEPENDS ${TEX_DIR}/gmsh.info)
  add_custom_command(OUTPUT ${TEX_DIR}/gmsh.txt DEPENDS ${TEX_SRC} 
                     COMMAND ${MAKEINFO} --plaintext -o gmsh.txt
                     ARGS ${TEX_DIR}/gmsh.texi WORKING_DIRECTORY ${TEX_DIR})
  add_custom_target(txt DEPENDS ${TEX_DIR}/gmsh.txt)
  add_custom_command(OUTPUT ${TEX_DIR}/gmsh.html DEPENDS ${TEX_SRC} 
                     COMMAND ${MAKEINFO} --html --css-ref=http://geuz.org/geuz.css --no-split
                     ARGS ${TEX_DIR}/gmsh.texi WORKING_DIRECTORY ${TEX_DIR})
  add_custom_target(html DEPENDS ${TEX_DIR}/gmsh.html)
  install(FILES ${TEX_DIR}/gmsh.html DESTINATION ${GMSH_DOC} OPTIONAL)
endif(MAKEINFO)

find_program(TEXI2PDF texi2pdf)
if(TEXI2PDF)
  add_custom_command(OUTPUT ${TEX_DIR}/gmsh.pdf DEPENDS ${TEX_SRC} 
                     COMMAND ${TEXI2PDF} ARGS gmsh.texi 
                     WORKING_DIRECTORY ${TEX_DIR})
  add_custom_target(pdf DEPENDS ${TEX_DIR}/gmsh.pdf)
  install(FILES ${TEX_DIR}/gmsh.pdf DESTINATION ${GMSH_DOC} OPTIONAL)
endif(TEXI2PDF)

execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${OPT_TEXI}"
                OUTPUT_FILE cmake_options.texi)

if(MAKEINFO AND TEXI2PDF)
  add_custom_target(doc COMMAND ${CMAKE_COMMAND} -E tar zcf 
                    ${CMAKE_CURRENT_BINARY_DIR}/gmsh-${GMSH_VERSION}-doc.tgz
                    doc/CREDITS.txt doc/LICENSE.txt doc/VERSIONS.txt
                    doc/gmsh.1 doc/texinfo/gmsh.html doc/texinfo/gmsh.info 
                    doc/texinfo/gmsh.pdf doc/texinfo/gmsh.txt
                    COMMAND ${CMAKE_COMMAND} -E remove ${TEX_OBJ}
                    DEPENDS ${TEX_DIR}/gmsh.info ${TEX_DIR}/gmsh.txt 
                    ${TEX_DIR}/gmsh.html ${TEX_DIR}/gmsh.pdf
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif(MAKEINFO AND TEXI2PDF)

if(MAKEINFO OR TEXI2PDF)
  add_custom_target(clean_doc COMMAND ${CMAKE_COMMAND} -E remove ${TEX_OBJ})
endif(MAKEINFO OR TEXI2PDF)

if(APPLE AND ENABLE_BUILD_LIB)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/utils/misc/gmsh_framework.plist F0)
  string(REPLACE GMSH_VERSION "${GMSH_VERSION}" F1 "${F0}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Info_framework.plist "${F1}")
  get_target_property(LIBNAME lib LOCATION)
  add_custom_target(framework DEPENDS lib
    COMMAND ${CMAKE_COMMAND} -E remove_directory Gmsh.framework
    COMMAND ${CMAKE_COMMAND} -E make_directory Gmsh.framework/Versions/A/Headers
    COMMAND ${CMAKE_COMMAND} -E make_directory Gmsh.framework/Versions/A/Resources
    COMMAND ${CMAKE_COMMAND} -E copy ${LIBNAME} Gmsh.framework/Versions/A/Gmsh
    COMMAND ${CMAKE_COMMAND} -E copy Info_framework.plist 
                                     Gmsh.framework/Versions/A/Resources/Info.plist
    COMMAND ${CMAKE_COMMAND} -E create_symlink A Gmsh.framework/Versions/Current
    COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/Gmsh 
                                               Gmsh.framework/Gmsh
    COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/Headers
                                               Gmsh.framework/Headers
    COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/Resources
                                               Gmsh.framework/Resources 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  foreach(FILE ${GMSH_API})
    add_custom_command(TARGET framework POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
        ${FILE} ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.framework/Versions/A/Headers/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach(FILE)
endif(APPLE AND ENABLE_BUILD_LIB)

set(CPACK_PACKAGE_VENDOR "Christophe Geuzaine and Jean-Francois Remacle")
set(CPACK_PACKAGE_VERSION_MAJOR ${GMSH_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${GMSH_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${GMSH_PATCH_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_FILE ${WELCOME_FILE})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
    "3D finite element mesh generator with built-in CAD engine and post-processor")
set(CPACK_PACKAGE_FILE_NAME gmsh-${GMSH_VERSION}-${GMSH_OS})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Gmsh")
set(CPACK_RESOURCE_FILE_LICENSE ${LICENSE_FILE})
set(CPACK_RESOURCE_FILE_README ${WELCOME_FILE})
set(CPACK_RESOURCE_FILE_WELCOME ${WELCOME_FILE})
set(CPACK_PACKAGE_EXECUTABLE "gmsh;Gmsh")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_SOURCE_PACKAGE_FILE_NAME gmsh-${GMSH_VERSION}-source)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}" "/CVS/" "/.svn" "~$" 
    "DS_Store$" "GmshConfig.h$" "GmshVersion.h$" "/benchmarks/" "/tmp/" 
    "/bin/" "/lib/" "/nightly/" "GPATH" "GRTAGS" "GSYMS" "GTAGS" "/HTML/" 
    "/projects/" 
    "/FunctionSpace/" "/Mesh/meshRecombine*" "/utils/solvers/legacy/"
    "/contrib/3M/")

if(UNIX)
  # make sure we remove previous installs before doing the next one (on Mac for
  # example "make package; make package_source" would lead to huge file lists
  # getting generated due to the 'Applications' symlink in the bundle)
  set(CPACK_INSTALL_COMMANDS "rm -rf ${CMAKE_CURRENT_BINARY_DIR}/_CPack_Packages")
endif(UNIX)

if(APPLE AND ENABLE_OS_SPECIFIC_INSTALL)
  set(CPACK_GENERATOR Bundle)
  set(CPACK_BUNDLE_NAME Gmsh)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/utils/misc/gmsh_app.plist F0)
  string(REPLACE GMSH_VERSION "${GMSH_VERSION}" F1 "${F0}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Info.plist "${F1}")
  set(CPACK_BUNDLE_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/MacIcons.icns)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/MacIconsGeo.icns DESTINATION .
          RENAME GmshGeo.icns)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/MacIconsMsh.icns DESTINATION .
          RENAME GmshMsh.icns)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/MacIconsSol.icns DESTINATION .
          RENAME GmshSol.icns)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/MacIconsPos.icns DESTINATION .
          RENAME GmshPos.icns)
  set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/Fltk/MacIcons.icns)
elseif(WIN32 OR CYGWIN)
  set(CPACK_GENERATOR ZIP)
else(APPLE AND ENABLE_OS_SPECIFIC_INSTALL)
  set(CPACK_GENERATOR TGZ)
endif(APPLE AND ENABLE_OS_SPECIFIC_INSTALL)

if (HAVE_PYTHON)
  add_subdirectory(wrappers/gmshpy)
endif(HAVE_PYTHON)

if(ENABLE_WRAP_JAVA) 
  if(NOT HAVE_BLAS OR NOT HAVE_LAPACK) 
    message(WARNING "Java wrapping samples will not work without BLAS and LAPACK")
  endif(NOT HAVE_BLAS OR NOT HAVE_LAPACK)
  add_subdirectory(wrappers/java) 
endif(ENABLE_WRAP_JAVA)

include(CPack)

include(CTest)
file(GLOB_RECURSE TESTFILES tutorial/*.geo)
foreach(TESTFILE ${TESTFILES})
  add_test(${TESTFILE} ./gmsh ${TESTFILE} -3 -o ./tmp.msh)
endforeach()

message(STATUS "")
message(STATUS "Gmsh ${GMSH_VERSION} has been configured for ${GMSH_OS}")
message(STATUS "")
message(STATUS " * Build options:" ${GMSH_CONFIG_OPTIONS})
message(STATUS " * Build type: " ${CMAKE_BUILD_TYPE})
message(STATUS " * C compiler: " ${CMAKE_C_COMPILER})
message(STATUS " * C++ compiler: " ${CMAKE_CXX_COMPILER})
message(STATUS " * Install prefix: " ${CMAKE_INSTALL_PREFIX})
message(STATUS "")
message(STATUS "Run 'ccmake ${CMAKE_CURRENT_SOURCE_DIR}' to fine-tune the configuration.")
message(STATUS "")

mark_as_advanced(GMSH_EXTRA_VERSION 
                 ACIS_LIB ANN_INC ANN_LIB CAIRO_LIB CAIRO_INC CGNS_INC GMM_INC 
                 GMP_INC GMP_LIB MMG3D_INC MMG3D_LIB TAUCS_INC TAUCS_LIB HDF5_LIB 
                 MED_LIB OCC_INC OCC_CONFIG_H SZ_LIB 
                 PETSC_LIBS SLEPC_INC SLEPC_INC2 SLEPC_LIB
                 BISON FLEX MAKEINFO TEXI2PDF FLTK_CONFIG_SCRIPT
                 GMSH_EXTERNAL_INCLUDE_DIRS GMSH_EXTERNAL_LIBRARIES)
