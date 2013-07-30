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

        string getMatrixSolver()
        void setMatrixSolver(string &matrixSolver) except +

        double getLinearSolverAbsoluteTolerance()
        void setLinearSolverAbsoluteTolerance(double absoluteTolerance) except +

        int getLinearSolverIterations()
        void setLinearSolverIterations(int numberOfIterations)  except +

        string getLinearSolverMethod()
        void setLinearSolverMethod(string &linearSolverMethod) except +

        string getLinearSolverPreconditioner()
        void setLinearSolverPreconditioner(string &linearSolverPreconditioner) except +

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

        void solverInfo(int timeStep, int adaptivityStep, string &solutionType, vector[double] &residual, vector[double] &dampingCoeff, int &jacobianCalculations) except +

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

    property field_id:
        def __get__(self):
            return self.thisptr.fieldId().c_str()

    property analysis_type:
        def __get__(self):
            return self.thisptr.getAnalysisType().c_str()
        def __set__(self, field_id):
            self.thisptr.setAnalysisType(string(field_id))

    property number_of_refinements:
        def __get__(self):
            return self.thisptr.getNumberOfRefinements()
        def __set__(self, refinements):
            self.thisptr.setNumberOfRefinements(refinements)

    property polynomial_order:
        def __get__(self):
            return self.thisptr.getPolynomialOrder()
        def __set__(self, order):
            self.thisptr.setPolynomialOrder(order)

    property matrix_solver:
        def __get__(self):
            return self.thisptr.getMatrixSolver().c_str()
        def __set__(self, solver):
            self.thisptr.setMatrixSolver(string(solver))

    property matrix_iterative_solver_tolerance:
        def __get__(self):
            return self.thisptr.getLinearSolverAbsoluteTolerance()
        def __set__(self, tolerance):
            self.thisptr.setLinearSolverAbsoluteTolerance(tolerance)

    property matrix_iterative_solver_iterations:
        def __get__(self):
            return self.thisptr.getLinearSolverIterations()
        def __set__(self, iterations):
            self.thisptr.setLinearSolverIterations(iterations)

    property matrix_iterative_solver_method:
        def __get__(self):
            return self.thisptr.getLinearSolverMethod().c_str()
        def __set__(self, method):
            self.thisptr.setLinearSolverMethod(string(method))

    property matrix_iterative_solver_preconditioner:
        def __get__(self):
            return self.thisptr.getLinearSolverPreconditioner().c_str()
        def __set__(self, preconditioner):
            self.thisptr.setLinearSolverPreconditioner(string(preconditioner))

    # solver
    property solver:
        def __get__(self):
            return self.thisptr.getLinearityType().c_str()
        def __set__(self, solver):
            self.thisptr.setLinearityType(string(solver))

    # nonlinear
    property nonlinear_steps:
        def __get__(self):
            return self.thisptr.getNonlinearSteps()
        def __set__(self, steps):
            self.thisptr.setNonlinearSteps(steps)

    property nonlinear_tolerance:
        def __get__(self):
            return self.thisptr.getNonlinearTolerance()
        def __set__(self, tolerance):
            self.thisptr.setNonlinearTolerance(tolerance)

    property nonlinear_convergence_measurement:
        def __get__(self):
            return self.thisptr.getNonlinearConvergenceMeasurement().c_str()
        def __set__(self, measurement):
            self.thisptr.setNonlinearConvergenceMeasurement(string(measurement))

    # newton
    property newton_damping_type:
        def __get__(self):
            return self.thisptr.getNewtonDampingType().c_str()
        def __set__(self, damping):
            self.thisptr.setNewtonDampingType(string(damping))

    property newton_damping_factor:
        def __get__(self):
            return self.thisptr.getNewtonDampingCoeff()
        def __set__(self, factor):
            self.thisptr.setNewtonDampingCoeff(factor)

    property newton_damping_decrease_ratio:
        def __get__(self):
            return self.thisptr.getNewtonSufficientImprovementFactor()
        def __set__(self, ratio):
            self.thisptr.setNewtonSufficientImprovementFactor(ratio)

    property newton_damping_increase_steps:
        def __get__(self):
            return self.thisptr.getNewtonDampingNumberToIncrease()
        def __set__(self, steps):
            self.thisptr.setNewtonDampingNumberToIncrease(steps)

    property newton_jacobian_reuse:
        def __get__(self):
            return self.thisptr.getNewtonReuseJacobian()
        def __set__(self, reuse):
            self.thisptr.setNewtonReuseJacobian(reuse)

    property newton_jacobian_reuse_ratio:
        def __get__(self):
            return self.thisptr.getNewtonSufficientImprovementFactorForJacobianReuse()
        def __set__(self, ratio):
            self.thisptr.setNewtonSufficientImprovementFactorForJacobianReuse(ratio)

    property newton_jacobian_reuse_steps:
        def __get__(self):
            return self.thisptr.getNewtonMaximumStepsWithReusedJacobian()
        def __set__(self, steps):
            self.thisptr.setNewtonMaximumStepsWithReusedJacobian(steps)

    # picard
    """
    property picard_anderson_acceleration:
        def __get__(self):
            return self.thisptr.getPicardAndersonAcceleration()
        def __set__(self, acceleration):
            self.thisptr.setPicardAndersonAcceleration(acceleration)

    property picard_anderson_beta:
        def __get__(self):
            return self.thisptr.getPicardAndersonBeta()
        def __set__(self, beta):
            self.thisptr.setPicardAndersonBeta(beta)

    property picard_anderson_last_vectors:
        def __get__(self):
            return self.thisptr.getPicardAndersonNumberOfLastVectors()
        def __set__(self, number):
            self.thisptr.setPicardAndersonNumberOfLastVectors(number)
    """

    # adaptivity
    property adaptivity_type:
        def __get__(self):
            return self.thisptr.getAdaptivityType().c_str()
        def __set__(self, adaptivity_type):
            self.thisptr.setAdaptivityType(string(adaptivity_type))

    property adaptivity_steps:
        def __get__(self):
            return self.thisptr.getAdaptivitySteps()
        def __set__(self, steps):
            self.thisptr.setAdaptivitySteps(steps)

    property adaptivity_tolerance:
        def __get__(self):
            return self.thisptr.getAdaptivityTolerance()
        def __set__(self, tolerance):
            self.thisptr.setAdaptivityTolerance(tolerance)

    property adaptivity_threshold:
        def __get__(self):
            return self.thisptr.getAdaptivityThreshold()
        def __set__(self, threshold):
            self.thisptr.setAdaptivityThreshold(threshold)

    property adaptivity_stopping_criterion:
        def __get__(self):
            return self.thisptr.getAdaptivityStoppingCriterion().c_str()
        def __set__(self, criterion):
            self.thisptr.setAdaptivityStoppingCriterion(string(criterion))

    property adaptivity_norm_type:
        def __get__(self):
            return self.thisptr.getAdaptivityNormType().c_str()
        def __set__(self, norm):
            self.thisptr.setAdaptivityNormType(string(norm))

    property adaptivity_anisotropic_refinement:
        def __get__(self):
            return self.thisptr.getAdaptivityAnisotropic()
        def __set__(self, anisotropic):
            self.thisptr.setAdaptivityAnisotropic(anisotropic)

    property adaptivity_finer_reference:
        def __get__(self):
            return self.thisptr.getAdaptivityFinerReference()
        def __set__(self, finer_reference):
            self.thisptr.setAdaptivityFinerReference(finer_reference)

    property adaptivity_back_steps:
        def __get__(self):
            return self.thisptr.getAdaptivityBackSteps()
        def __set__(self, back_steps):
            self.thisptr.setAdaptivityBackSteps(back_steps)

    property adaptivity_redone_steps:
        def __get__(self):
            return self.thisptr.getAdaptivityRedoneEach()
        def __set__(self, steps):
            self.thisptr.setAdaptivityRedoneEach(steps)

    # transient
    property transient_initial_condition:
        def __get__(self):
            return self.thisptr.getInitialCondition()
        def __set__(self, condition):
            self.thisptr.setInitialCondition(condition)

    property transient_time_skip:
        def __get__(self):
            return self.thisptr.getTimeSkip()
        def __set__(self, skip):
            self.thisptr.setTimeSkip(skip)

    # boundaries
    def add_boundary(self, name, type, parameters = {}):
        """Add new boundary condition.

        add_boundary(name, type, parameters = {})

        Keyword arguments:
        name -- boundary condition name
        type -- boundary contition type
        parameters -- dict of boundary condition parameters (default is {})
        """
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)

        self.thisptr.addBoundary(string(name), string(type), parameters_map, expression_map)

    def modify_boundary(self, name, type = "", parameters = {}):
        """Modify existing boundary condition.

        modify_boundary(name, type = "", parameters = {})

        Keyword arguments:
        name -- boundary condition name
        type -- boundary contition type (default is "")
        parameters -- dict of boundary condition parameters (default is {})
        """
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)

        self.thisptr.modifyBoundary(string(name), string(type), parameters_map, expression_map)

    def remove_boundary(self, name):
        """Remove existing boundary condition.

        remove_boundary(name)

        Keyword arguments:
        name -- boundary condition name
        """
        self.thisptr.removeBoundary(string(name))

    # materials
    def add_material(self, name, parameters = {}):
        """Add new material.

        add_material(name, type, parameters = {})

        Keyword arguments:
        name -- material name
        type -- material type
        parameters -- dict of material parameters (default is {})
        """
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)
        cdef map[string, vector[double]] nonlin_x_map = get_nonlin_x_map(parameters)
        cdef map[string, vector[double]] nonlin_y_map = get_nonlin_y_map(parameters)
        cdef map[string, map[string, string]] settings_map = get_settings_map(parameters)

        self.thisptr.addMaterial(string(name), parameters_map, expression_map, nonlin_x_map, nonlin_y_map, settings_map)

    def modify_material(self, name, parameters = {}):
        """Modify existing material.

        modify_material(name, type = "", parameters = {})

        Keyword arguments:
        name -- material name
        type -- material type (default is {})
        parameters -- dict of material parameters (default is {})
        """
        cdef map[string, double] parameters_map = get_parameters_map(parameters)
        cdef map[string, string] expression_map = get_expression_map(parameters)
        cdef map[string, vector[double]] nonlin_x_map = get_nonlin_x_map(parameters)
        cdef map[string, vector[double]] nonlin_y_map = get_nonlin_y_map(parameters)
        cdef map[string, map[string, string]] settings_map = get_settings_map(parameters)

        self.thisptr.modifyMaterial(string(name), parameters_map, expression_map, nonlin_x_map, nonlin_y_map, settings_map)

    def remove_material(self, name):
        """Remove existing material.

        remove_material(name, type parameters = {})

        Keyword arguments:
        name -- material name
        """
        self.thisptr.removeMaterial(string(name))

    # local values
    def local_values(self, x, y, time_step = None, adaptivity_step = None, solution_type = "normal"):
        """Compute local values in point and return dictionary with results.

        local_values(x, y, time_step = None, adaptivity_step = None, solution_type = "normal")

        Keyword arguments:
        x -- x or r coordinate of point
        y -- y or z coordinate of point
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        solution_type -- solution type (default is "normal")
        """
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
        """Compute surface integrals on edges and return dictionary with results.

        surface_integrals(edges = [], time_step = None, adaptivity_step = None, solution_type = "normal")

        Keyword arguments:
        edges -- list of edges (default is [] - compute integrals on all edges)
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        solution_type -- solution type (default is "normal")
        """
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
        """Compute volume integrals on labels and return dictionary with results.

        volume_integrals(labels = [], time_step = None, adaptivity_step = None, solution_type = "normal")

        Keyword arguments:
        labels -- list of labels (default is [] - compute integrals on all labels)
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        solution_type -- solution type (default is "normal")
        """
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

    # mesh info
    def initial_mesh_info(self):
        """Return dictionary with initial mesh info."""
        info = dict()
        cdef map[string, int] info_map

        self.thisptr.initialMeshInfo(info_map)
        it = info_map.begin()
        while it != info_map.end():
            info[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return info

    def solution_mesh_info(self, time_step = None, adaptivity_step = None, solution_type = "normal"):
        """Return dictionary with solution mesh info.

        solution_mesh_info(time_step = None, adaptivity_step = None, solution_type = "normal")

        Keyword arguments:
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        solution_type -- solution type (default is "normal")
        """
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
        """Return dictionary with solver info.

        solver_info(time_step = None, adaptivity_step = None, solution_type = "normal")

        Keyword arguments:
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        solution_type -- solution type (default is "normal")
        """
        cdef vector[double] residual_vector
        cdef vector[double] damping_vector
        cdef int jacobian_calculations
        jacobian_calculations = -1
        self.thisptr.solverInfo(int(-1 if time_step is None else time_step),
                                int(-1 if adaptivity_step is None else adaptivity_step),
                                string(solution_type), residual_vector, damping_vector, jacobian_calculations)

        residual = list()
        for i in range(residual_vector.size()):
            residual.append(residual_vector[i])

        damping = list()
        for i in range(damping_vector.size()):
            damping.append(damping_vector[i])

        return {'residual' : residual, 'damping' : damping, 'jacobian_calculations' : jacobian_calculations}

    # adaptivity info
    def adaptivity_info(self, time_step = None, solution_type = 'normal'):
        """Return dictionary with adaptivity process info.

        adaptivity_info(time_step = None, solution_type = "normal")

        Keyword arguments:
        time_step -- time step (default is None - use last time step)
        solution_type -- solution type (default is "normal")
        """
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
