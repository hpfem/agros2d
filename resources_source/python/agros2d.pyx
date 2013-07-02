# imports
from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.vector cimport vector
from libcpp.map cimport map

from cython.operator cimport preincrement as incr
from cython.operator cimport dereference as deref

cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "<string>" namespace "std":
    cdef cppclass string:
        string()
        string(char *)
        char * c_str()

# global functions
cdef vector[int] list_to_int_vector(list):
    cdef vector[int] int_vector
    for item in list:
        int_vector.push_back(item)

    return int_vector

cdef object double_vector_to_list(vector[double] vector):
    out = list()
    for i in range(vector.size()):
        out.append(vector[i])

    return out

cdef map[string, int] dictionary_to_int_map(dictionary):
    cdef map[string, int] int_map
    cdef pair[string, int] row
    for key in dictionary:
        row.first = string(key)
        row.second = dictionary[key]
        int_map.insert(row)

    return int_map

cdef map[string, string] dictionary_to_string_map(dictionary):
    cdef map[string, string] string_map
    cdef pair[string, string] row
    for key in dictionary:
        row.first = string(key)
        row.second = string(dictionary[key])
        string_map.insert(row)

    return string_map

# wrappers
include "pyproblem.pxi"
include "pyfield.pxi"
include "pygeometry.pxi"
include "pyview.pxi"
include "pyparticletracing.pxi"

cdef extern from "../../agros2d-library/pythonlab/pythonengine_agros.h":
    void openFile(string &file) except +
    void saveFile(string &file, bool saveWithSolution) except +

def open_file(file):
    openFile(string(file))

def save_file(file, save_with_solution = False):
    saveFile(string(file), save_with_solution)
