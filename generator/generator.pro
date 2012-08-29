# agros2d - hp-FEM multiphysics application based on Hermes2D library
TARGET = ../agros2d_generator
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3dparty
LIBS += -lagros2d

HEADERS += generator.h

SOURCES += main.cpp \
           generator.cpp

include(../agros2d.pri)

OTHER_FILES += \
    ../resources/templates/generator/filter_cpp.tpl \
    ../resources/templates/generator/filter_h.tpl \
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
    ../resources/templates/generator/weakform_pri.tpl \
    ../resources/templates/generator/module_pro.tpl \
    ../resources/templates/generator/plugins_pro.tpl \
    ../resources/templates/generator/coupling_weakform_h.tpl \
    ../resources/templates/generator/coupling_weakform_cpp.tpl \
    ../resources/templates/generator/coupling_interface_h.tpl \
    ../resources/templates/generator/coupling_interface_cpp.tpl
