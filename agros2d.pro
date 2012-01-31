# agros2d - hp-FEM multiphysics application based on Hermes2D library
SUBDIRS += hermes2d
SUBDIRS += src-remote
SUBDIRS += src

CONFIG += ordered
TEMPLATE = subdirs

linux-g++|linux-g++-64|linux-g++-32 {
    # use qmake PREFIX=... to customize your installation
    isEmpty(PREFIX):PREFIX = /usr/local

    # install resources
    system(rm `find resources/python -name "*.pyc"`)
    resources.path = $${PREFIX}/share/agros2d/resources
    resources.CONFIG = recursive
    resources.files = resources/*

    # install examples
    examples.path = $${PREFIX}/share/agros2d/data
    examples.files = data/*.a2d

    # install script
    script.path = $${PREFIX}/share/agros2d/data/script
    script.files = data/script/*.py

    # install help
    help.path = $${PREFIX}/share/agros2d/doc/help
    help.files = doc/help/*

    # install report
    report.path = $${PREFIX}/share/agros2d/doc/report
    report.files = doc/report/*

    # install translations
    lang.path = $${PREFIX}/share/agros2d/lang
    lang.files = lang/*.qm

    # install script
    script.path = $${PREFIX}/share/agros2d
    script.files = *.py

    # install pixmap
    pixmap.path = $${PREFIX}/share/pixmaps
    pixmap.files = src/images/agros2d.xpm

    # install desktop
    desktop.path = $${PREFIX}/share/applications
    desktop.files = agros2d.desktop

    # install binary
    system(touch agros2d)
    system(touch src-remote/agros2d-remote)
    target.path = $${PREFIX}/bin
    target.files = agros2d
    target-remote.path = $${PREFIX}/bin
    target-remote.files = src-remote/agros2d-remote

    # "make install" configuration options
    INSTALLS *= target \
        resources \
        target-remote \
        examples \
        help \
        lang \
        script \
        pixmap \
        report \
        desktop
}

OTHER_FILES += \
    agros2d.supp
