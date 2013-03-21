#ifndef PYTHONLABUTIL_H
#define PYTHONLABUTIL_H

#include "util.h"

// Windows DLL export/import definitions
#ifdef Q_WS_WIN
  // DLL build
  #ifdef AGROS_PYTHONLAB_DLL
    #define AGROS_PYTHONLAB_API __declspec(dllexport)
  // DLL usage
  #else
    #define AGROS_PYTHONLAB_API __declspec(dllimport)
  #endif
#else
  #define AGROS_PYTHONLAB_API
#endif

#endif