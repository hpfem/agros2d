QT -= GUI

OBJECTS_DIR = build

TEMPLATE = lib

DEFINES += NOGLUT
DEFINES += WITH_UMFPACK
DEFINES += H2D_DISABLE_MULTIMESH_TESTS

INCLUDEPATH += ../3rdparty/bson

# bson
DEFINES += WITH_BSON

# paralution
DEFINES += WITH_PARALUTION
INCLUDEPATH += ../3rdparty/paralution/src

# mumps
DEFINES += WITH_MUMPS

LIBS += -L../libs
LIBS += -L../3rdparty/libs
LIBS += -lagros_3rdparty_paralution
LIBS += -lagros_3rdparty_bson

# set 'HERMES_DEBUG' in qtcreator
contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

INCLUDEPATH += include \
               include/adapt \
               include/boundary_conditions \
               include/function \
               include/integrals \
               include/mesh \
               include/quadrature \
               include/projections \
               include/refinement_selectors \
               include/shapeset \
               include/space \
               include/views \
               include/weakform \
               ../hermes_common \
               ../hermes_common/include/ \
               ../hermes_common/include/solvers/ \

SOURCES +=      ../hermes_common/src/api.cpp \
                ../hermes_common/src/callstack.cpp \
                ../hermes_common/src/matrix.cpp \
                ../hermes_common/src/range.cpp \
                ../hermes_common/src/table.cpp \
                ../hermes_common/src/qsort.cpp \
                ../hermes_common/src/c99_functions.cpp \
                ../hermes_common/src/ord.cpp \
                ../hermes_common/src/hermes_function.cpp \
                ../hermes_common/src/exceptions.cpp \
                ../hermes_common/src/mixins.cpp \
                ../hermes_common/src/cs_matrix.cpp \
                ../hermes_common/src/solvers/linear_matrix_solver.cpp \
                ../hermes_common/src/solvers/epetra.cpp \
                ../hermes_common/src/solvers/aztecoo_solver.cpp \
                ../hermes_common/src/solvers/amesos_solver.cpp \
                ../hermes_common/src/solvers/mumps_solver.cpp \
                ../hermes_common/src/solvers/paralution_solver.cpp \
                ../hermes_common/src/solvers/superlu_solver.cpp \
                ../hermes_common/src/solvers/superlu_solver_real.cpp \
                ../hermes_common/src/solvers/superlu_solver_cplx.cpp \
                ../hermes_common/src/solvers/petsc_solver.cpp \
                ../hermes_common/src/solvers/umfpack_solver.cpp \
                ../hermes_common/src/solvers/precond_ml.cpp \
                ../hermes_common/src/solvers/precond_ifpack.cpp \
                src/api2d.cpp \
                src/mixins2d.cpp \
                src/forms.cpp \
                src/asmlist.cpp \
                src/solver/linear_solver.cpp \
                src/solver/nonlinear_solver.cpp \
                src/solver/newton_solver.cpp \
                src/solver/newton_solver_convergence_measurement.cpp \
                src/solver/picard_solver.cpp \
                src/solver/runge_kutta.cpp \
                src/solver/solver.cpp \
                src/adapt/adapt.cpp \
                src/adapt/error_calculator.cpp \
                src/adapt/error_thread_calculator.cpp \
                src/boundary_conditions/essential_boundary_conditions.cpp \
                src/function/transformable.cpp \
                src/function/function.cpp \
                src/function/mesh_function.cpp \
                src/function/exact_solution.cpp \
                src/function/solution.cpp  \
                src/function/filter.cpp \
                src/function/solution_h2d_xml.cpp \
                src/mesh/curved.cpp \
                src/mesh/hash.cpp \
                src/mesh/mesh_data.cpp \
                src/mesh/mesh_reader_exodusii.cpp \
                src/mesh/mesh_h1d_xml.cpp \
                src/mesh/mesh_h2d_xml.cpp \
                src/mesh/mesh_reader_h1d_xml.cpp \
                src/mesh/mesh_reader_h2d_xml.cpp \
                src/mesh/mesh_reader_h2d.cpp \
                src/mesh/subdomains_h2d_xml.cpp \
                src/mesh/mesh_reader_h2d_bson.cpp \
                src/mesh/traverse.cpp \
                src/mesh/mesh.cpp \
                src/mesh/refmap.cpp \
                src/mesh/mesh_util.cpp \
                src/mesh/element.cpp \
                src/quadrature/limit_order.cpp \
                src/quadrature/quad_std.cpp \
                src/projections/ogprojection.cpp \
                src/projections/ogprojection_nox.cpp \
                src/refinement_selectors/selector.cpp  \
                src/refinement_selectors/candidates.cpp \
                src/refinement_selectors/element_to_refine.cpp \
                src/refinement_selectors/order_permutator.cpp  \
                src/refinement_selectors/optimum_selector.cpp  \
                src/refinement_selectors/proj_based_selector.cpp  \
                src/refinement_selectors/l2_proj_based_selector.cpp  \
                src/refinement_selectors/h1_proj_based_selector.cpp  \
                src/refinement_selectors/hcurl_proj_based_selector.cpp \
                src/shapeset/shapeset.cpp  \
                src/shapeset/shapeset_h1_ortho.cpp  \
                src/shapeset/shapeset_h1_jacobi.cpp  \
                src/shapeset/shapeset_h1_quad.cpp \
                src/shapeset/shapeset_hc_legendre.cpp  \
                src/shapeset/shapeset_hc_gradleg.cpp \
                src/shapeset/shapeset_hd_legendre.cpp \
                src/shapeset/shapeset_l2_legendre.cpp \
                src/shapeset/precalc.cpp  \
                src/space/space.cpp  \
                src/space/space_h1.cpp  \
                src/space/space_hcurl.cpp  \
                src/space/space_l2.cpp \
                src/space/space_hdiv.cpp \
                src/space/space_h2d_xml.cpp \
                src/views/view.cpp  \
                src/views/view_data.cpp  \
                src/views/view_support.cpp \
                src/views/linearizer_base.cpp \
                src/views/linearizer.cpp \
                src/views/orderizer.cpp \
                src/views/orderizer_quad.cpp \
                src/views/vectorizer.cpp \
                src/weakform/weakform.cpp  \
                src/neighbor_search.cpp \
                src/norm_form.cpp \
                src/graph.cpp \
                src/discrete_problem.cpp \
                src/discrete_problem/discrete_problem_cache.cpp \
                # src/discrete_problem/discrete_problem_form_assembler.cpp \
                src/discrete_problem/discrete_problem_helpers.cpp \
                src/discrete_problem/discrete_problem_integration_order_calculator.cpp \
                src/discrete_problem/discrete_problem_selective_assembler.cpp \
                src/discrete_problem/discrete_problem_thread_assembler.cpp \
                src/discrete_problem/dg/discrete_problem_dg_assembler.cpp \
                src/discrete_problem/dg/multimesh_dg_neighbor_tree.cpp \
                src/discrete_problem/dg/multimesh_dg_neighbor_tree_node.cpp \
                src/spline.cpp

HEADERS += \
    ../hermes_common/include/config.h \
    omp/omp.h

linux-g++|linux-g++-64|linux-g++-32 {
    QMAKE_LFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp

    INCLUDEPATH += /usr/include/google
}

linux-clang {
    INCLUDEPATH += omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros2d_hermes2d

    CONFIG += warn_off

    DEFINES += HAVE_FMEMOPEN
    DEFINES += HAVE_LOG2

    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/google
    LIBS += -lumfpack
    LIBS += -lxerces-c
    # LIBS += -ltcmalloc_minimal
    LIBS += -lstdc++

    # mumps
    DEFINES += WITH_MUMPS
    LIBS += -ldmumps_seq
    LIBS += -lzmumps_seq
    # superlu
    contains(CONFIG, WITH_SUPERLU) {
        DEFINES += WITH_SUPERLU
        INCLUDEPATH += /usr/include/superlu
        LIBS += -lsuperlu
    }
    # petsc
    contains(CONFIG, WITH_PETSC) {
        DEFINES += WITH_PETSC
        INCLUDEPATH += /usr/lib/petscdir/3.2/include/
        INCLUDEPATH += /usr/lib/openmpi/include/
    }

    contains(CONFIG, WITH_PARALUTION_OPENCL) {
        LIBS += -lOpenCL
    }
}

macx-g++ {
    TARGET = ../libs/agros2d_hermes2d
    QMAKE_CXXFLAGS += -fpermissive

    DEFINES += HAVE_FMEMOPEN
    DEFINES += HAVE_LOG2
    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -lpthread
    LIBS += -llibtcmalloc_minimal
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}

win32-msvc2010|win32-msvc2012 {
    TARGET = ../../agros2d_hermes2d

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += AGROS
    DEFINES += HERMES_FOR_AGROS
    DEFINES += XML_LIBRARY
    DEFINES += WIN32
    DEFINES += _WINDOWS
    DEFINES += IMPLEMENT_C99
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"
    DEFINES += "M_LN2=0.69314718055994530942"

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += C:/Python27/Lib/site-packages/numpy/core/include
    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -Lc:/hpfem/hermes/dependencies/lib
    LIBS += -Ld:/hpfem/hermes/dependencies/lib
    LIBS += -llibumfpack
    # LIBS += -llibtcmalloc_minimal
    LIBS += -llibamd
    LIBS += -lpthreadVCE2
    LIBS += -lvcomp
    LIBS += -ladvapi32
    LIBS += -lws2_32

    # mumps
    DEFINES += WITH_MUMPS

    LIBS += -ldmumps_c
    LIBS += -ldmumps_fortran
    LIBS += -lzmumps_c
    LIBS += -lzmumps_fortran
    LIBS += -lmumps_common_c
    LIBS += -llibseq_c
    LIBS += -llibseq_fortran
    LIBS += -lpord_c
    LIBS += -lBLAS
    LIBS += -lsvml_disp
    # superlu
    contains(CONFIG, WITH_SUPERLU) {
        DEFINES += WITH_SUPERLU
        LIBS += -lsuperlu
        LIBS += -llibblas
    }

    CONFIG(release, debug|release) {
        LIBS += -lxerces-c_static_3
    }
    CONFIG(debug, debug|release) {
        LIBS += -lxerces-c_static_3D
    }
}
