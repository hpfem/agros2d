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

linux-g++ {
    # use qmake PREFIX=... to customize your installation
    isEmpty(PREFIX):PREFIX = /usr/local

    # install examples
    examples.path = $${PREFIX}/share/agros2d/data
    examples.files = data/*.a2d

    # install script
    script.path = $${PREFIX}/share/agros2d/data/script
    script.files = data/script/*.py

    # install help
    help.path = $${PREFIX}/share/agros2d/resources/help
    help.files = resources/help/*

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
    target.path = $${PREFIX}/bin
    target.files = agros2d

    # install libraries
    script.path = $${PREFIX}/lib/
    script.files = libs/*

    # "make install" configuration options
    INSTALLS *= target \
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


