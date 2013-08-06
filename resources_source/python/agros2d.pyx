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

# Parametrs class
class Parameters(dict):
    def __init__(self, get_method, set_method):
        self.get = get_method
        self.set = set_method
        dict.__init__(self, self.get())

    def __getitem__(self, key):
        parameters = self.get()
        for parameters_key in parameters:
            dict.__setitem__(self, parameters_key, parameters[parameters_key])

        return dict.__getitem__(self, key)

    def __setitem__(self, key, value):
        if (not self.has_key(key)):
            raise KeyError("Invalid key. Valid keys: {0}".format(self.keys()))

        dict.__setitem__(self, key, value)
        self.set(dict(self))

# test functions
def value_in_range(value, min, max, key):
    if (value < min or value > max):
        raise IndexError("Value of '{0}' is out of range ({1} - {2})".format(key, min, max))

def value_in_list(value, list, key):
    for item in list:
        if (value == item):
            return

    raise KeyError("Key '{0}' is invalid. Valid keys: {1}".format(list, key))

def positive_value(value, key):
    if (value < 0):
        raise IndexError("Value of {0} must be possitive.".format(key))

# convert functions
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
    # open and save
    void openFile(string &file) except +
    void saveFile(string &file, bool saveWithSolution) except +

    # memory
    int appTime()
    void memoryUsage(vector[int] &time, vector[int] &usage)

    # PyOptions
    cdef cppclass PyOptions:
        int getNumberOfThreads()
        void setNumberOfThreads(int threads) except +

        int getCacheSize()
        void setCacheSize(int size) except +

        bool getSolverCache()
        void setSolverCache(bool cache)

        bool getSaveMatrixRHS()
        void setSaveMatrixRHS(bool save)
        
        string getDumpFormat()
        void setDumpFormat(string format) except +

def open_file(file):
    openFile(string(file))

def save_file(file, save_with_solution = False):
    saveFile(string(file), save_with_solution)

def app_time():
    return appTime()

def memory_usage():
    cdef vector[int] time_vector
    cdef vector[int] usage_vector
    memoryUsage(time_vector, usage_vector)

    time = list()
    usage = list()
    for i in range(time_vector.size()):
        time.append(time_vector[i])
        usage.append(usage_vector[i])

    return time, usage

cdef class __Options__:
    cdef PyOptions *thisptr

    def __cinit__(self):
        self.thisptr = new PyOptions()
    def __dealloc__(self):
        del self.thisptr

    property number_of_threads:
        def __get__(self):
            return self.thisptr.getNumberOfThreads()
        def __set__(self, threads):
            self.thisptr.setNumberOfThreads(threads)

    property cache_size:
        def __get__(self):
            return self.thisptr.getCacheSize()
        def __set__(self, size):
            self.thisptr.setCacheSize(size)

    property solver_cache:
        def __get__(self):
            return self.thisptr.getSolverCache()
        def __set__(self, cache):
            self.thisptr.setSolverCache(cache)

    property save_matrix_and_rhs:
        def __get__(self):
            return self.thisptr.getSaveMatrixRHS()
        def __set__(self, save):
            self.thisptr.setSaveMatrixRHS(save)
       
    property dump_format:
        def __get__(self):
            return self.thisptr.getDumpFormat()
        def __set__(self, format):
            self.thisptr.setDumpFormat(format)

options = __Options__()
