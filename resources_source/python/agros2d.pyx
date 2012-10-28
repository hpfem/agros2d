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

cdef extern from "../../src/pythonlabagros.h":
    # PyProblem
    cdef cppclass PyProblem:
        PyProblem(int clear)

        void clear()
        void refresh()

        char *getName()
        void setName(char *name)

        char *getCoordinateType()
        void setCoordinateType(char *coordinateType) except +

        char *getMeshType()
        void setMeshType(char *meshType) except +

        char *getMatrixSolver()
        void setMatrixSolver(char *matrixSolver) except +

        double getFrequency()
        void setFrequency(double frequency) except +

        char *getTimeStepMethod()
        void setTimeStepMethod(char *timeStepMethod) except +

        int getTimeMethodOrder()
        void setTimeMethodOrder(int timeMethodOrder) except +

        double getTimeMethodTolerance()
        void setTimeMethodTolerance(double timeMethodTolerance) except +

        double getTimeTotal()
        void setTimeTotal(double timeTotal) except +

        int getNumConstantTimeSteps()
        void setNumConstantTimeSteps(int numConstantTimeSteps) except +

        char *getCouplingType(char *sourceField, char *targetField) except +
        void setCouplingType(char *sourceField, char *targetField, char *type) except +

        void solve()

    # PyField
    cdef cppclass PyField:
        PyField(char *field_id) except +

        char *getFieldId()

        char *getAnalysisType()
        void setAnalysisType(char*) except +

        int getNumberOfRefinements()
        void setNumberOfRefinements(int) except +

        int getPolynomialOrder()
        void setPolynomialOrder(int) except +

        char *getLinearityType()
        void setLinearityType(char*) except +

        double getNonlinearTolerance()
        void setNonlinearTolerance(double) except +

        int getNonlinearSteps()
        void setNonlinearSteps(int) except +

        char *getAdaptivityType()
        void setAdaptivityType(char*) except +

        double getAdaptivityTolerance()
        void setAdaptivityTolerance(double) except +

        int getAdaptivitySteps()
        void setAdaptivitySteps(int) except +

        double getInitialCondition()
        void setInitialCondition(double) except +

        double getTimeSkip()
        void setTimeSkip(double) except +

        void addBoundary(char*, char*, map[char*, double] parameters, map[char*, char*] expressions) except +
        void setBoundary(char*, char*, map[char*, double] parameters, map[char*, char*] expressions) except +
        void removeBoundary(char*)

        void addMaterial(char *id, map[char*, double] parameters, map[char*, char*] expressions, map[char*, vector[double]] nonlin_x, map[char*, vector[double]] nonlin_y) except +
        void setMaterial(char* name, map[char*, double] parameters, map[char*, char*] expressions, map[char*, vector[double]] nonlin_x, map[char*, vector[double]] nonlin_y) except +
        void removeMaterial(char* name)

        void solve()

        void localValues(double x, double y, map[string, double] results) except +
        void surfaceIntegrals(vector[int], map[string, double] results) except +
        void volumeIntegrals(vector[int], map[string, double] results) except +

    # PyGeometry
    cdef cppclass PyGeometry:
        PyGeometry()

        void activate()

        void addNode(double, double) except +
        void addEdge(double, double, double, double, double, map[char*, int], map[char*, char*]) except +
        void addEdgeByNodes(int, int, double, map[char*, int], map[char*, char*]) except +
        void addLabel(double, double, double, map[char*, int], map[char*, int], map[char*, char*]) except +

        void removeNode(int index) except +
        void removeEdge(int index) except +
        void removeLabel(int index) except +

        void removeNodePoint(double, double)
        void removeEdgePoint(double, double, double, double, double)
        void removeLabelPoint(double, double)

        void selectNodes(vector[int]) except +
        void selectEdges(vector[int]) except +
        void selectLabels(vector[int]) except +

        void selectNodePoint(double, double)
        void selectEdgePoint(double, double)
        void selectLabelPoint(double, double)

        void selectNone()

        void moveSelection(double, double, bool)
        void rotateSelection(double, double, double, bool)
        void scaleSelection(double, double, double, bool)
        void removeSelection()

        void mesh()
        char *meshFileName() except +

        void zoomBestFit()
        void zoomIn()
        void zoomOut()
        void zoomRegion(double, double, double, double)

    # PyViewConfig
    cdef cppclass PyViewConfig:
        void setField(char *fieldid) except +
        char *getField()

        void setActiveTimeStep(int timeStep) except +
        int getActiveTimeStep()

        void setActiveAdaptivityStep(int adaptiveStep) except +
        int getActiveAdaptivityStep()

        void setActiveSolutionType(char *solutionType) except +
        char *getActiveSolutionType()

        void setGridShow(bool show)
        bool getGridShow()

        void setGridStep(double step)
        double getGridStep()

        void setAxesShow(bool show)
        bool getAxesShow()

        void setRulersShow(bool show)
        bool getRulersShow()

    # PyViewMesh
    cdef cppclass PyViewMesh:
        void activate()

        void setInitialMeshViewShow(bool show)
        bool getInitialMeshViewShow()
        void setSolutionMeshViewShow(bool show)
        bool getSolutionMeshViewShow()

        void setOrderViewShow(bool show)
        bool getOrderViewShow()
        void setOrderViewColorBar(bool show)
        bool getOrderViewColorBar()
        void setOrderViewLabel(bool show)
        bool getOrderViewLabel()
        void setOrderViewPalette(char *palette) except +
        char* getOrderViewPalette()

    # PyViewPost2D
    cdef cppclass PyViewPost2D:
        void activate()

        void setScalarViewShow(bool show)
        bool getScalarViewShow()
        void setScalarViewVariable(char *variable) except +
        char *getScalarViewVariable()
        void setScalarViewVariableComp(char *component) except +
        char *getScalarViewVariableComp()
        void setScalarViewPalette(char *palette) except +
        char *getScalarViewPalette()
        void setScalarViewPaletteQuality(char *quality) except +
        char *getScalarViewPaletteQuality()

        void setScalarViewPaletteSteps(int steps) except +
        int getScalarViewPaletteSteps()
        void setScalarViewPaletteFilter(bool filter)
        bool getScalarViewPaletteFilter()

        void setScalarViewRangeLog(bool log)
        bool getScalarViewRangeLog()
        void setScalarViewRangeBase(double base)
        double getScalarViewRangeBase()

        void setScalarViewColorBar(bool show)
        bool getScalarViewColorBar()
        void setScalarViewDecimalPlace(int place) except +
        int getScalarViewDecimalPlace()

        void setScalarViewRangeAuto(bool autoRange)
        bool getScalarViewRangeAuto()
        void setScalarViewRangeMin(double min)
        double getScalarViewRangeMin()
        void setScalarViewRangeMax(double max)
        double getScalarViewRangeMax()

        void setContourShow(bool show)
        bool getContourShow()
        void setContourCount(int count) except +
        int getContourCount()
        void setContourVariable(char *variable) except +
        char *getContourVariable()

        void setVectorShow(bool show)
        bool getVectorShow()
        void setVectorCount(int count) except +
        int getVectorCount()
        void setVectorScale(double scale) except +
        int getVectorScale()
        void setVectorVariable(char *variable) except +
        char *getVectorVariable()
        void setVectorProportional(bool show)
        bool getVectorProportional()
        void setVectorColor(bool show)
        bool getVectorColor()

    # PyViewPost3D
    cdef cppclass PyViewPost3D:
        void activate()

        void setPost3DMode(char *mode) except +
        char *getPost3DMode()

        void setScalarViewVariable(char *variable) except +
        char *getScalarViewVariable()
        void setScalarViewVariableComp(char *component) except +
        char *getScalarViewVariableComp()
        void setScalarViewPalette(char *palette) except +
        char *getScalarViewPalette()
        void setScalarViewPaletteQuality(char *quality) except +
        char *getScalarViewPaletteQuality()

        void setScalarViewPaletteSteps(int steps) except +
        int getScalarViewPaletteSteps()
        void setScalarViewPaletteFilter(bool filter)
        bool getScalarViewPaletteFilter()

        void setScalarViewRangeLog(bool log)
        bool getScalarViewRangeLog()
        void setScalarViewRangeBase(double base)
        double getScalarViewRangeBase()

        void setScalarViewColorBar(bool show)
        bool getScalarViewColorBar()
        void setScalarViewDecimalPlace(int place) except +
        int getScalarViewDecimalPlace()

        void setScalarViewRangeAuto(bool autoRange)
        bool getScalarViewRangeAuto()
        void setScalarViewRangeMin(double min)
        double getScalarViewRangeMin()
        void setScalarViewRangeMax(double max)
        double getScalarViewRangeMax()

    char *pyVersion()
    void pyQuit()

    char *pyInput(char *str)
    void pyMessage(char *str)

    void pyOpenDocument(char *str) except +
    void pySaveDocument(char *str) except +
    void pyCloseDocument()

    void pySaveImage(char *str, int w, int h) except +

    # PyParticleTracing
    cdef cppclass PyParticleTracing:
        PyParticleTracing()

        void setInitialPosition(double x, double y) except +
        void initialPosition(double x, double y)

        void setInitialVelocity(double x, double y)
        void initialVelocity(double x, double y)

        void setParticleMass(double mass) except +
        double particleMass()

        void setParticleCharge(double charge)
        double particleCharge()

        void setIncludeGravitation(int incl)
        int includeGravitation()

        void setReflectOnDifferentMaterial(int reflect)
        int reflectOnDifferentMaterial()
        void setReflectOnBoundary(int reflect)
        int reflectOnBoundary()
        void setCoefficientOfRestitution(double coeff)
        double coefficientOfRestitution()

        void setDragForceDensity(double rho) except +
        double dragForceDensity()
        void setDragForceReferenceArea(double area) except +
        double dragForceReferenceArea()
        void setDragForceCoefficient(double coeff) except +
        double dragForceCoefficient()

        void setMaximumTolerance(double tolerance) except +
        double maximumTolerance()
        void setMaximumNumberOfSteps(int steps) except +
        int maximumNumberOfSteps()
        void setMinimumStep(double step) except +
        double minimumStep()

        void solve() except +

        int length()
        void positions(vector[double] x, vector[double] y, vector[double] z)
        void velocities(vector[double] x, vector[double] y, vector[double] z)

# Problem
cdef class __Problem__:
    cdef PyProblem *thisptr

    # Problem(clear)
    def __cinit__(self, int clear = 0):
        self.thisptr = new PyProblem(clear)

    def __dealloc__(self):
        del self.thisptr

    # clear
    def clear(self):
        self.thisptr.clear()

    # refresh
    def refresh(self):
        self.thisptr.refresh()

    # name
    property name:
        def __get__(self):
            return self.thisptr.getName()
        def __set__(self, name):
            self.thisptr.setName(name)

    # coordinate_type
    property coordinate_type:
        def __get__(self):
            return self.thisptr.getCoordinateType()
        def __set__(self, coordinate_type):
            self.thisptr.setCoordinateType(coordinate_type)

    # mesh_type
    property mesh_type:
        def __get__(self):
            return self.thisptr.getMeshType()
        def __set__(self, mesh_type):
            self.thisptr.setMeshType(mesh_type)

    # matrix_solver
    property matrix_solver:
        def __get__(self):
            return self.thisptr.getMatrixSolver()
        def __set__(self, matrix_solver):
            self.thisptr.setMatrixSolver(matrix_solver)

    # frequency
    property frequency:
        def __get__(self):
            return self.thisptr.getFrequency()
        def __set__(self, frequency):
            self.thisptr.setFrequency(frequency)

    # time_step_method
    property time_step_method:
        def __get__(self):
            return self.thisptr.getTimeStepMethod()
        def __set__(self, time_step_method):
            self.thisptr.setTimeStepMethod(time_step_method)

    # time_method_order
    property time_method_order:
        def __get__(self):
            return self.thisptr.getTimeMethodOrder()
        def __set__(self, time_method_order):
            self.thisptr.setTimeMethodOrder(time_method_order)

    # time_method_tolerance
    property time_method_tolerance:
        def __get__(self):
            return self.thisptr.getTimeMethodTolerance()
        def __set__(self, time_method_tolerance):
            self.thisptr.setTimeMethodTolerance(time_method_tolerance)

    # time_total
    property time_total:
        def __get__(self):
            return self.thisptr.getTimeTotal()
        def __set__(self, time_total):
            self.thisptr.setTimeTotal(time_total)

    # time_steps
    property time_steps:
        def __get__(self):
            return self.thisptr.getNumConstantTimeSteps()
        def __set__(self, steps):
            self.thisptr.setNumConstantTimeSteps(steps)

    # coupling type
    def get_coupling_type(self, source_field, target_field):
        return self.thisptr.getCouplingType(source_field, target_field)
    def set_coupling_type(self, source_field, target_field, type):
            self.thisptr.setCouplingType(source_field, target_field, type)

    # solve
    def solve(self):
        self.thisptr.solve()

# Field
cdef class __Field__:
    cdef PyField *thisptr

    # Field(field_id)
    def __cinit__(self, char *field_id):
        # todo - more problems
        self.thisptr = new PyField(field_id)
    def __dealloc__(self):
        del self.thisptr

    # field_id
    property field_id:
        def __get__(self):
            return self.thisptr.getFieldId()

    # analysis_type
    property analysis_type:
        def __get__(self):
            return self.thisptr.getAnalysisType()
        def __set__(self, field_id):
            self.thisptr.setAnalysisType(field_id)

    # number_of_refinements
    property number_of_refinements:
        def __get__(self):
            return self.thisptr.getNumberOfRefinements()
        def __set__(self, number_of_refinements):
            self.thisptr.setNumberOfRefinements(number_of_refinements)

    # polynomial_order
    property polynomial_order:
        def __get__(self):
            return self.thisptr.getPolynomialOrder()
        def __set__(self, polynomial_order):
            self.thisptr.setPolynomialOrder(polynomial_order)

    # linearity_type
    property linearity_type:
        def __get__(self):
            return self.thisptr.getLinearityType()
        def __set__(self, linearity_type):
            self.thisptr.setLinearityType(linearity_type)

    # nonlinear_tolerance
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

    # adaptivity_type
    property adaptivity_type:
        def __get__(self):
            return self.thisptr.getAdaptivityType()
        def __set__(self, adaptivity_type):
            self.thisptr.setAdaptivityType(adaptivity_type)

    # adaptivity_tolerance
    property adaptivity_tolerance:
        def __get__(self):
            return self.thisptr.getAdaptivityTolerance()
        def __set__(self, adaptivity_tolerance):
            self.thisptr.setAdaptivityTolerance(adaptivity_tolerance)

    # adaptivity_steps
    property adaptivity_steps:
        def __get__(self):
            return self.thisptr.getAdaptivitySteps()
        def __set__(self, adaptivity_steps):
            self.thisptr.setAdaptivitySteps(adaptivity_steps)

    # initial_condition
    property initial_condition:
        def __get__(self):
            return self.thisptr.getInitialCondition()
        def __set__(self, initial_condition):
            self.thisptr.setInitialCondition(initial_condition)

    # time_skip
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

    def set_boundary(self, char *name, char *type = "", parameters = {}):
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

        self.thisptr.setBoundary(name, type, parameters_map, expression_map)

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

    def set_material(self, char *name, parameters = {}):
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

        self.thisptr.setMaterial(name, parameters_map, expression_map, nonlin_x_map, nonlin_y_map)

    def remove_material(self, char *name):
        self.thisptr.removeMaterial(name)

    # local values
    def local_values(self, double x, double y):
        out = dict()
        cdef map[string, double] results

        self.thisptr.localValues(x, y, results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # surface integrals
    def surface_integrals(self, edges = []):
        cdef vector[int] edges_vector
        for i in edges:
            edges_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self.thisptr.surfaceIntegrals(edges_vector, results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

    # volume integrals
    def volume_integrals(self, labels = []):
        cdef vector[int] labels_vector
        for i in labels:
            labels_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self.thisptr.volumeIntegrals(labels_vector, results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.c_str()] = deref(it).second
            incr(it)

        return out

# Geometry
cdef class __Geometry__:
    cdef PyGeometry *thisptr

    # Geometry()
    def __cinit__(self):
        self.thisptr = new PyGeometry()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    # add_node(x, y)
    def add_node(self, double x, double y):
        self.thisptr.addNode(x, y)

    # remove_node(index)
    def remove_node(self, int index):
        self.thisptr.removeNode(index)

    # add_edge(x1, y1, x2, y2, angle, refinements, boundaries)
    def add_edge(self, double x1, double y1, double x2, double y2, double angle = 0.0, refinements = {}, boundaries = {}):

        cdef map[char*, int] refinements_map
        cdef pair[char*, int] refinement
        for key in refinements:
            refinement.first = key
            refinement.second = refinements[key]
            refinements_map.insert(refinement)

        cdef map[char*, char*] boundaries_map
        cdef pair[char*, char *] boundary
        for key in boundaries:
            boundary.first = key
            boundary.second = boundaries[key]
            boundaries_map.insert(boundary)

        self.thisptr.addEdge(x1, y1, x2, y2, angle, refinements_map, boundaries_map)

    # add_edge_by_nodes(start_node_index, end_node_index, angle, refinements, boundaries)
    def add_edge_by_nodes(self, int start_node_index, int end_node_index, double angle = 0.0, refinements = {}, boundaries = {}):

        cdef map[char*, int] refinements_map
        cdef pair[char*, int] refinement
        for key in refinements:
            refinement.first = key
            refinement.second = refinements[key]
            refinements_map.insert(refinement)

        cdef map[char*, char*] boundaries_map
        cdef pair[char*, char *] boundary
        for key in boundaries:
            boundary.first = key
            boundary.second = boundaries[key]
            boundaries_map.insert(boundary)

        self.thisptr.addEdgeByNodes(start_node_index, end_node_index, angle, refinements_map, boundaries_map)

    # remove_edge(index)
    def remove_edge(self, int index):
        self.thisptr.removeEdge(index)

    # add_label(x, y, area, refinements, orders, materials)
    def add_label(self, double x, double y, double area = 0.0, refinements = {}, orders = {}, materials = {}):

        cdef map[char*, int] refinements_map
        cdef pair[char*, int] refinement
        for key in refinements:
            refinement.first = key
            refinement.second = refinements[key]
            refinements_map.insert(refinement)

        cdef map[char*, int] orders_map
        cdef pair[char*, int] order
        for key in orders:
            order.first = key
            order.second = orders[key]
            orders_map.insert(order)

        cdef map[char*, char*] materials_map
        cdef pair[char*, char *] material
        for key in materials:
            material.first = key
            material.second = materials[key]
            materials_map.insert(material)

        self.thisptr.addLabel(x, y, area, refinements_map, orders_map, materials_map)

    # remove_label(index)
    def remove_label(self, int index):
        self.thisptr.removeLabel(index)

    # remove_node_point(x, y)
    def remove_node_point(self, double x, double y):
        self.thisptr.removeNodePoint(x, y)

    # remove_edge_point(x1, y1, x2, y2, angle)
    def remove_edge_point(self, double x1, double y1, double x2, double y2, double angle):
        self.thisptr.removeEdgePoint(x1, y1, x2, y2, angle)

    # remove_label_point(x, y)
    def remove_label_point(self, double x, double y):
        self.thisptr.removeLabelPoint(x, y)

    # select_nodes(nodes)
    def select_nodes(self, nodes = []):
        cdef vector[int] nodes_vector
        for i in nodes:
            nodes_vector.push_back(i)

        self.thisptr.selectNodes(nodes_vector)

    # select_edges(edges)
    def select_edges(self, edges = []):
        cdef vector[int] edges_vector
        for i in edges:
            edges_vector.push_back(i)

        self.thisptr.selectEdges(edges_vector)

    # select_labels(labels)
    def select_labels(self, labels = []):
        cdef vector[int] labels_vector
        for i in labels:
            labels_vector.push_back(i)

        self.thisptr.selectLabels(labels_vector)

    # select_node_point(x, y)
    def select_node_point(self, double x, double y):
        self.thisptr.selectNodePoint(x, y)

    # select_edge_point(x, y)
    def select_edge_point(self, double x, double y):
        self.thisptr.selectEdgePoint(x, y)

    # select_label_point(x, y)
    def select_label_point(self, double x, double y):
        self.thisptr.selectLabelPoint(x, y)


    # move_selection(dx, dy, copy)
    def move_selection(self, double dx, double dy, int copy = False):
        self.thisptr.moveSelection(dx, dy, int(copy))

    # rotate_selection(x, y, angle, copy)
    def rotate_selection(self, double x, double y, double angle, int copy = False):
        self.thisptr.rotateSelection(x, y, angle, int(copy))

    # scale_selection(x, y, scale, copy)
    def scale_selection(self, double x, double y, double scale, int copy = False):
        self.thisptr.scaleSelection(x, y, scale, int(copy))

    # remove_selection(dx, dy, copy)
    def remove_selection(self):
        self.thisptr.removeSelection()

    # select_none()
    def select_none(self):
        self.thisptr.selectNone()

    # mesh()
    def mesh(self):
        self.thisptr.mesh()

    # mesh_file_name()
    def mesh_file_name(self):
        self.thisptr.meshFileName()

    # zoom_best_fit()
    def zoom_best_fit(self):
        self.thisptr.zoomBestFit()

    # zoom_in()
    def zoom_in(self):
        self.thisptr.zoomIn()

    # zoom_out()
    def zoom_out(self):
        self.thisptr.zoomOut()

    # zoom_region()
    def zoom_region(self, double x1, double y1, double x2, double y2):
        self.thisptr.zoomRegion(x1, y1, x2, y2)

# ViewConfig
cdef class __ViewConfig__:
    cdef PyViewConfig *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewConfig()
    def __dealloc__(self):
        del self.thisptr

    property field:
        def __get__(self):
            return self.thisptr.getField()
        def __set__(self, id):
            self.thisptr.setField(id)

    property time_step:
        def __get__(self):
            return self.thisptr.getActiveTimeStep()
        def __set__(self, time_step):
            self.thisptr.setActiveTimeStep(time_step)

    property adaptivity_step:
        def __get__(self):
            return self.thisptr.getActiveAdaptivityStep()
        def __set__(self, adaptivity_step):
            self.thisptr.setActiveAdaptivityStep(adaptivity_step)

    property solution_type:
        def __get__(self):
            return self.thisptr.getActiveSolutionType()
        def __set__(self, solution_type):
            self.thisptr.setActiveSolutionType(solution_type)

    property grid:
        def __get__(self):
            return self.thisptr.getGridShow()
        def __set__(self, show):
            self.thisptr.setGridShow(show)

    property grid_step:
        def __get__(self):
            return self.thisptr.getGridStep()
        def __set__(self, step):
            self.thisptr.setGridStep(step)

    property axes:
        def __get__(self):
            return self.thisptr.getAxesShow()
        def __set__(self, show):
            self.thisptr.setAxesShow(show)

    property rulers:
        def __get__(self):
            return self.thisptr.getRulersShow()
        def __set__(self, show):
            self.thisptr.setRulersShow(show)

# ViewMesh
cdef class __ViewMesh__:
    cdef PyViewMesh *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewMesh()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    property solution_mesh:
        def __get__(self):
            return self.thisptr.getSolutionMeshViewShow()
        def __set__(self, show):
            self.thisptr.setSolutionMeshViewShow(show)

    property initial_mesh:
        def __get__(self):
            return self.thisptr.getInitialMeshViewShow()
        def __set__(self, show):
            self.thisptr.setInitialMeshViewShow(show)

    property order:
        def __get__(self):
            return self.thisptr.getOrderViewShow()
        def __set__(self, show):
            self.thisptr.setOrderViewShow(show)

    property order_color_bar:
        def __get__(self):
            return self.thisptr.getOrderViewColorBar()
        def __set__(self, show):
            self.thisptr.setOrderViewColorBar(show)

    property order_label:
        def __get__(self):
            return self.thisptr.getOrderViewLabel()
        def __set__(self, show):
            self.thisptr.setOrderViewLabel(show)

    property order_palette:
        def __get__(self):
            return self.thisptr.getOrderViewPalette()
        def __set__(self, palette):
            self.thisptr.setOrderViewPalette(palette)

# ViewPost2D
cdef class __ViewPost2D__:
    cdef PyViewPost2D *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewPost2D()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    # scalar view
    property scalar:
        def __get__(self):
            return self.thisptr.getScalarViewShow()
        def __set__(self, show):
            self.thisptr.setScalarViewShow(show)

    property scalar_variable:
        def __get__(self):
            return self.thisptr.getScalarViewVariable()
        def __set__(self, variable):
            self.thisptr.setScalarViewVariable(variable)

    property scalar_component:
        def __get__(self):
            return self.thisptr.getScalarViewVariableComp()
        def __set__(self, component):
            self.thisptr.setScalarViewVariableComp(component)

    property scalar_palette:
        def __get__(self):
            return self.thisptr.getScalarViewPalette()
        def __set__(self, palette):
            self.thisptr.setScalarViewPalette(palette)

    property scalar_palette_quality:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteQuality()
        def __set__(self, quality):
            self.thisptr.setScalarViewPaletteQuality(quality)

    property scalar_palette_steps:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteSteps()
        def __set__(self, steps):
            self.thisptr.setScalarViewPaletteSteps(steps)

    property scalar_palette_filter:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteFilter()
        def __set__(self, filter):
            self.thisptr.setScalarViewPaletteFilter(filter)

    property scalar_log_scale:
        def __get__(self):
            return self.thisptr.getScalarViewRangeLog()
        def __set__(self, log):
            self.thisptr.setScalarViewRangeLog(log)

    property scalar_log_base:
        def __get__(self):
            return self.thisptr.getScalarViewRangeBase()
        def __set__(self, base):
            self.thisptr.setScalarViewRangeBase(base)

    property scalar_color_bar:
        def __get__(self):
            return self.thisptr.getScalarViewColorBar()
        def __set__(self, show):
            self.thisptr.setScalarViewColorBar(show)

    property scalar_decimal_place:
        def __get__(self):
            return self.thisptr.getScalarViewDecimalPlace()
        def __set__(self, place):
            self.thisptr.setScalarViewDecimalPlace(place)

    property scalar_auto_range:
        def __get__(self):
            return self.thisptr.getScalarViewRangeAuto()
        def __set__(self, range_auto):
            self.thisptr.setScalarViewRangeAuto(range_auto)

    property scalar_range_min:
        def __get__(self):
            return self.thisptr.getScalarViewRangeMin()
        def __set__(self, min):
            self.thisptr.setScalarViewRangeMin(min)

    property scalar_range_max:
        def __get__(self):
            return self.thisptr.getScalarViewRangeMax()
        def __set__(self, max):
            self.thisptr.setScalarViewRangeMax(max)

    # contour
    property contours:
        def __get__(self):
            return self.thisptr.getContourShow()
        def __set__(self, show):
            self.thisptr.setContourShow(show)

    property contours_count:
        def __get__(self):
            return self.thisptr.getContourCount()
        def __set__(self, count):
            self.thisptr.setContourCount(count)

    property contours_variable:
        def __get__(self):
            return self.thisptr.getContourVariable()
        def __set__(self, variable):
            self.thisptr.setContourVariable(variable)

    # vector
    property vectors:
        def __get__(self):
            return self.thisptr.getVectorShow()
        def __set__(self, show):
            self.thisptr.setVectorShow(show)

    property vectors_count:
        def __get__(self):
            return self.thisptr.getVectorCount()
        def __set__(self, count):
            self.thisptr.setVectorCount(count)

    property vectors_scale:
        def __get__(self):
            return self.thisptr.getVectorScale()
        def __set__(self, count):
            self.thisptr.setVectorScale(count)

    property vectors_variable:
        def __get__(self):
            return self.thisptr.getVectorVariable()
        def __set__(self, variable):
            self.thisptr.setVectorVariable(variable)

    property vectors_proportional:
        def __get__(self):
            return self.thisptr.getVectorProportional()
        def __set__(self, show):
            self.thisptr.setVectorProportional(show)

    property vectors_color:
        def __get__(self):
            return self.thisptr.getVectorColor()
        def __set__(self, show):
            self.thisptr.setVectorColor(show)

# ViewPost3D
cdef class __ViewPost3D__:
    cdef PyViewPost3D *thisptr

    def activate(self):
        self.thisptr.activate()

    def __cinit__(self):
        self.thisptr = new PyViewPost3D()
    def __dealloc__(self):
        del self.thisptr

    # mode
    property mode:
        def __get__(self):
            return self.thisptr.getPost3DMode()
        def __set__(self, mode):
            self.thisptr.setPost3DMode(mode)

    # scalar view
    property scalar_variable:
        def __get__(self):
            return self.thisptr.getScalarViewVariable()
        def __set__(self, variable):
            self.thisptr.setScalarViewVariable(variable)

    property scalar_component:
        def __get__(self):
            return self.thisptr.getScalarViewVariableComp()
        def __set__(self, component):
            self.thisptr.setScalarViewVariableComp(component)

    property scalar_palette:
        def __get__(self):
            return self.thisptr.getScalarViewPalette()
        def __set__(self, palette):
            self.thisptr.setScalarViewPalette(palette)

    property scalar_palette_quality:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteQuality()
        def __set__(self, quality):
            self.thisptr.setScalarViewPaletteQuality(quality)

    property scalar_palette_steps:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteSteps()
        def __set__(self, steps):
            self.thisptr.setScalarViewPaletteSteps(steps)

    property scalar_palette_filter:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteFilter()
        def __set__(self, filter):
            self.thisptr.setScalarViewPaletteFilter(filter)

    property scalar_log_scale:
        def __get__(self):
            return self.thisptr.getScalarViewRangeLog()
        def __set__(self, log):
            self.thisptr.setScalarViewRangeLog(log)

    property scalar_log_base:
        def __get__(self):
            return self.thisptr.getScalarViewRangeBase()
        def __set__(self, base):
            self.thisptr.setScalarViewRangeBase(base)

    property scalar_color_bar:
        def __get__(self):
            return self.thisptr.getScalarViewColorBar()
        def __set__(self, show):
            self.thisptr.setScalarViewColorBar(show)

    property scalar_decimal_place:
        def __get__(self):
            return self.thisptr.getScalarViewDecimalPlace()
        def __set__(self, place):
            self.thisptr.setScalarViewDecimalPlace(place)

    property scalar_auto_range:
        def __get__(self):
            return self.thisptr.getScalarViewRangeAuto()
        def __set__(self, range_auto):
            self.thisptr.setScalarViewRangeAuto(range_auto)

    property scalar_range_min:
        def __get__(self):
            return self.thisptr.getScalarViewRangeMin()
        def __set__(self, min):
            self.thisptr.setScalarViewRangeMin(min)

    property scalar_range_max:
        def __get__(self):
            return self.thisptr.getScalarViewRangeMax()
        def __set__(self, max):
            self.thisptr.setScalarViewRangeMax(max)

# particle tracing
cdef class ParticleTracing:
    cdef PyParticleTracing *thisptr

    def __cinit__(self):
        self.thisptr = new PyParticleTracing()
    def __dealloc__(self):
        del self.thisptr

    # solve
    def solve(self):
        self.thisptr.solve()

    # length
    def length(self):
        return self.thisptr.length()

    # positions - x, y, z or r, z, phi
    def positions(self):
        outx = list()
        outy = list()
        outz = list()
        cdef vector[double] x
        cdef vector[double] y
        cdef vector[double] z
        self.thisptr.positions(x, y, z)
        for i in range(self.thisptr.length()):
            outx.append(x[i])
            outy.append(y[i])
            outz.append(z[i])
        return outx, outy, outz

    # velocities - x, y, z or r, z, phi
    def velocities(self):
        outx = list()
        outy = list()
        outz = list()
        cdef vector[double] x
        cdef vector[double] y
        cdef vector[double] z
        self.thisptr.velocities(x, y, z)
        for i in range(self.thisptr.length()):
            outx.append(x[i])
            outy.append(y[i])
            outz.append(z[i])
        return outx, outy, outz

    # initial position
    property initial_position:
        def __get__(self):
            cdef double x = 0.0
            cdef double y = 0.0
            self.thisptr.initialPosition(x, y)
            return x, y
        def __set__(self, xy):
            self.thisptr.setInitialPosition(xy[0], xy[1])

    # initial velocity
    property initial_velocity:
        def __get__(self):
            cdef double x = 0.0
            cdef double y = 0.0
            self.thisptr.initialVelocity(x, y)
            return x, y
        def __set__(self, xy):
            self.thisptr.setInitialVelocity(xy[0], xy[1])

    # mass
    property mass:
        def __get__(self):
            return self.thisptr.particleMass()
        def __set__(self, mass):
            self.thisptr.setParticleMass(mass)

    # charge
    property charge:
        def __get__(self):
            return self.thisptr.particleCharge()
        def __set__(self, charge):
            self.thisptr.setParticleCharge(charge)

    # gravitational force
    property include_gravitation:
        def __get__(self):
            return self.thisptr.includeGravitation()
        def __set__(self, incl):
            self.thisptr.setIncludeGravitation(incl)

    # reflect on different material
    property reflect_on_different_material:
        def __get__(self):
            return self.thisptr.reflectOnDifferentMaterial()
        def __set__(self, reflect):
            self.thisptr.setReflectOnDifferentMaterial(reflect)

    # reflect on boundary
    property reflect_on_boundary:
        def __get__(self):
            return self.thisptr.reflectOnBoundary()
        def __set__(self, reflect):
            self.thisptr.setReflectOnBoundary(reflect)

    property coefficient_of_restitution:
        def __get__(self):
            return self.thisptr.coefficientOfRestitution()
        def __set__(self, coeff):
            self.thisptr.setCoefficientOfRestitution(coeff)

    # drag force
    property drag_force_density:
        def __get__(self):
            return self.thisptr.dragForceDensity()
        def __set__(self, rho):
            self.thisptr.setDragForceDensity(rho)

    property drag_force_reference_area:
        def __get__(self):
            return self.thisptr.dragForceReferenceArea()
        def __set__(self, area):
            self.thisptr.setDragForceReferenceArea(area)

    property drag_force_coefficient:
        def __get__(self):
            return self.thisptr.dragForceCoefficient()
        def __set__(self, coeff):
            self.thisptr.setDragForceCoefficient(coeff)

    # maximum number of steps
    property maximum_number_of_steps:
        def __get__(self):
            return self.thisptr.maximumNumberOfSteps()
        def __set__(self, steps):
            self.thisptr.setMaximumNumberOfSteps(steps)

    # tolerance
    property tolerance:
        def __get__(self):
            return self.thisptr.maximumTolerance()
        def __set__(self, tolerance):
            self.thisptr.setMaximumTolerance(tolerance)

    # minimum step
    property minimum_step:
        def __get__(self):
            return self.thisptr.minimumStep()
        def __set__(self, step):
            self.thisptr.setMinimumStep(step)

# problem
__problem__ = __Problem__()
def problem(int clear = False):
    if (clear):
        __problem__.clear()
    return __problem__

# geometry
geometry = __Geometry__()

# field
def field(char *field_id):
    return __Field__(field_id)

# config
# class __Config__:
# config = __Config__()

class __View__:
    config = __ViewConfig__()

    mesh = __ViewMesh__()
    post2d = __ViewPost2D__()
    post3d = __ViewPost3D__()

view = __View__()

# version()
def version():
    return pyVersion()

# quit()
def quit():
    pyQuit()

# input()
def input(char *str):
    return pyInput(str)

# message()
def message(char *str):
    pyMessage(str)

def open_document(char *str):
    pyOpenDocument(str)

def save_document(char *str):
    pySaveDocument(str)

def close_document():
    pyCloseDocument()

def save_image(char *str, int w = 0, int h = 0):
    pySaveImage(str, w, h)
