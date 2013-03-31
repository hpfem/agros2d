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

cdef extern from "../../agros2d-library/pythonlab/pyfield.h":
    cdef cppclass PyField:
        PyField(char *field_id) except +

        char *fieldId()

        char *getAnalysisType()
        void setAnalysisType(char* analysisType) except +

        int getNumberOfRefinements()
        void setNumberOfRefinements(int numberOfRefinements) except +

        int getPolynomialOrder()
        void setPolynomialOrder(int polynomialOrder) except +

        char *getLinearityType()
        void setLinearityType(char* linearityType) except +

        double getNonlinearTolerance()
        void setNonlinearTolerance(double nonlinearTolerance) except +

        int getNonlinearSteps()
        void setNonlinearSteps(int nonlinearSteps) except +

        double getNewtonDampingCoeff()
        void setNewtonDampingCoeff(double dampingCoeff) except +

        bool getNewtonAutomaticDamping()
        void setNewtonAutomaticDamping(bool automaticDamping)

        int getNewtonDampingNumberToIncrease()
        void setNewtonDampingNumberToIncrease(int dampingNumberToIncrease) except +

        bool getPicardAndersonAcceleration()
        void setPicardAndersonAcceleration(bool acceleration) except +

        double getPicardAndersonBeta()
        void setPicardAndersonBeta(double beta) except +

        int getPicardAndersonNumberOfLastVectors()
        void setPicardAndersonNumberOfLastVectors(int number) except +

        char *getAdaptivityType()
        void setAdaptivityType(char* adaptivityType) except +

        double getAdaptivityTolerance()
        void setAdaptivityTolerance(double adaptivityTolerance) except +

        int getAdaptivitySteps()
        void setAdaptivitySteps(int adaptivitySteps) except +

        int getAdaptivityBackSteps()
        void setAdaptivityBackSteps(int adaptivityBackSteps) except +

        int getAdaptivityRedoneEach()
        void setAdaptivityRedoneEach(int adaptivityRedoneEach) except +

        double getInitialCondition()
        void setInitialCondition(double initialCondition) except +

        double getTimeSkip()
        void setTimeSkip(double timeSkip) except +

        void addBoundary(char*, char*, map[char*, double] parameters, map[char*, char*] expressions) except +
        void modifyBoundary(char*, char*, map[char*, double] parameters, map[char*, char*] expressions) except +
        void removeBoundary(char*)

        void addMaterial(char *id, map[char*, double] parameters, map[char*, char*] expressions, map[char*, vector[double]] nonlin_x, map[char*, vector[double]] nonlin_y) except +
        void modifyMaterial(char* name, map[char*, double] parameters, map[char*, char*] expressions, map[char*, vector[double]] nonlin_x, map[char*, vector[double]] nonlin_y) except +
        void removeMaterial(char* name)

        void solve()

        void localValues(double x, double y, int timeStep, int adaptivityStep, char *solutionType, map[string, double] results) except +
        void surfaceIntegrals(vector[int], int timeStep, int adaptivityStep, char *solutionType, map[string, double] results) except +
        void volumeIntegrals(vector[int], int timeStep, int adaptivityStep, char *solutionType, map[string, double] results) except +

        void initialMeshParameters(map[string , int] parameters) except +
        void solutionMeshParameters(map[string , int] parameters) except +

        void adaptivityInfo(vector[double] &error, vector[int] &dofs) except +

cdef class __Field__:
    cdef PyField *thisptr

    def __cinit__(self, char *field_id):
        self.thisptr = new PyField(field_id)
    def __dealloc__(self):
        del self.thisptr

    # field id
    property field_id:
        def __get__(self):
            return self.thisptr.fieldId()

    # analysis type
    property analysis_type:
        def __get__(self):
            return self.thisptr.getAnalysisType()
        def __set__(self, field_id):
            self.thisptr.setAnalysisType(field_id)

    # number of refinements
    property number_of_refinements:
        def __get__(self):
            return self.thisptr.getNumberOfRefinements()
        def __set__(self, number_of_refinements):
            self.thisptr.setNumberOfRefinements(number_of_refinements)

    # polynomial order
    property polynomial_order:
        def __get__(self):
            return self.thisptr.getPolynomialOrder()
        def __set__(self, polynomial_order):
            self.thisptr.setPolynomialOrder(polynomial_order)

    # linearity type
    property linearity_type:
        def __get__(self):
            return self.thisptr.getLinearityType()
        def __set__(self, linearity_type):
            self.thisptr.setLinearityType(linearity_type)

    # nonlinear tolerance
    property nonlinear_tolerance:
        def __get__(self):
            return self.thisptr.getNonlinearTolerance()
        def __set__(self, nonlinear_tolerance):
            self.thisptr.setNonlinearTolerance(nonlinear_tolerance)

    # nonlinear steps
    property nonlinear_steps:
        def __get__(self):
            return self.thisptr.getNonlinearSteps()
        def __set__(self, nonlinear_steps):
            self.thisptr.setNonlinearSteps(nonlinear_steps)

    # damping coeff
    property damping_coeff:
        def __get__(self):
            return self.thisptr.getNewtonDampingCoeff()
        def __set__(self, damping_coeff):
            self.thisptr.setNewtonDampingCoeff(damping_coeff)

    # automatic damping
    property automatic_damping:
        def __get__(self):
            return self.thisptr.getNewtonAutomaticDamping()
        def __set__(self, automatic_damping):
            self.thisptr.setNewtonAutomaticDamping(automatic_damping)

    # damping number to increase
    property damping_number_to_increase:
        def __get__(self):
            return self.thisptr.getNewtonDampingNumberToIncrease()
        def __set__(self, damping_number_to_increase):
            self.thisptr.setNewtonDampingNumberToIncrease(damping_number_to_increase)

    # anderson acceleration
    property anderson_acceleration:
        def __get__(self):
            return self.thisptr.getPicardAndersonAcceleration()
        def __set__(self, acceleration):
            self.thisptr.setPicardAndersonAcceleration(acceleration)

    # anderson beta
    property anderson_beta:
        def __get__(self):
            return self.thisptr.getPicardAndersonBeta()
        def __set__(self, beta):
            self.thisptr.setPicardAndersonBeta(beta)

    # anderson number of last vectors
    property anderson_last_vectors:
        def __get__(self):
            return self.thisptr.getPicardAndersonNumberOfLastVectors()
        def __set__(self, number):
            self.thisptr.setPicardAndersonNumberOfLastVectors(number)

    # adaptivity type
    property adaptivity_type:
        def __get__(self):
            return self.thisptr.getAdaptivityType()
        def __set__(self, adaptivity_type):
            self.thisptr.setAdaptivityType(adaptivity_type)

    # adaptivity tolerance
    property adaptivity_tolerance:
        def __get__(self):
            return self.thisptr.getAdaptivityTolerance()
        def __set__(self, adaptivity_tolerance):
            self.thisptr.setAdaptivityTolerance(adaptivity_tolerance)

    # adaptivity back_steps
    property adaptivity_back_steps:
        def __get__(self):
            return self.thisptr.getAdaptivityBackSteps()
        def __set__(self, adaptivity_back_steps):
            self.thisptr.setAdaptivityBackSteps(adaptivity_back_steps)

    # adaptivity redone
    property adaptivity_redone:
        def __get__(self):
            return self.thisptr.getAdaptivityRedoneEach()
        def __set__(self, adaptivity_redone_each):
            self.thisptr.setAdaptivityRedoneEach(adaptivity_redone_each)

    # adaptivity steps
    property adaptivity_steps:
        def __get__(self):
            return self.thisptr.getAdaptivitySteps()
        def __set__(self, adaptivity_steps):
            self.thisptr.setAdaptivitySteps(adaptivity_steps)


    # initial condition
    property initial_condition:
        def __get__(self):
            return self.thisptr.getInitialCondition()
        def __set__(self, initial_condition):
            self.thisptr.setInitialCondition(initial_condition)

    # time skip
    property time_skip:
        def __get__(self):
            return self.thisptr.getTimeSkip()
        def __set__(self, skip):
            self.thisptr.setTimeSkip(skip)

    # boundaries
    def add_boundary(self, char *name, char *type, parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter

        cdef map[char*, char*] expression_map
        cdef pair[char*, char*] expression

        for key in parameters:
            if isinstance(parameters[key], dict):
                if ("value" in parameters[key]):
                    val = parameters[key]["value"]
                else:
                    val = 0.0

                if ("expression" in parameters[key]):
                    expression.first = key
                    expression.second = parameters[key]["expression"]
                    expression_map.insert(expression)
            else:
                val = parameters[key]

            parameter.first = key
            parameter.second = val
            parameters_map.insert(parameter)

        self.thisptr.addBoundary(name, type, parameters_map, expression_map)

    def modify_boundary(self, char *name, char *type = "", parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter

        cdef map[char*, char*] expression_map
        cdef pair[char*, char*] expression

        for key in parameters:
            if isinstance(parameters[key], dict):
                if ("value" in parameters[key]):
                    val = parameters[key]["value"]
                else:
                    val = 0.0

                if ("expression" in parameters[key]):
                    expression.first = key
                    expression.second = parameters[key]["expression"]
                    expression_map.insert(expression)
            else:
                val = parameters[key]

            parameter.first = key
            parameter.second = val
            parameters_map.insert(parameter)

        self.thisptr.modifyBoundary(name, type, parameters_map, expression_map)

    def remove_boundary(self, char *name):
        self.thisptr.removeBoundary(name)

    # materials
    def add_material(self, char *name, parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter

        cdef map[char*, char*] expression_map
        cdef pair[char*, char*] expression

        cdef map[char*, vector[double]] nonlin_x_map
        cdef pair[char*, vector[double]] nonlin_x
        cdef vector[double] x
        cdef map[char*, vector[double]] nonlin_y_map
        cdef pair[char*, vector[double]] nonlin_y
        cdef vector[double] y

        for key in parameters:
            if isinstance(parameters[key], dict):
                if ("value" in parameters[key]):
                    val = parameters[key]["value"]
                else:
                    val = 0.0

                if ("expression" in parameters[key]):
                    expression.first = key
                    expression.second = parameters[key]["expression"]
                    expression_map.insert(expression)

                if ("x" in parameters[key]):
                    for v in parameters[key]["x"]:
                        x.push_back(v)
                    nonlin_x.first = key
                    nonlin_x.second = x
                    nonlin_x_map.insert(nonlin_x)

                if ("y" in parameters[key]):
                    for v in parameters[key]["y"]:
                        y.push_back(v)
                    nonlin_y.first = key
                    nonlin_y.second = y
                    nonlin_y_map.insert(nonlin_y)
            else:
                val = parameters[key]

            parameter.first = key
            parameter.second = val
            parameters_map.insert(parameter)

        self.thisptr.addMaterial(name, parameters_map, expression_map, nonlin_x_map, nonlin_y_map)

    def modify_material(self, char *name, parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter

        cdef map[char*, char*] expression_map
        cdef pair[char*, char*] expression

        cdef map[char*, vector[double]] nonlin_x_map
        cdef pair[char*, vector[double]] nonlin_x
        cdef vector[double] x
        cdef map[char*, vector[double]] nonlin_y_map
        cdef pair[char*, vector[double]] nonlin_y
        cdef vector[double] y

        for key in parameters:
            if isinstance(parameters[key], dict):
                if ("value" in parameters[key]):
                    val = parameters[key]["value"]
                else:
                    val = 0.0

                if ("expression" in parameters[key]):
                    expression.first = key
                    expression.second = parameters[key]["expression"]
                    expression_map.insert(expression)

                if ("x" in parameters[key]):
                    for v in parameters[key]["x"]:
                        x.push_back(v)
                    nonlin_x.first = key
                    nonlin_x.second = x
                    nonlin_x_map.insert(nonlin_x)

                if ("y" in parameters[key]):
                    for v in parameters[key]["y"]:
                        y.push_back(v)
                    nonlin_y.first = key
                    nonlin_y.second = y
                    nonlin_y_map.insert(nonlin_y)
            else:
                val = parameters[key]

            parameter.first = key
            parameter.second = val
            parameters_map.insert(parameter)

        self.thisptr.modifyMaterial(name, parameters_map, expression_map, nonlin_x_map, nonlin_y_map)

    def remove_material(self, char *name):
        self.thisptr.removeMaterial(name)

    # local values
    def local_values(self, double x, double y, time_step = None, adaptivity_step = None, char *solution_type = "normal"):
        out = dict()
        cdef map[string, double] results

        self.thisptr.localValues(x, y,
                                 int(-1 if time_step is None else time_step),
                                 int(-1 if adaptivity_step is None else adaptivity_step),
                                 solution_type, results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # surface integrals
    def surface_integrals(self, edges = [], time_step = None, adaptivity_step = None, char *solution_type = "normal"):
        cdef vector[int] edges_vector
        for i in edges:
            edges_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self.thisptr.surfaceIntegrals(edges_vector,
                                      int(-1 if time_step is None else time_step),
                                      int(-1 if adaptivity_step is None else adaptivity_step),
                                      solution_type, results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # volume integrals
    def volume_integrals(self, labels = [], time_step = None, adaptivity_step = None, char *solution_type = "normal"):
        cdef vector[int] labels_vector
        for i in labels:
            labels_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self.thisptr.volumeIntegrals(labels_vector,
                                     int(-1 if time_step is None else time_step),
                                     int(-1 if adaptivity_step is None else adaptivity_step),
                                     solution_type, results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # initial mesh parameters
    def initial_mesh_parameters(self):
        parameters = dict()
        cdef map[string, int] parameters_map

        self.thisptr.initialMeshParameters(parameters_map)
        it = parameters_map.begin()
        while it != parameters_map.end():
            parameters[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return parameters

    # solution mesh parameters
    def solution_mesh_parameters(self):
        parameters = dict()
        cdef map[string, int] parameters_map

        self.thisptr.solutionMeshParameters(parameters_map)
        it = parameters_map.begin()
        while it != parameters_map.end():
            parameters[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return parameters

    # relative error
    def relative_error(self):
        cdef vector[double] error_vector
        cdef vector[int] dofs_vector
        self.thisptr.adaptivityInfo(error_vector, dofs_vector)

        error = list()
        for i in range(error_vector.size()):
            error.append(error_vector[i])

        return error

    # dofs
    def dofs(self):
        cdef vector[double] error_vector
        cdef vector[int] dofs_vector
        self.thisptr.adaptivityInfo(error_vector, dofs_vector)

        dofs = list()
        for i in range(dofs_vector.size()):
            dofs.append(dofs_vector[i])

        return dofs

def field(char *field_id):
    return __Field__(field_id)
