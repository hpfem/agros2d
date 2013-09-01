from libcpp.vector cimport vector
from libcpp.map cimport map
from libcpp.pair cimport pair
from libcpp cimport bool
from cython.operator cimport preincrement as incr, dereference as deref

cdef extern from "<string>" namespace "std":
    cdef cppclass string:
        string()
        string(char *)
        char * c_str()

cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "../../pythonlab-library/pythonlab_functions.h":
    char *pyVersion()
    void pyQuit()

    char *pyInput(string &str)
    void pyMessage(string &str)

    string pyDatadir(string &str)
    
    
# version()
def version():
    return pyVersion()
    
# quit()
def quit():
    pyQuit()

# input()
def input(str):
    return pyInput(string(str))

# message()
def message(str):
    pyMessage(string(str))

# datadir()
def datadir(str = ""):
    return pyDatadir(string(str))
