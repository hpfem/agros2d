QT -= GUI
TARGET = lib/hermes2d
TEMPLATE = lib
OBJECTS_DIR = build
CONFIG = += staticlib
# CONFIG = += debug
DEFINES += NOGLUT
DEFINES += WITH_UMFPACK

INCLUDEPATH += src \
        src/compat \
        ../hermes_common

SOURCES +=  ../hermes_common/compat/fmemopen.cpp \
            ../hermes_common/compat/c99_functions.cpp \
            ../hermes_common/callstack.cpp \
            ../hermes_common/common.cpp \
            ../hermes_common/common_time_period.cpp \
            ../hermes_common/error.cpp \
            ../hermes_common/hermes_logging.cpp \
            ../hermes_common/logging.cpp \
            ../hermes_common/matrix.cpp \
            ../hermes_common/trace.cpp \
            ../hermes_common/utils.cpp \
            ../hermes_common/tables.cpp \
            ../hermes_common/solver/amesos.cpp \
            ../hermes_common/solver/aztecoo.cpp \
            ../hermes_common/solver/epetra.cpp \
            ../hermes_common/solver/mumps.cpp \
            ../hermes_common/solver/nox.cpp \
            ../hermes_common/solver/petsc.cpp \
            ../hermes_common/solver/precond_ifpack.cpp \
            ../hermes_common/solver/precond_ml.cpp \
            ../hermes_common/solver/superlu.cpp \
            ../hermes_common/solver/umfpack_solver.cpp \
            ../hermes_common/python/python_api.cpp \
            ../hermes_common/python/python_engine.cpp \
            ../hermes_common/third_party_codes/trilinos-teuchos/Teuchos_stacktrace.cpp \
            src/discrete_problem.cpp \
            src/graph.cpp \
            src/hermes2d.cpp \
            src/h2d_common.cpp \
            src/neighbor.cpp \
            src/numerical_flux.cpp \
            src/ogprojection.cpp \
            src/qsort.cpp \
            src/runge_kutta.cpp \
            src/boundaryconditions/essential_bcs.cpp \
            src/adapt/adapt.cpp \
            src/function/filter.cpp \
            src/function/norm.cpp \
            src/function/forms.cpp \
            src/function/solution.cpp \
            src/gen/gen_hc_gradeigen.cpp \
            src/gen/gen_hc_gradleg.cpp \
            src/gen/gen_hc_gradleg_triang.cpp \
            src/gen/gen_hc_leg.cpp \
            src/gen/gen_hdiv_leg.cpp \
            src/gen/gen_h1_simple_quad.cpp \
            src/gen/gen_l2_leg_quad.cpp \
            src/gen/gen_l2_leg_tri.cpp \
            src/linearizer/linear_data.cpp \
            src/linearizer/linear1.cpp \
            src/linearizer/linear2.cpp \
            src/linearizer/linear3.cpp \
            src/mesh/curved.cpp \
            src/mesh/element_to_refine.cpp \
            src/mesh/exodusii.cpp \
            src/mesh/hash.cpp \
            src/mesh/h2d_reader.cpp \
            src/mesh/mesh2d_parser.cpp \
            src/mesh/mesh.cpp \
            src/mesh/python_reader.cpp \
            src/mesh/refinement_type.cpp \
            src/mesh/refmap.cpp \
            src/mesh/regul.cpp \
            src/mesh/trans.cpp \
            src/mesh/transform.cpp \
            src/mesh/traverse.cpp \
            src/quadrature/limit_order.cpp \
            src/quadrature/quad_std.cpp \
            src/shapeset/precalc.cpp \
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
            src/space/space.cpp \
            src/space/space_hcurl.cpp \
            src/space/space_hdiv.cpp \
            src/space/space_h1.cpp \
            src/space/space_l2.cpp \
            src/ref_selectors/hcurl_proj_based_selector.cpp \
            src/ref_selectors/h1_proj_based_selector.cpp \
            src/ref_selectors/l2_proj_based_selector.cpp \
            src/ref_selectors/optimum_selector.cpp \
            src/ref_selectors/order_permutator.cpp \
            src/ref_selectors/proj_based_selector.cpp \
            src/ref_selectors/selector.cpp \
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
            src/weakform/weakform.cpp

HEADERS = += ../hermes_common/src/compat.h

linux-g++|linux-g++-64|linux-g++-32 {
    # DEFINES += WITH_MUMPS
    # DEFINES += WITH_SUPERLU
    DEFINES += HAVE_FMEMOPEN
    DEFINES += HAVE_LOG2

    INCLUDEPATH += /usr/include/suitesparse
    # INCLUDEPATH += /usr/include/superlu
    INCLUDEPATH += /usr/include/python2.6
    INCLUDEPATH += /usr/include/python2.7
    LIBS += -lumfpack
    # LIBS += -ldmumps_seq
    # LIBS += -lsuperlu
    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")
}

win32-msvc2008 {
    DEFINES += WIN32
    DEFINES += IMPLEMENT_C99
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"
    DEFINES += "pclose=_pclose"
    DEFINES += "M_LN2=0.69314718055994530942"

    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += C:/Python27/Lib/site-packages/numpy/core/include
    LIBS += -Lc:/Python27
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lpthread
    LIBS += -lpython27
}

macx-g++|macx-g++42 {
    INCLUDEPATH += /opt/local/include
    INCLUDEPATH += /opt/local/include/ufsparse
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/Current/include/python2.7
    INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/numpy/core/include
    LIBS += -L/opt/local/lib
    LIBS += -L/usr/lib
    LIBS += -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config
    LIBS += -lpthread
    LIBS += -lpython2.7
    LIBS += -lqwt
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
}
