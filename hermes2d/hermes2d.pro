QT -= GUI
TARGET = lib/hermes2d
TEMPLATE = lib
OBJECTS_DIR = build
CONFIG = += staticlib
DEFINES += NOGLUT
DEFINES += WITH_UMFPACK

INCLUDEPATH += src \
        src/compat \
        ../hermes_common

SOURCES +=  ../hermes_common/callstack.cpp \
            ../hermes_common/common.cpp \
            ../hermes_common/common_time_period.cpp \
            ../hermes_common/error.cpp \
            ../hermes_common/hermes_logging.cpp \
            ../hermes_common/logging.cpp \
            ../hermes_common/matrix.cpp \
            ../hermes_common/Teuchos_stacktrace.cpp \
            ../hermes_common/Teuchos_stacktrace.hpp \
            ../hermes_common/trace.cpp \
            ../hermes_common/utils.cpp \
            ../hermes_common/solver/amesos.cpp \
            ../hermes_common/solver/aztecoo.cpp \
            ../hermes_common/solver/epetra.cpp \
            ../hermes_common/solver/mumps.cpp \
            ../hermes_common/solver/nox.cpp \
            ../hermes_common/solver/pardiso.cpp \
            ../hermes_common/solver/petsc.cpp \
            ../hermes_common/solver/precond_ifpack.cpp \
            ../hermes_common/solver/precond_ml.cpp \
            ../hermes_common/solver/superlu.cpp \
            ../hermes_common/solver/umfpack_solver.cpp \
            src/curved.cpp \
            src/exodusii.cpp \
            src/graph.cpp \
            src/h2d_common.cpp \
            src/linear_data.cpp \
            src/linear3.cpp \
            src/mesh_lexer.cpp \
            src/mesh_parser.cpp \
            src/numerical_flux.cpp \
            src/qsort.cpp \
            src/refmap.cpp \
            src/trans.cpp \
            src/weakform.cpp \
            src/discrete_problem.cpp \
            src/filter.cpp \
            src/hash.cpp \
            src/h2d_reader.cpp \
            src/linear1.cpp \
            src/mesh.cpp \
            src/neighbor.cpp \
            src/ogprojection.cpp \
            src/quad_std.cpp \
            src/regul.cpp \
            src/transform.cpp \
            src/element_to_refine.cpp \
            src/forms.cpp \
            src/hermes2d.cpp \
            src/limit_order.cpp \
            src/linear2.cpp \
            src/norm.cpp \
            src/precalc.cpp \
            src/refinement_type.cpp \
            src/solution.cpp \
            src/traverse.cpp \
            src/adapt/adapt.cpp \
            src/views/base_view.cpp \
            src/views/mesh_view.cpp \
            src/views/order_view.cpp \
            src/views/scalar_view.cpp \
            src/views/stream_view.cpp \
            src/views/vector_base_view.cpp \
            src/views/vector_view.cpp \
            src/views/view.cpp \
            src/views/view_data.cpp \
            src/views/view_support.cpp \
            src/space/space.cpp \
            src/space/space_hcurl.cpp \
            src/space/space_hdiv.cpp \
            src/space/space_h1.cpp \
            src/space/space_l2.cpp \
            src/shapeset/shapeset.cpp \
            # src/shapeset/shapeset_h1_eigen.cpp \
            src/shapeset/shapeset_h1_ortho.cpp \
            src/shapeset/shapeset_h1_jacobi.cpp \
            src/shapeset/shapeset_h1_quad.cpp \
            src/shapeset/shapeset_l2_legendre.cpp \
            # src/shapeset/shapeset_hc_eigen2.cpp \
            # src/shapeset/shapeset_hc_gradleg.cpp \
            # src/shapeset/shapeset_hc_gradeigen.cpp \
            # src/shapeset/shapeset_hc_legendre.cpp \
            # src/shapeset/shapeset_hd_legendre.cpp \
            src/ref_selectors/hcurl_proj_based_selector.cpp \
            src/ref_selectors/h1_proj_based_selector.cpp \
            src/ref_selectors/l2_proj_based_selector.cpp \
            src/ref_selectors/optimum_selector.cpp \
            src/ref_selectors/order_permutator.cpp \
            src/ref_selectors/proj_based_selector.cpp \
            src/ref_selectors/selector.cpp \
            src/gen/gen_hc_gradeigen.cpp \
            src/gen/gen_hc_gradleg.cpp \
            src/gen/gen_hc_gradleg_triang.cpp \
            src/gen/gen_hc_leg.cpp \
            src/gen/gen_hdiv_leg.cpp \
            src/gen/gen_h1_simple_quad.cpp \
            src/gen/gen_l2_leg_quad.cpp \
            src/gen/gen_l2_leg_tri.cpp

HEADERS = += src/common.h

linux-g++ {
    DEFINES += WITH_MUMPS
    DEFINES += WITH_SUPERLU

    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/superlu
    LIBS += -lumfpack
    LIBS += -ldmumps_seq
    LIBS += -lsuperlu
}
