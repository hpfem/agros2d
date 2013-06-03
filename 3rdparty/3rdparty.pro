QT -= GUI
CONFIG += qt
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = lib

contains(CONFIG, HERMES_DEBUG) {
    CONFIG += debug
}

SOURCES += dxflib/dl_writer_ascii.cpp \
    dxflib/dl_dxf.cpp \
    ctemplate/base/arena.cc \
    ctemplate/htmlparser/htmlparser.cc \
    ctemplate/htmlparser/htmlparser_cpp.h \
    ctemplate/htmlparser/jsparser.cc \
    ctemplate/htmlparser/statemachine.cc \
    ctemplate/per_expand_data.cc \
    ctemplate/template_annotator.cc \
    ctemplate/template_cache.cc \
    ctemplate/template.cc \
    ctemplate/template_dictionary.cc \
    ctemplate/template_modifiers.cc \
    ctemplate/template_namelist.cc \
    ctemplate/template_pathops.cc \
    ctemplate/template_string.cc \
    poly2tri/common/shapes.cc \
    poly2tri/sweep/advancing_front.cc \
    poly2tri/sweep/cdt.cc \
    poly2tri/sweep/sweep.cc \
    poly2tri/sweep/sweep_context.cc \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quaadler32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipnewinfo.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    qcustomplot/qcustomplot.cpp

contains(CONFIG, WITH_PARALUTION) {
SOURCES += paralution/src/base/backend_manager.cpp \
    paralution/src/base/base_paralution.cpp \
    paralution/src/base/global_matrix.cpp \
    paralution/src/base/global_vector.cpp \
    paralution/src/base/local_stencil.cpp \
    paralution/src/base/operator.cpp \
    paralution/src/base/base_matrix.cpp \
    paralution/src/base/base_vector.cpp \
    paralution/src/base/global_stencil.cpp \
    paralution/src/base/local_matrix.cpp \
    paralution/src/base/local_vector.cpp \
    paralution/src/base/vector.cpp \
    paralution/src/base/host/host_conversion.cpp \
    paralution/src/base/host/host_matrix_coo.cpp \
    paralution/src/base/host/host_matrix_dense.cpp \
    paralution/src/base/host/host_matrix_ell.cpp \
    paralution/src/base/host/host_matrix_mcsr.cpp \
    paralution/src/base/host/host_matrix_bcsr.cpp \
    paralution/src/base/host/host_matrix_csr.cpp \
    paralution/src/base/host/host_matrix_dia.cpp \
    paralution/src/base/host/host_matrix_hyb.cpp \
    paralution/src/base/host/host_vector.cpp \
    paralution/src/solvers/chebyshev.cpp \
    paralution/src/solvers/iter_ctrl.cpp \
    paralution/src/solvers/mixed_precision.cpp \
    paralution/src/solvers/solver.cpp \
    paralution/src/solvers/krylov/bicgstab.cpp \
    paralution/src/solvers/krylov/cg.cpp \
    paralution/src/solvers/krylov/gmres.cpp \
    paralution/src/solvers/multigrid/multigrid_amg.cpp \
    paralution/src/solvers/multigrid/multigrid.cpp \
    paralution/src/solvers/preconditioners/preconditioner.cpp \
    paralution/src/solvers/preconditioners/preconditioner_multicolored.cpp \
    paralution/src/solvers/preconditioners/preconditioner_multicolored_gs.cpp \
    paralution/src/solvers/preconditioners/preconditioner_multicolored_ilu.cpp \
    paralution/src/solvers/preconditioners/preconditioner_multielimination.cpp \
    paralution/src/utils/allocate_free.cpp
}

HEADERS += \
    quazip/crypt.h \
    quazip/ioapi.h \
    quazip/JlCompress.h \
    quazip/quaadler32.h \
    quazip/quachecksum32.h \
    quazip/quacrc32.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \
    quazip/quazipdir.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazip_global.h \
    quazip/quazip.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    qcustomplot/qcustomplot.h \
    stb_truetype/stb_truetype.h

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../libs/agros_3rdparty

    INCLUDEPATH += ctemplate \
            ctemplate/linux \
            .
}

macx-g++ {
    CONFIG += staticlib
    TARGET = ../libs/agros_3rdparty
    INCLUDEPATH += ctemplate \
            ctemplate/osx \
            .
}

win32-msvc2010 {
    CONFIG += staticlib
    TARGET = ../../libs/agros_3rdparty

    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    INCLUDEPATH += ctemplate
    INCLUDEPATH += ctemplate/windows
    INCLUDEPATH += c:/hpfem/hermes/dependencies/include
    INCLUDEPATH += d:/hpfem/hermes/dependencies/include

    LIBS += -Lc:/hpfem/hermes/dependencies/lib
    LIBS += -Ld:/hpfem/hermes/dependencies/lib

    SOURCES += ctemplate/windows/port.cc
}

macx-g++ {
}
