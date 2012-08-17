TEMPLATE = subdirs
SUBDIRS = plugins

linux-g++|linux-g++-64|linux-g++-32 {
    system(python ./xml_parser.py)
}

HEADERS      += plugins.h
