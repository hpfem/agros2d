QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib
CONFIG += staticlib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += src/base/backend_manager.cpp \
    src/base/base_paralution.cpp \
    src/base/global_matrix.cpp \
    src/base/global_vector.cpp \
    src/base/local_stencil.cpp \
    src/base/operator.cpp \
    src/base/base_matrix.cpp \
    src/base/base_vector.cpp \
    src/base/global_stencil.cpp \
    src/base/local_matrix.cpp \
    src/base/local_vector.cpp \
    src/base/vector.cpp \
    src/base/host/host_conversion.cpp \
    src/base/host/host_matrix_coo.cpp \
    src/base/host/host_matrix_dense.cpp \
    src/base/host/host_matrix_ell.cpp \
    src/base/host/host_matrix_mcsr.cpp \
    src/base/host/host_matrix_bcsr.cpp \
    src/base/host/host_matrix_csr.cpp \
    src/base/host/host_matrix_dia.cpp \
    src/base/host/host_matrix_hyb.cpp \
    src/base/host/host_vector.cpp \
    src/solvers/chebyshev.cpp \
    src/solvers/iter_ctrl.cpp \
    src/solvers/mixed_precision.cpp \
    src/solvers/solver.cpp \
    src/solvers/krylov/bicgstab.cpp \
    src/solvers/krylov/cg.cpp \
    src/solvers/krylov/gmres.cpp \
    src/solvers/multigrid/multigrid_amg.cpp \
    src/solvers/multigrid/multigrid.cpp \
    src/solvers/preconditioners/preconditioner.cpp \
    src/solvers/preconditioners/preconditioner_multicolored.cpp \
    src/solvers/preconditioners/preconditioner_multicolored_gs.cpp \
    src/solvers/preconditioners/preconditioner_multicolored_ilu.cpp \
    src/solvers/preconditioners/preconditioner_multielimination.cpp \
    src/utils/allocate_free.cpp \
    thirdparty/matrix-market/mmio.c

linux-g++|linux-g++-64|linux-g++-32 {
    QMAKE_LFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp

    INCLUDEPATH += /usr/include/google
}

linux-clang {
    INCLUDEPATH += omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty_paralution
}

macx-g++ {
    TARGET = ../libs/agros_3rdparty_paralution
}

win32-msvc2010 {
    TARGET = ../../libs/agros_3rdparty_paralution

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd


    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include
}

macx-g++ {
}
