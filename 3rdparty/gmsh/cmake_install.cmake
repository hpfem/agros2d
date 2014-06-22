# Install script for directory: /home/karban/Projects/agros2d-optilab/3rdparty/gmsh

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
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY OPTIONAL FILES
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/CMakeFiles/CMakeRelink.dir/libagros2d_3dparty_gmsh.so.2.8.4"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/CMakeFiles/CMakeRelink.dir/libagros2d_3dparty_gmsh.so.2.8"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/CMakeFiles/CMakeRelink.dir/libagros2d_3dparty_gmsh.so"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gmsh" TYPE FILE FILES
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/GmshConfig.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/GmshVersion.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/Gmsh.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/Context.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/GmshDefines.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/GmshMessage.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/VertexArray.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/Octree.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/OctreeInternals.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/OS.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/StringUtils.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/OpenFile.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/onelab.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/GmshSocket.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/onelabUtils.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/Numeric.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/GaussIntegration.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/polynomialBasis.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/JacobianBasis.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/bezierBasis.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/fullMatrix.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/simpleFunction.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/cartesian.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/ElementType.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GModel.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GEntity.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GPoint.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GVertex.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GEdge.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GFace.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GRegion.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GEdgeLoop.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GEdgeCompound.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GFaceCompound.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GRegionCompound.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/GRbf.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MVertex.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MVertexBoundaryLayerData.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MEdge.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MFace.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MElement.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MElementOctree.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MPoint.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MLine.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MTriangle.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MQuadrangle.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MTetrahedron.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MHexahedron.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MPrism.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MPyramid.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MElementCut.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/MElementOctree.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/discreteVertex.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/discreteEdge.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/discreteFace.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/discreteRegion.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/SPoint2.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/SPoint3.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/SVector3.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/STensor3.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/SBoundingBox3d.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/Pair.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/Range.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/SOrientedBoundingBox.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/CellComplex.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/ChainComplex.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/Cell.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/Homology.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/Chain.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/partitionEdge.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/CGNSOptions.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/gmshLevelset.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/boundaryLayersData.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshGEdge.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshGFace.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshGFaceOptimize.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshGFaceElliptic.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshPartition.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshGFaceDelaunayInsertion.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/simple3D.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshPartitionOptions.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/directions3D.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/yamakawa.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/Voronoi3D.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/Levy3D.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/periodical.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/meshMetric.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/mathEvaluator.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/dofManager.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/femTerm.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/laplaceTerm.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/elasticityTerm.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/crossConfTerm.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/orthogonalTerm.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/linearSystem.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/linearSystemGMM.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/linearSystemCSR.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/linearSystemFull.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/elasticitySolver.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/sparsityPattern.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/groupOfElements.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/linearSystemPETSc.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Solver/linearSystemMUMPS.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/PView.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/PViewData.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Plugin/PluginManager.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/OctreePost.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/PViewDataList.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/PViewDataGModel.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/PViewOptions.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Post/ColorTable.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/nodalBasis.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Graphics/drawContext.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/kbipack/gmp_normal_form.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/kbipack/gmp_matrix.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/kbipack/gmp_blas.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/kbipack/mpz.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/DiscreteIntegration/Integration3D.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/HighOrderMeshOptimizer/OptHOM.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/HighOrderMeshOptimizer/OptHomMesh.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/HighOrderMeshOptimizer/OptHomRun.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/HighOrderMeshOptimizer/ParamCoord.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/HighOrderMeshOptimizer/OptHomFastCurving.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/HighOrderMeshOptimizer/SuperEl.h"
    "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/MathEx/mathex.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/gmsh" TYPE FILE RENAME "README.txt" FILES "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/doc/WELCOME.txt")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/gmsh" TYPE FILE FILES "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/doc/LICENSE.txt")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/gmsh" TYPE FILE FILES "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/doc/CREDITS.txt")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/doc/gmsh.1")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/gmsh" TYPE FILE OPTIONAL FILES "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/doc/texinfo/gmsh.html")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/gmsh" TYPE FILE OPTIONAL FILES "/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/doc/texinfo/gmsh.pdf")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Common/cmake_install.cmake")
  INCLUDE("/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Numeric/cmake_install.cmake")
  INCLUDE("/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Geo/cmake_install.cmake")
  INCLUDE("/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/Mesh/cmake_install.cmake")
  INCLUDE("/home/karban/Projects/agros2d-optilab/3rdparty/gmsh/contrib/blossom/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

