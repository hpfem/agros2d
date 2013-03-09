# agros2d - hp-FEM multiphysics application based on Hermes2D library
SUBDIRS += pythonlab
SUBDIRS += 3rdparty
SUBDIRS += hermes2d
SUBDIRS += src
SUBDIRS += binary
SUBDIRS += solver
SUBDIRS += generator

CONFIG += ordered
TEMPLATE = subdirs

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    # use qmake PREFIX=... to customize your installation
    isEmpty(PREFIX):PREFIX = /usr/local

    # install script examples
    script_examples.path = $${PREFIX}/share/agros2d/data/script
    script_examples.files = data/script/*.py

    # install resources
    resources.path = $${PREFIX}/share/agros2d/resources
    resources.files = resources/*
    resources_python.path = $${PREFIX}/share/agros2d
    resources_python.files = *.py

    # install script files
    python_startup.path = $${PREFIX}/share/agros2d
    python_startup.files = *.py

    # install pixmap
    pixmap.path = $${PREFIX}/share/pixmaps
    pixmap.files = resources/images/agros2d.xpm

    # install desktop
    desktop.path = $${PREFIX}/share/applications
    desktop.files = agros2d.desktop

    # libs
    system(mkdir libs)
    system(touch libs/libagros2d_3rdparty.so)
    system(touch libs/libagros2d_3rdparty.so.1)
    system(touch libs/libagros2d_3rdparty.so.1.0)
    system(touch libs/libagros2d_3rdparty.so.1.0.0)
    system(touch libs/libagros2d_hermes2d.so)
    system(touch libs/libagros2d_hermes2d.so.1)
    system(touch libs/libagros2d_hermes2d.so.1.0)
    system(touch libs/libagros2d_hermes2d.so.1.0.0)
    system(touch libs/libagros2d_library.so)
    system(touch libs/libagros2d_library.so.1)
    system(touch libs/libagros2d_library.so.1.0)
    system(touch libs/libagros2d_library.so.1.0.0)
    system(touch libs/libagros2d_plugin_acoustic.so)
    system(touch libs/libagros2d_plugin_current.so)
    system(touch libs/libagros2d_plugin_elasticity.so)
    system(touch libs/libagros2d_plugin_electrostatic.so)
    system(touch libs/libagros2d_plugin_flow.so)
    system(touch libs/libagros2d_plugin_heat.so)
    system(touch libs/libagros2d_plugin_magnetic.so)
    system(touch libs/libagros2d_plugin_rf.so)
    system(touch libs/libagros2d_plugin_current-heat.so)
    system(touch libs/libagros2d_plugin_heat-elasticity.so)
    system(touch libs/libagros2d_plugin_magnetic-heat.so)
    libs.path = $${PREFIX}/lib
    libs.files = libs/*

    # install binary
    system(touch agros2d)
    target.path = $${PREFIX}/bin
    target.files = agros2d

    system(touch agros2d_solver)
    target_solver.path = $${PREFIX}/bin
    target_solver.files = agros2d_solver

    system(touch agros2d_pythonlab)
    target_pytholab.path = $${PREFIX}/bin
    target_pytholab.files = agros2d_pythonlab

    # "make install" configuration options
    INSTALLS *= target \
        target_solver \
        target_pytholab \
        libs \
        resources \
        python_startup \
        script_examples \
        pixmap \
        desktop
}

OTHER_FILES += \
    agros2d.supp


