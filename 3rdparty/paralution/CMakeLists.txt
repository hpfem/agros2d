PROJECT(agros2d_3dparty_paralution)

IF(WITH_PARALUTION_OPENCL)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} SUPPORT_OCL")
ENDIF(WITH_PARALUTION_OPENCL)

SET(SOURCES src/base/backend_manager.cpp 
    src/base/base_paralution.cpp 
    src/base/global_matrix.cpp 
    src/base/global_vector.cpp 
    src/base/local_stencil.cpp 
    src/base/operator.cpp 
    src/base/base_matrix.cpp 
    src/base/base_vector.cpp 
    src/base/global_stencil.cpp 
    src/base/local_matrix.cpp 
    src/base/local_vector.cpp 
    src/base/vector.cpp 
    src/base/host/host_conversion.cpp 
    src/base/host/host_matrix_coo.cpp 
    src/base/host/host_matrix_dense.cpp 
    src/base/host/host_matrix_ell.cpp 
    src/base/host/host_matrix_mcsr.cpp 
    src/base/host/host_matrix_bcsr.cpp 
    src/base/host/host_matrix_csr.cpp 
    src/base/host/host_matrix_dia.cpp 
    src/base/host/host_matrix_hyb.cpp 
    src/base/host/host_vector.cpp 
    src/solvers/chebyshev.cpp 
    src/solvers/iter_ctrl.cpp 
    src/solvers/mixed_precision.cpp 
    src/solvers/solver.cpp 
    src/solvers/krylov/bicgstab.cpp 
    src/solvers/krylov/cg.cpp 
    src/solvers/krylov/gmres.cpp 
    src/solvers/multigrid/multigrid_amg.cpp 
    src/solvers/multigrid/multigrid.cpp 
    src/solvers/preconditioners/preconditioner.cpp 
    src/solvers/preconditioners/preconditioner_multicolored.cpp 
    src/solvers/preconditioners/preconditioner_multicolored_gs.cpp 
    src/solvers/preconditioners/preconditioner_multicolored_ilu.cpp 
    src/solvers/preconditioners/preconditioner_multielimination.cpp 
    src/utils/allocate_free.cpp 
    thirdparty/matrix-market/mmio.c
  )
  
  IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  IF(WITH_PARALUTION_OPENCL)
    SET(SOURCES ${SOURCES} src/base/ocl/ocl_allocate_free.cpp 
            src/base/ocl/backend_ocl.cpp 
            src/base/ocl/ocl_vector.cpp 
            src/base/ocl/ocl_matrix_csr.cpp 
            src/base/ocl/ocl_matrix_coo.cpp 
            src/base/ocl/ocl_matrix_dense.cpp 
            src/base/ocl/ocl_matrix_mcsr.cpp 
            src/base/ocl/ocl_matrix_bcsr.cpp 
            src/base/ocl/ocl_matrix_ell.cpp 
            src/base/ocl/ocl_matrix_dia.cpp 
            src/base/ocl/ocl_matrix_hyb.cpp
        )
    ENDIF(WITH_PARALUTION_OPENCL)
  ENDIF()

INCLUDE_DIRECTORIES(src)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
INSTALL(TARGETS ${PROJECT_NAME}
				RUNTIME DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				LIBRARY DESTINATION ${CMAKE_HOME_DIRECTORY}/libs
				ARCHIVE DESTINATION ${CMAKE_HOME_DIRECTORY}/libs)