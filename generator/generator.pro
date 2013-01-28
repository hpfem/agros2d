# agros2d - hp-FEM multiphysics application based on Hermes2D library
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3rdparty
LIBS += -lagros2d_library

HEADERS += generator.h \
    generator_module.h \
    generator_coupling.h

SOURCES += main.cpp \
    generator.cpp \
    generator_module.cpp \
    generator_coupling.cpp

include(../agros2d.pri)

linux-g++|linux-g++-64|linux-g++-32 {
    TARGET = ../agros2d_generator
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

win32-msvc2010 {
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt

    LIBS += -L..
    TARGET = ../../agros2d_generator
}

OTHER_FILES += \
    ../resources/templates/generator/filter_cpp.tpl \
    ../resources/templates/generator/filter_h.tpl \
    ../resources/templates/generator/force_cpp.tpl \
    ../resources/templates/generator/force_h.tpl \
    ../resources/templates/generator/localvalue_cpp.tpl \
    ../resources/templates/generator/localvalue_h.tpl \
    ../resources/templates/generator/surfaceintegral_cpp.tpl \
    ../resources/templates/generator/surfaceintegral_h.tpl \
    ../resources/templates/generator/volumeintegral_cpp.tpl \
    ../resources/templates/generator/volumeintegral_h.tpl \
    ../resources/templates/generator/interface_cpp.tpl \
    ../resources/templates/generator/interface_h.tpl \
    ../resources/templates/generator/module_pro.tpl \
    ../resources/templates/generator/weakform_cpp.tpl \
    ../resources/templates/generator/weakform_h.tpl \
    ../resources/templates/generator/module_pro.tpl \
    ../resources/templates/generator/plugins_pro.tpl \
    ../resources/templates/generator/coupling_interface_h.tpl \
    ../resources/templates/generator/coupling_interface_cpp.tpl \
    ../resources/templates/generator/coupling_pro.tpl \
    ../resources/templates/generator/doc_index.tpl

