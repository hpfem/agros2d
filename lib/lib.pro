QT -= GUI
TARGET = ../build/lib/lib/lib
OBJECTS_DIR = ../build/lib
TEMPLATE = lib
CONFIG = += staticlib

SOURCES += dxflib/dl_writer_ascii.cpp \
    dxflib/dl_dxf.cpp \
    muparser/muParser.cpp \
    muparser/muParserBase.cpp \
    muparser/muParserTest.cpp \
    muparser/muParserTokenReader.cpp \
    muparser/muParserBytecode.cpp \
    muparser/muParserCallback.cpp \
    muparser/muParserError.cpp \
    muparser/muParserInt.cpp \
    gl2ps/gl2ps.c \
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
    ctemplate/template_string.cc

linux-g++|linux-g++-64|linux-g++-32 {
    INCLUDEPATH += ctemplate \
            ctemplate/unix \
            .
}

win32-msvc2008 {
    INCLUDEPATH += ctemplate \
            ctemplate/windows \
            .
}

macx-g++ {
}

