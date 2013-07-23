# agros2d - hp-FEM multiphysics application based on Hermes2D library
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include
INCLUDEPATH += ../util
INCLUDEPATH += ../agros2d-library

LIBS += -lagros_pythonlab_library
LIBS += -lagros2d_library
LIBS += -lagros2d_hermes2d

HEADERS += generator.h \
    generator_module.h \
    generator_coupling.h

SOURCES += main.cpp \
    generator.cpp \
    generator_module.cpp \
    generator_coupling.cpp

include(../agros2d.pri)

linux-clang {
    INCLUDEPATH += ../hermes2d/omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../agros2d_generator
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

win32-msvc2010|win32-msvc2012 {
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    CONFIG += console

    SOURCES      += ../resources_source/classes/module_xml.cpp
    HEADERS      += ../resources_source/classes/module_xml.h
    
    SOURCES      += ../resources_source/classes/coupling_xml.cpp
    HEADERS      += ../resources_source/classes/coupling_xml.h
    
    DEFINES += XERCES_STATIC_LIBRARY
    DEFINES += _WINDOWS
    
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

