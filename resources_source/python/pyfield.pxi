cdef extern from "../../agros2d-library/pythonlab/pyfield.h":
    cdef cppclass PyField:
        PyField(string fieldId) except +

        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +

        bool getBoolParameter(string &parameter) except +
        int getIntParameter(string &parameter) except +
        double getDoubleParameter(string &parameter) except +

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

        string getLinearSolverMethod()
        void setLinearSolverMethod(string &linearSolverMethod) except +

        string getLinearSolverPreconditioner()
        void setLinearSolverPreconditioner(string &linearSolverPreconditioner) except +

        string getNonlinearDampingType()
        void setNonlinearDampingType(string &dampingType) except +

        string getAdaptivityType()
        void setAdaptivityType(string &adaptivityType) except +

        string getAdaptivityStoppingCriterion()
        void setAdaptivityStoppingCriterion(string &adaptivityStoppingCriterion) except +

        string getAdaptivityErrorCalculator()
        void setAdaptivityErrorCalculator(string &calculator) except +

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

        string filenameMatrix(int timeStep, int adaptivityStep) except +
        string filenameRHS(int timeStep, int adaptivityStep) except +

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
    cdef object matrix_solver_parameters
    cdef object solver_parameters
    cdef object adaptivity_parameters
    cdef object adaptivity_callback

    def __cinit__(self, field_id):
        self.thisptr = new PyField(string(field_id))
        self.matrix_solver_parameters = __Parameters__(self.__get_matrix_solver_parameters__,
                                                       self.__set_matrix_solver_parameters__)
        self.solver_parameters = __Parameters__(self.__get_solver_parameters__,
                                                self.__set_solver_parameters__)
        self.adaptivity_parameters = __Parameters__(self.__get_adaptivity_parameters__,
                                                    self.__set_adaptivity_parameters__)
        self.adaptivity_callback = None

    def __dealloc__(self):
        del self.thisptr

    # field id
    property field_id:
        def __get__(self):
            return self.thisptr.fieldId().c_str()

    # analysis
    property analysis_type:
        def __get__(self):
            return self.thisptr.getAnalysisType().c_str()
        def __set__(self, analysis):
            self.thisptr.setAnalysisType(string(analysis))

    # solver
    property solver:
        def __get__(self):
            return self.thisptr.getLinearityType().c_str()
        def __set__(self, solver):
            self.thisptr.setLinearityType(string(solver))

    property solver_parameters:
        def __get__(self):
            return self.solver_parameters.get_parameters()

    property adaptivity_callback:
        def __get__(self):
            return self.adaptivity_callback
        def __set__(self, callback):
            self.adaptivity_callback = callback

    def __get_solver_parameters__(self):
        return {'residual' : self.thisptr.getDoubleParameter(string('NonlinearResidualNorm')),
                'relative_change_of_solutions' : self.thisptr.getDoubleParameter(string('NonlinearRelativeChangeOfSolutions')),
                'damping' : self.thisptr.getNonlinearDampingType().c_str(),
                'damping_factor' : self.thisptr.getDoubleParameter(string('NonlinearDampingCoeff')),
                'damping_factor_decrease_ratio' : self.thisptr.getDoubleParameter(string('NonlinearDampingFactorDecreaseRatio')),
                'damping_factor_increase_steps' : self.thisptr.getIntParameter(string('NonlinearStepsToIncreaseDampingFactor')),
                'jacobian_reuse' : self.thisptr.getBoolParameter(string('NewtonReuseJacobian')),
                'jacobian_reuse_ratio' : self.thisptr.getDoubleParameter(string('NewtonJacobianReuseRatio')),
                'jacobian_reuse_steps' : self.thisptr.getIntParameter(string('NewtonMaxStepsReuseJacobian')),
                'anderson_acceleration' : self.thisptr.getBoolParameter(string('PicardAndersonAcceleration')),
                'anderson_beta' : self.thisptr.getDoubleParameter(string('PicardAndersonBeta')),
                'anderson_last_vectors' : self.thisptr.getIntParameter(string('PicardAndersonNumberOfLastVectors'))}

    def __set_solver_parameters__(self, parameters):
        # residual
        positive_value(parameters['residual'], 'residual')
        self.thisptr.setParameter(string('NonlinearResidualNorm'), <double>parameters['residual'])

        # relative change of solutions
        positive_value(parameters['relative_change_of_solutions'], 'relative_change_of_solutions')
        self.thisptr.setParameter(string('NonlinearRelativeChangeOfSolutions'), <double>parameters['relative_change_of_solutions'])

        # damping type
        self.thisptr.setNonlinearDampingType(string(parameters['damping']))

        # damping factor
        value_in_range(parameters['damping_factor'], 0.0, 1.0, 'damping_factor')
        self.thisptr.setParameter(string('NonlinearDampingCoeff'), <double>parameters['damping_factor'])

        # damping decrese ratio
        self.thisptr.setParameter(string('NonlinearDampingFactorDecreaseRatio'), <double>parameters['damping_factor_decrease_ratio'])

        # damping increase step
        value_in_range(parameters['damping_factor_increase_steps'], 1, 5, 'damping_factor_increase_steps')
        self.thisptr.setParameter(string('NonlinearStepsToIncreaseDampingFactor'), <int>parameters['damping_factor_increase_steps'])

        # jacobian reuse
        self.thisptr.setParameter(string('NewtonReuseJacobian'), <bool>parameters['jacobian_reuse'])

        # jacobian reuse ratio
        self.thisptr.setParameter(string('NewtonJacobianReuseRatio'), <double>parameters['jacobian_reuse_ratio'])

        # jacobian reuse step
        value_in_range(parameters['jacobian_reuse_steps'], 0, 100, 'jacobian_reuse_steps')
        self.thisptr.setParameter(string('NewtonMaxStepsReuseJacobian'), <int>parameters['jacobian_reuse_steps'])

        # Picard solver
        self.thisptr.setParameter(string('PicardAndersonAcceleration'), <int>parameters['anderson_acceleration'])
        value_in_range(parameters['anderson_last_vectors'], 1, 100, 'anderson_last_vectors')
        self.thisptr.setParameter(string('PicardAndersonNumberOfLastVectors'), <int>parameters['anderson_last_vectors'])
        value_in_range(parameters['anderson_beta'], 0.0, 1.0, 'anderson_beta')
        self.thisptr.setParameter(string('PicardAndersonBeta'), <int>parameters['anderson_beta'])

    # matrix solver
    property matrix_solver:
        def __get__(self):
            return self.thisptr.getMatrixSolver().c_str()
        def __set__(self, solver):
            self.thisptr.setMatrixSolver(string(solver))

    property matrix_solver_parameters:
        def __get__(self):
            return self.matrix_solver_parameters.get_parameters()

    def __get_matrix_solver_parameters__(self):
        return {'tolerance' : self.thisptr.getDoubleParameter(string('LinearSolverIterToleranceAbsolute')),
                'iterations' : self.thisptr.getIntParameter(string('LinearSolverIterIters')),
                'method' : self.thisptr.getLinearSolverMethod().c_str(),
                'preconditioner' : self.thisptr.getLinearSolverPreconditioner().c_str()}

    def __set_matrix_solver_parameters__(self, parameters):
        # tolerance
        positive_value(parameters['tolerance'], 'tolerance')
        self.thisptr.setParameter(string('LinearSolverIterToleranceAbsolute'), <double>parameters['tolerance'])

        # max iterations
        value_in_range(parameters['iterations'], 1, 1e4, 'iterations')
        self.thisptr.setParameter(string('LinearSolverIterIters'), <int>parameters['iterations'])

        # method, preconditioner
        self.thisptr.setLinearSolverMethod(string(parameters['method']))
        self.thisptr.setLinearSolverPreconditioner(string(parameters['preconditioner']))

    # refinements
    property number_of_refinements:
        def __get__(self):
            return self.thisptr.getNumberOfRefinements()
        def __set__(self, refinements):
            self.thisptr.setNumberOfRefinements(refinements)

    # order
    property polynomial_order:
        def __get__(self):
            return self.thisptr.getPolynomialOrder()
        def __set__(self, order):
            self.thisptr.setPolynomialOrder(order)

    # adaptivity
    property adaptivity_type:
        def __get__(self):
            return self.thisptr.getAdaptivityType().c_str()
        def __set__(self, adaptivity_type):
            self.thisptr.setAdaptivityType(string(adaptivity_type))

    property adaptivity_parameters:
        def __get__(self):
            return self.adaptivity_parameters.get_parameters()

    def __get_adaptivity_parameters__(self):
        return {'tolerance' : self.thisptr.getDoubleParameter(string('AdaptivityTolerance')),
                'steps' : self.thisptr.getIntParameter(string('AdaptivitySteps')),
                'stopping_criterion' : self.thisptr.getAdaptivityStoppingCriterion().c_str(),
                'threshold' : self.thisptr.getDoubleParameter(string('AdaptivityThreshold')),
                'error_calculator' : self.thisptr.getAdaptivityErrorCalculator().c_str(),
                'anisotropic_refinement' : self.thisptr.getBoolParameter(string('AdaptivityUseAniso')),
                'order_increase' : self.thisptr.getIntParameter(string('AdaptivityOrderIncrease')),
                'space_refinement' : self.thisptr.getBoolParameter(string('AdaptivitySpaceRefinement')),
                'finer_reference_solution' : self.thisptr.getBoolParameter(string('AdaptivityFinerReference')),
                'transient_back_steps' : self.thisptr.getIntParameter(string('AdaptivityTransientBackSteps')),
                'transient_redone_steps' : self.thisptr.getIntParameter(string('AdaptivityTransientRedoneEach'))}

    def __set_adaptivity_parameters__(self, parameters):
        # tolerance
        positive_value(parameters['tolerance'], 'tolerance')
        self.thisptr.setParameter(string('AdaptivityTolerance'), <double>parameters['tolerance'])

        # steps
        value_in_range(parameters['steps'], 1, 100, 'steps')
        self.thisptr.setParameter(string('AdaptivitySteps'), <int>parameters['steps'])

        # stoping criterion, norm
        self.thisptr.setAdaptivityStoppingCriterion(string(parameters['stopping_criterion']))
        self.thisptr.setAdaptivityErrorCalculator(string(parameters['error_calculator']))

        # threshold
        value_in_range(parameters['threshold'], 0.01, 1.0, 'threshold')
        self.thisptr.setParameter(string('AdaptivityThreshold'), <double>parameters['threshold'])

        # aniso, finer reference
        self.thisptr.setParameter(string('AdaptivityUseAniso'), <bool>parameters['anisotropic_refinement'])
        self.thisptr.setParameter(string('AdaptivityFinerReference'), <bool>parameters['finer_reference_solution'])

        # space refinement, order increase
        self.thisptr.setParameter(string('AdaptivitySpaceRefinement'), <bool>parameters['space_refinement'])
        value_in_range(parameters['order_increase'], 1, 10, 'order_increase')
        self.thisptr.setParameter(string('AdaptivityOrderIncrease'), <int>parameters['order_increase'])

        # back steps
        value_in_range(parameters['transient_back_steps'], 0, 100, 'transient_back_steps')
        self.thisptr.setParameter(string('AdaptivityTransientBackSteps'), <int>parameters['transient_back_steps'])

        # redone steps
        value_in_range(parameters['transient_redone_steps'], 1, 100, 'transient_redone_steps')
        self.thisptr.setParameter(string('AdaptivityTransientRedoneEach'), <int>parameters['transient_redone_steps'])

    # initial condition
    property transient_initial_condition:
        def __get__(self):
            return self.thisptr.getInitialCondition()
        def __set__(self, condition):
            self.thisptr.setInitialCondition(condition)

    # time skip
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

        # filename - matrix
    def filename_matrix(self, time_step = None, adaptivity_step = None):
        return self.thisptr.filenameMatrix(int(-1 if time_step is None else time_step),
                                           int(-1 if adaptivity_step is None else adaptivity_step))

        # filename - vector
    def filename_rhs(self, time_step = None, adaptivity_step = None):
        return self.thisptr.filenameRHS(int(-1 if time_step is None else time_step),
                                        int(-1 if adaptivity_step is None else adaptivity_step))
__fields__ = {}
def field(field_id):
    if not (__fields__.has_key(field_id)):
        __fields__[field_id] = __Field__(field_id)

    return __fields__[field_id]

def __remove_field__(field_id):
    if (__fields__.has_key(field_id)):
        del __fields__[field_id]
