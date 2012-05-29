QT -= GUI
TARGET = build/lib/hermes2d
OBJECTS_DIR = build
TEMPLATE = lib
CONFIG += staticlib
DEFINES += NOGLUT
DEFINES += WITH_UMFPACK

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
               include/refinement_selectors \
               include/shapeset \
               include/space \
               include/views \
               include/weakform \
               include/weakform_library \
               ../hermes_common \
               ../hermes_common/include/ \
               ../hermes_common/include/solvers/ \
               ../hermes_common/include/third_party_codes/trilinos-teuchos/

SOURCES +=  ../hermes_common/src/hermes_logging.cpp \
                ../hermes_common/src/common_time_period.cpp \
                ../hermes_common/src/callstack.cpp \
                ../hermes_common/src/error.cpp \
                ../hermes_common/src/matrix.cpp \
                ../hermes_common/src/tables.cpp \
                ../hermes_common/src/qsort.cpp \
                ../hermes_common/src/c99_functions.cpp \
                ../hermes_common/src/ord.cpp \
                ../hermes_common/src/hermes_function.cpp \
                ../hermes_common/src/exceptions.cpp \
                ../hermes_common/src/third_party_codes/trilinos-teuchos/Teuchos_stacktrace.cpp \
                ../hermes_common/src/solvers/dp_interface.cpp \
                ../hermes_common/src/solvers/linear_solver.cpp \
                ../hermes_common/src/solvers/nonlinear_solver.cpp \
                ../hermes_common/src/solvers/epetra.cpp \
                ../hermes_common/src/solvers/aztecoo_solver.cpp \
                ../hermes_common/src/solvers/amesos_solver.cpp \
                ../hermes_common/src/solvers/mumps_solver.cpp \
                ../hermes_common/src/solvers/superlu_solver.cpp \
                ../hermes_common/src/solvers/superlu_real.cpp \
                ../hermes_common/src/solvers/superlu_cplx.cpp \
                ../hermes_common/src/solvers/petsc_solver.cpp \
                ../hermes_common/src/solvers/umfpack_solver.cpp \
                ../hermes_common/src/solvers/precond_ml.cpp \
                ../hermes_common/src/solvers/precond_ifpack.cpp \
                src/forms.cpp \
                src/asmlist.cpp \
                src/newton_solver.cpp \
                src/picard_solver.cpp \
                src/adapt/adapt.cpp \
                # src/adapt/kelly_type_adapt.cpp \
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
                src/mesh/refinement_type.cpp \
                src/mesh/traverse.cpp \
                src/mesh/element_to_refine.cpp \
                src/mesh/mesh.cpp \
                src/mesh/refmap.cpp \
                src/quadrature/limit_order.cpp \
                src/quadrature/quad_std.cpp \
                src/projections/localprojection.cpp \
                src/projections/ogprojection.cpp \
                src/projections/ogprojection_nox.cpp \
                src/refinement_selectors/selector.cpp  \
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
                src/views/vectorizer.cpp \
                src/weakform/weakform.cpp  \
                src/neighbor.cpp \
                src/graph.cpp \
                src/discrete_problem.cpp \
                src/runge_kutta.cpp \
                src/spline.cpp \
                src/global.cpp

HEADERS += \
    ../hermes_common/include/config.h

linux-g++|linux-g++-64|linux-g++-32 {
    QMAKE_LFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp

    # DEFINES += WITH_MUMPS
    # DEFINES += WITH_SUPERLU
    DEFINES += HAVE_FMEMOPEN
    DEFINES += HAVE_LOG2

    INCLUDEPATH += /usr/include/suitesparse
    INCLUDEPATH += /usr/include/python2.6
    INCLUDEPATH += /usr/include/python2.7
    LIBS += -lumfpack
    LIBS += -lxerces-c
    LIBS += -lstdc++
    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")

    # mumps
    contains(CONFIG, WITH_MUMPS) {
        DEFINES += WITH_MUMPS
        LIBS += -ldmumps_seq
        LIBS += -lzmumps_seq
    }
    # superlu
    contains(CONFIG, WITH_SUPERLU) {
        DEFINES += WITH_SUPERLU
        INCLUDEPATH += /usr/include/superlu
        LIBS += -lsuperlu
    }
}

macx-g++ {
    DEFINES += HAVE_FMEMOPEN
    DEFINES += HAVE_LOG2

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

win32-msvc2010 {
    # QMAKE_LFLAGS += /MD /openmp
    QMAKE_CXXFLAGS += /MD /MP /openmp /Zc:wchar_t

    DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += XML_LIBRARY
    DEFINES += WIN32
    DEFINES += IMPLEMENT_C99
    DEFINES += "finite=_finite"
    DEFINES += "popen=_popen"
    DEFINES += "M_LN2=0.69314718055994530942"
	
    INCLUDEPATH += c:/Python27/include
    INCLUDEPATH += C:/Python27/Lib/site-packages/numpy/core/include
	INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -Lc:/Python27
    LIBS += -Lc:/Qt/4.8.2/lib
    LIBS += -lumfpack
    LIBS += -lamd
    LIBS += -lblas
    LIBS += -lpthread
    LIBS += -lpython27
    LIBS += -lvcomp
    LIBS += -lxerces-c_static_3
    LIBS += -ladvapi32
    LIBS += -lws2_32
}
