include "pyproblem.pxi"
include "pyfield.pxi"
include "pygeometry.pxi"
include "pyview.pxi"
include "pyparticletracing.pxi"

cdef extern from "../../src/pythonlab/pythonengine_agros.h":
    void pyOpenDocument(char *str) except +
    void pySaveDocument(char *str) except +

def open_document(char *str):
    pyOpenDocument(str)

def save_document(char *str):
    pySaveDocument(str)
