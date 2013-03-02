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

linux-g++|linux-g++-64|linux-g++-32 {
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
    pixmap.files = src/images/agros2d.xpm

    # install desktop
    desktop.path = $${PREFIX}/share/applications
    desktop.files = agros2d.desktop

    # libs
    libs_3rdparty.path = $${PREFIX}/lib
    libs_3rdparty.files = libs/libagros2d_3rdparty.so
    libs_hermes2d.path = $${PREFIX}/lib
    libs_hermes2d.files = libs/libagros2d_hermes2d.so
    libs_library.path = $${PREFIX}/lib
    libs_library.files = libs/libagros2d_library.so
    libs_plugins.path = $${PREFIX}/lib
    libs_plugins.files = libs/libagros2d_plugin_*.so

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
        libs_3rdparty \
        libs_hermes2d \
        libs_library \
        libs_plugins \
        resources \
        python_startup \
        script_examples \
        pixmap \
        desktop
}

OTHER_FILES += \
    agros2d.supp


