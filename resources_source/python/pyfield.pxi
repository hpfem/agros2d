cdef extern from "../../agros2d-library/pythonlab/pyfield.h":
    cdef cppclass PyField:
        PyField(string fieldId) except +

        string fieldId()

        string getAnalysisType()
        void setAnalysisType(string &analysisType) except +

        int getNumberOfRefinements()
        void setNumberOfRefinements(int numberOfRefinements) except +

        int getPolynomialOrder()
        void setPolynomialOrder(int polynomialOrder) except +

        string getLinearityType()
        void setLinearityType(string &linearityType) except +

        string getNonlinearConvergenceMeasurement()
        void setNonlinearConvergenceMeasurement(string &nonlinearConvergenceMeasurement) except +

        double getNonlinearTolerance()
        void setNonlinearTolerance(double nonlinearTolerance) except +

        int getNonlinearSteps()
        void setNonlinearSteps(int nonlinearSteps) except +

        double getNewtonDampingCoeff()
        void setNewtonDampingCoeff(double dampingCoeff) except +

        string getNewtonDampingType()
        void setNewtonDampingType(string &dampingType) except +

        int getNewtonDampingNumberToIncrease()
        void setNewtonDampingNumberToIncrease(int dampingNumberToIncrease) except +

        double getNewtonSufficientImprovementFactorForJacobianReuse()
        void setNewtonSufficientImprovementFactorForJacobianReuse(double sufficientImprovementFactorJacobian) except +

        double getNewtonSufficientImprovementFactor()
        void setNewtonSufficientImprovementFactor(double sufficientImprovementFactor) except +

        int getNewtonMaximumStepsWithReusedJacobian()
        void setNewtonMaximumStepsWithReusedJacobian(int maximumStepsWithReusedJacobian) except +

        bool getNewtonReuseJacobian()
        void setNewtonReuseJacobian(bool reuse) except +

        bool getPicardAndersonAcceleration()
        void setPicardAndersonAcceleration(bool acceleration) except +

        double getPicardAndersonBeta()
        void setPicardAndersonBeta(double beta) except +

        int getPicardAndersonNumberOfLastVectors()
        void setPicardAndersonNumberOfLastVectors(int number) except +

        string getAdaptivityType()
        void setAdaptivityType(string &adaptivityType) except +

        double getAdaptivityTolerance()
        void setAdaptivityTolerance(double adaptivityTolerance) except +

        int getAdaptivitySteps()
        void setAdaptivitySteps(int adaptivitySteps) except +

        double getAdaptivityThreshold()
        void setAdaptivityThreshold(double adaptivityThreshold)  except +

        string getAdaptivityStoppingCriterion()
        void setAdaptivityStoppingCriterion(string &adaptivityStoppingCriterion) except +

        string getAdaptivityNormType()
        void setAdaptivityNormType(string &adaptivityNormType) except +

        bool getAdaptivityAnisotropic()
        void setAdaptivityAnisotropic(bool adaptivityAnisotropic)

        bool getAdaptivityFinerReference()
        void setAdaptivityFinerReference(bool adaptivityFinerReference)

        int getAdaptivityBackSteps()
        void setAdaptivityBackSteps(int adaptivityBackSteps) except +

        int getAdaptivityRedoneEach()
        void setAdaptivityRedoneEach(int adaptivityRedoneEach) except +

        double getInitialCondition()
        void setInitialCondition(double initialCondition) except +

        double getTimeSkip()
        void setTimeSkip(double timeSkip) except +

        void addBoundary(string &name, string &type,
                         map[string, double] &parameters,
                         map[string, string] &expressions) except +
        void modifyBoundary(string &name, string &type,
                            map[string, double] &parameters,
                            map[string, string] &expressions) except +
        void removeBoundary(string &name)

        void addMaterial(string &name, map[string, double] &parameters,
                         map[string, string] &expressions,
                         map[string, vector[double]] &nonlin_x,
                         map[string, vector[double]] &nonlin_y,
                         map[string, map[string, string]] &settings) except +
        void modifyMaterial(string &name, map[string, double] &parameters,
                            map[string, string] &expressions,
                            map[string, vector[double]] &nonlin_x,
                            map[string, vector[double]] &nonlin_y,
                            map[string, map[string, string]] &settings) except +
        void removeMaterial(string &name)

        void solve()

        void localValues(double x, double y, int timeStep, int adaptivityStep,
                         string &solutionType, map[string, double] &results) except +
        void surfaceIntegrals(vector[int], int timeStep, int adaptivityStep,
                              string &solutionType, map[string, double] &results) except +
        void volumeIntegrals(vector[int], int timeStep, int adaptivityStep,
                             string &solutionType, map[string, double] &results) except +

        void initialMeshInfo(map[string , int] &info) except +
        void solutionMeshInfo(int timeStep, int adaptivityStep, string &solutionType, map[string , int] &info) except +

        void solverInfo(int timeStep, int adaptivityStep, string &solutionType, vector[double] &residual, vector[double] &dampingCoeff) except +

        void adaptivityInfo(int timeStep, string &solutionType, vector[double] &error, vector[int] &dofs) except +

cdef map[string, double] get_parameters_map(parameters):
    cdef map[string, double] parameters_map
    cdef pair[string, double] parameter

    for key in parameters:
        if isinstance(parameters[key], dict):
            if ("value" in parameters[key]):
                val = parameters[key]["value"]
            else:
                val = 0.0
        else:
            val = parameters[key]

        parameter.first = string(key)
        parameter.second = val
        parameters_map.insert(parameter)

    return parameters_map

cdef map[string, string] get_expression_map(parameters):
    cdef map[string, string] expression_map
    cdef pair[string, string] expression

    for key in parameters:
        if isinstance(parameters[key], dict):
            if ("expression" in parameters[key]):
                expression.first = string(key)
                expression.second = string(parameters[key]["expression"])
                expression_map.insert(expression)

    return expression_map

cdef map[string, vector[double]] get_nonlin_x_map(parameters):
    cdef map[string, vector[double]] nonlin_x_map
    cdef pair[string, vector[double]] nonlin_x
    cdef vector[double] x

    for key in parameters:
        if isinstance(parameters[key], dict):
            if ("x" in parameters[key]):
                for value in parameters[key]["x"]:
                    x.push_back(value)

                nonlin_x.first = string(key)
                nonlin_x.second = x
                nonlin_x_map.insert(nonlin_x)
                x.clear()

    return nonlin_x_map

cdef map[string, vector[double]] get_nonlin_y_map(parameters):
    cdef map[string, vector[double]] nonlin_y_map
    cdef pair[string, vector[double]] nonlin_y
    cdef vector[double] y

    for key in parameters:
        if isinstance(parameters[key], dict):
          if ("y" in parameters[key]):
              for value in parameters[key]["y"]:
                  y.push_back(value)

              nonlin_y.first = string(key)
              nonlin_y.second = y
              nonlin_y_map.insert(nonlin_y)
              y.clear()

    return nonlin_y_map

cdef map[string, map[string, string]] get_settings_map(parameters):
    cdef map[string, map[string, string]] settings_map
    cdef pair[string, map[string, string]] settings_map_pair
    cdef map[string, string] settings
    cdef pair[string, string] setting

    for key in parameters:
        if isinstance(parameters[key], dict):
            if ("interpolation" in parameters[key]):
                setting.first = string("interpolation")
                setting.second = string(parameters[key]["interpolation"])
                settings.insert(setting)

            if ("derivative_at_endpoints" in parameters[key]):
                setting.first = string("derivative_at_endpoints")
                setting.second = string(parameters[key]["derivative_at_endpoints"])
                settings.insert(setting)

            if ("extrapolation" in parameters[key]):
                setting.first = string("extrapolation")
                setting.second = string(parameters[key]["extrapolation"])
                settings.insert(setting)

        if (settings.size()):
            settings_map_pair.first = string(key)
            settings_map_pair.second = settings
            settings_map.insert(settings_map_pair)

            settings.clear()

    return settings_map


cdef class __Field__:
    cdef PyField *thisptr

    def __cinit__(self, field_id):
        self.thisptr = new PyField(string(field_id))
    def __dealloc__(self):
        del self.thisptr

    # field id
    property field_id:
        def __get__(self):
            return self.thisptr.fieldId().c_str()

    # analysis type
    property analysis_type:
        def __get__(self):
            return self.thisptr.getAnalysisType().c_str()
        def __set__(self, field_id):
            self.thisptr.setAnalysisType(string(field_id))

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
            return self.thisptr.getLinearityType().c_str()
        def __set__(self, linearity_type):
            self.thisptr.setLinearityType(string(linearity_type))

    # convergence measurement
    property nonlinear_convergence_measurement:
        def __get__(self):
            return self.thisptr.getNonlinearConvergenceMeasurement().c_str()
        def __set__(self, nonlinearConvergenceMeasurement):
            self.thisptr.setNonlinearConvergenceMeasurement(string(nonlinearConvergenceMeasurement))

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

    # damping type
    property damping_type:
        def __get__(self):
            return self.thisptr.getNewtonDampingType().c_str()
        def __set__(self, damping_type):
            self.thisptr.setNewtonDampingType(string(damping_type))

    # reuse jacobian
    property reuse_jacobian:
        def __get__(self):
            return self.thisptr.getNewtonReuseJacobian()
        def __set__(self, reuse):
            self.thisptr.setNewtonReuseJacobian(reuse)

    # damping number to increase
    property damping_number_to_increase:
        def __get__(self):
            return self.thisptr.getNewtonDampingNumberToIncrease()
        def __set__(self, damping_number_to_increase):
            self.thisptr.setNewtonDampingNumberToIncrease(damping_number_to_increase)

    # sufficient improvement factor for Jacobian reuse
    property sufficient_improvement_factor_for_jacobian_reuse:
        def __get__(self):
            return self.thisptr.getNewtonSufficientImprovementFactorForJacobianReuse()
        def __set__(self, sufficientImprovementFactorJacobian):
            self.thisptr.setNewtonSufficientImprovementFactorForJacobianReuse(sufficientImprovementFactorJacobian)

    # sufficient improvement factor
    property sufficient_improvement_factor:
        def __get__(self):
            return self.thisptr.getNewtonSufficientImprovementFactor()
        def __set__(self, sufficientImprovementFactor):
            self.thisptr.setNewtonSufficientImprovementFactor(sufficientImprovementFactor)

    # maximum steps with reused Jacobian
    property maximum_steps_with_reused_jacobian:
        def __get__(self):
            return self.thisptr.getNewtonMaximumStepsWithReusedJacobian()
        def __set__(self, maximumStepsWithReusedJacobian):
            self.thisptr.setNewtonMaximumStepsWithReusedJacobian(maximumStepsWithReusedJacobian)

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
            return self.thisptr.getAdaptivityType().c_str()
        def __set__(self, adaptivity_type):
            self.thisptr.setAdaptivityType(string(adaptivity_type))

    # adaptivity tolerance
    property adaptivity_tolerance:
        def __get__(self):
            return self.thisptr.getAdaptivityTolerance()
        def __set__(self, adaptivity_tolerance):
            self.thisptr.setAdaptivityTolerance(adaptivity_tolerance)

    # adaptivity threshold
    property adaptivity_threshold:
        def __get__(self):
            return self.thisptr.getAdaptivityThreshold()
        def __set__(self, adaptivity_threshold):
            self.thisptr.setAdaptivityThreshold(adaptivity_threshold)

    # adaptivity stopping criterion
    property adaptivity_stopping_criterion:
        def __get__(self):
            return self.thisptr.getAdaptivityStoppingCriterion().c_str()
        def __set__(self, adaptivity_stopping_criterion):
            self.thisptr.setAdaptivityStoppingCriterion(string(adaptivity_stopping_criterion))

    # adaptivity norm type
    property adaptivity_norm_type:
        def __get__(self):
            return self.thisptr.getAdaptivityNormType().c_str()
        def __set__(self, adaptivity_norm_type):
            self.thisptr.setAdaptivityNormType(string(adaptivity_norm_type))

    # adaptivity anisotropic
    property adaptivity_anisotropic:
        def __get__(self):
            return self.thisptr.getAdaptivityAnisotropic()
        def __set__(self, adaptivity_anisotropic):
            self.thisptr.setAdaptivityAnisotropic(adaptivity_anisotropic)

    # adaptivity finer reference
    property adaptivity_finer_reference:
        def __get__(self):
            return self.thisptr.getAdaptivityFinerReference()
        def __set__(self, adaptivity_finer_reference):
            self.thisptr.setAdaptivityFinerReference(adaptivity_finer_reference)

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
    def add_boundary(self, name, type, parameters = {}):
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)

        self.thisptr.addBoundary(string(name), string(type), parameters_map, expression_map)

    def modify_boundary(self, name, type = "", parameters = {}):
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)

        self.thisptr.modifyBoundary(string(name), string(type), parameters_map, expression_map)

    def remove_boundary(self, name):
        self.thisptr.removeBoundary(string(name))

    # materials
    def add_material(self, name, parameters = {}):
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)
        cdef map[string, vector[double]] nonlin_x_map = get_nonlin_x_map(parameters)
        cdef map[string, vector[double]] nonlin_y_map = get_nonlin_y_map(parameters)
        cdef map[string, map[string, string]] settings_map = get_settings_map(parameters)

        self.thisptr.addMaterial(string(name), parameters_map, expression_map, nonlin_x_map, nonlin_y_map, settings_map)

    def modify_material(self, name, parameters = {}):
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)
        cdef map[string, vector[double]] nonlin_x_map = get_nonlin_x_map(parameters)
        cdef map[string, vector[double]] nonlin_y_map = get_nonlin_y_map(parameters)
        cdef map[string, map[string, string]] settings_map = get_settings_map(parameters)

        self.thisptr.modifyMaterial(string(name), parameters_map, expression_map, nonlin_x_map, nonlin_y_map, settings_map)

    def remove_material(self, name):
        self.thisptr.removeMaterial(string(name))

    # local values
    def local_values(self, x, y, time_step = None, adaptivity_step = None, solution_type = "normal"):
        out = dict()
        cdef map[string, double] results

        self.thisptr.localValues(x, y,
                                 int(-1 if time_step is None else time_step),
                                 int(-1 if adaptivity_step is None else adaptivity_step),
                                 string(solution_type), results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # surface integrals
    def surface_integrals(self, edges = [], time_step = None, adaptivity_step = None, solution_type = "normal"):
        cdef vector[int] edges_vector
        for i in edges:
            edges_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self.thisptr.surfaceIntegrals(edges_vector,
                                      int(-1 if time_step is None else time_step),
                                      int(-1 if adaptivity_step is None else adaptivity_step),
                                      string(solution_type), results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # volume integrals
    def volume_integrals(self, labels = [], time_step = None, adaptivity_step = None, solution_type = "normal"):
        cdef vector[int] labels_vector
        for i in labels:
            labels_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self.thisptr.volumeIntegrals(labels_vector,
                                     int(-1 if time_step is None else time_step),
                                     int(-1 if adaptivity_step is None else adaptivity_step),
                                     string(solution_type), results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # initial mesh info
    def initial_mesh_info(self):
        info = dict()
        cdef map[string, int] info_map

        self.thisptr.initialMeshInfo(info_map)
        it = info_map.begin()
        while it != info_map.end():
            info[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return info

    # solution mesh info
    def solution_mesh_info(self, time_step = None, adaptivity_step = None, solution_type = "normal"):
        info = dict()
        cdef map[string, int] info_map

        self.thisptr.solutionMeshInfo(int(-1 if time_step is None else time_step),
                                      int(-1 if adaptivity_step is None else adaptivity_step),
                                      string(solution_type), info_map)

        it = info_map.begin()
        while it != info_map.end():
            info[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return info

    # solver info
    def solver_info(self, time_step = None, adaptivity_step = None, solution_type = 'normal'):
        cdef vector[double] residual_vector
        cdef vector[double] damping_vector
        self.thisptr.solverInfo(int(-1 if time_step is None else time_step),
                                int(-1 if adaptivity_step is None else adaptivity_step),
                                string(solution_type), residual_vector, damping_vector)

        residual = list()
        for i in range(residual_vector.size()):
            residual.append(residual_vector[i])

        damping = list()
        for i in range(damping_vector.size()):
            damping.append(damping_vector[i])

        return {'residual' : residual, 'damping' : damping}

    # adaptivity info
    def adaptivity_info(self, time_step = None, solution_type = 'normal'):
        cdef vector[double] error_vector
        cdef vector[int] dofs_vector
        self.thisptr.adaptivityInfo(int(-1 if time_step is None else time_step),
                                    string(solution_type), error_vector, dofs_vector)

        error = list()
        for i in range(error_vector.size()):
            error.append(error_vector[i])

        dofs = list()
        for i in range(dofs_vector.size()):
            dofs.append(dofs_vector[i])

        return {'error' : error, 'dofs' : dofs}

def field(field_id):
    return __Field__(field_id)
