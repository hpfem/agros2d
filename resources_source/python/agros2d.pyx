from libcpp.vector cimport vector
from libcpp.map cimport map
from libcpp.pair cimport pair

cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "../../src/pythonlabagros.h":
    # PyProblem
    cdef cppclass PyProblem:
        PyProblem(char*, char*, char*, char*, double, double, double) except +

        char *getName()
        void setName(char *name)

        char *getCoordinateType()
        void setCoordinateType(char *coordinateType)

        char *getMeshType()
        void setMeshType(char *meshType)

        char *getMatrixSolver()
        void setMatrixSolver(char *matrixSolver)

        double getFrequency()
        void setFrequency(double frequency)

        double getTimeStep()
        void setTimeStep(double timeStep)

        double getTimeTotal()
        void setTimeTotal(double timeTotal)

        void solve()

    # PyField
    cdef cppclass PyField:
        PyField(char*, char*, int, int, char*, double, int, char*, double, int, double, char*)  except +

        char *getFieldId()

        char *getAnalysisType()
        void setAnalysisType(char*)

        int getNumberOfRefinemens()
        void setNumberOfRefinemens(int)

        int getPolynomialOrder()
        void setPolynomialOrder(int)

        char *getLinearityType()
        void setLinearityType(char*)

        double getNonlinearTolerance()
        void setNonlinearTolerance(double)

        int getNonlinearSteps()
        void setNonlinearSteps(int)

        char *getAdaptivityType()
        void setAdaptivityType(char*)

        double getAdaptivityTolerance()
        void setAdaptivityTolerance(double)

        int getAdaptivitySteps()
        void setAdaptivitySteps(int)

        double getInitialCondition()
        void setInitialCondition(double)

        char *getWeakForms()
        void setWeakForms(char*)

        void addBoundary(char*, char*, map[char*, double]) except +
        void setBoundary(char*, char*, map[char*, double]) except +
        void removeBoundary(char*)

        void addMaterial(char*, map[char*, double]) except +
        void setMaterial(char*, map[char*, double]) except +
        void removeMaterial(char*)

        void solve()

    # PyGeometry
    cdef cppclass PyGeometry:
        PyGeometry()

        void addNode(double, double) except +
        void addEdge(double, double, double, double, double, int, map[char*, char*]) except +
        void addLabel(double, double, double, int, map[char*, char*]) except +

        void removeNode(int index) except +
        void removeEdge(int index) except +
        void removeLabel(int index) except +

        void selectNodes(vector[int]) except +
        void selectEdges(vector[int]) except +
        void selectLabels(vector[int]) except +
        void selectNone()

        void mesh()

        void zoomBestFit()
        void zoomIn()
        void zoomOut()
        void zoomRegion(double, double, double, double)

    char *pyVersion()
    char *pyInput(char *str)
    void pyMessage(char *str)
    void pyQuit()



    char *pythonMeshFileName() except +
    char *pythonSolutionFileName() except +
    # Solution *pythonSolutionObject() except +

    void pythonOpenDocument(char *str) except +
    void pythonSaveDocument(char *str) except +
    void pythonCloseDocument()

    void pythonDeleteNodePoint(double x, double y)
    void pythonDeleteEdgePoint(double x1, double y1, double x2, double y2, double angle)
    void pythonDeleteLabelPoint(double x, double y)

    void pythonSelectNodePoint(double x, double y)
    void pythonSelectEdgePoint(double x, double y)
    void pythonSelectLabelPoint(double x, double y)

    void pythonRotateSelection(double x, double y, double angle, int copy)
    void pythonScaleSelection(double x, double y, double scale, int copy)
    void pythonMoveSelection(double dx, double dy, int copy)
    void pythonDeleteSelection()

    void pythonMesh()
    void pythonSolve()
    void pythonSolveAdaptiveStep()

    void pythonMode(char *str) except +
    void pythonPostprocessorMode(char *str) except +

    void pythonShowScalar(char *type, char *variable, char *component, double rangemin, double rangemax) except +
    void pythonShowGrid(int show)
    void pythonShowGeometry(int show)
    void pythonShowInitialMesh(int show)
    void pythonShowSolutionMesh(int show)
    void pythonShowContours(int show)
    void pythonShowVectors(int show)

    void pythonSetTimeStep(int timestep) except +
    int pythonTimeStepCount()
    void pythonSaveImage(char *str, int w, int h) except +

# Problem
cdef class Problem:
    cdef PyProblem *thisptr

    # Problem(coordinate_type, name, mesh_type, matrix_solver, frequency, time_step, time_total)
    def __cinit__(self, char *coordinate_type, char *name = "", char *mesh_type = "triangle", char *matrix_solver = "umfpack",
                  double frequency=0.0, double time_step = 0.0, double time_total = 0.0):
        self.thisptr = new PyProblem(coordinate_type, name, mesh_type, matrix_solver, frequency, time_step, time_total)

    def __dealloc__(self):
        del self.thisptr

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

    # time_step
    property time_step:
        def __get__(self):
            return self.thisptr.getTimeStep()
        def __set__(self, time_step):
            self.thisptr.setTimeStep(time_step)

    # time_total
    property time_total:
        def __get__(self):
            return self.thisptr.getTimeTotal()
        def __set__(self, time_total):
            self.thisptr.setTimeTotal(time_total)

# Field
cdef class Field:
    cdef PyField *thisptr

    # Field(field_id, analysis_type, number_of_refinements, polynomial_order, linearity_type, nonlinear_tolerance, nonlinear_steps, adaptivity_type, adaptivity_tolerance, adaptivity_steps, initial_condition, weak_forms)
    def __cinit__(self, char *field_id, char *analysis_type, int number_of_refinements = 0, int polynomial_order = 1, char *linearity_type = "linear",
                  double nonlinear_tolerance = 0.001, int nonlinear_steps = 10, char *adaptivity_type = "disabled", double adaptivity_tolerance = 1,
                  int adaptivity_steps = 1, double initial_condition = 0.0, char *weak_forms = "compiled"):
        self.thisptr = new PyField(field_id, analysis_type, number_of_refinements, polynomial_order, linearity_type, nonlinear_tolerance,
                                   nonlinear_steps, adaptivity_type, adaptivity_tolerance, adaptivity_steps, initial_condition, weak_forms)
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
            return self.thisptr.getNumberOfRefinemens()
        def __set__(self, number_of_refinements):
            self.thisptr.setNumberOfRefinemens(number_of_refinements)

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

    # weak_forms
    property weak_forms:
        def __get__(self):
            return self.thisptr.getWeakForms()
        def __set__(self, weak_forms):
            self.thisptr.setWeakForms(weak_forms)

    # boundaries
    def add_boundary(self, char *name, char *type, parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter
        for key in parameters:
            parameter.first = key
            parameter.second = parameters[key]
            parameters_map.insert(parameter)

        self.thisptr.addBoundary(name, type, parameters_map)

    def set_boundary(self, char *name, char *type = "", parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter
        for key in parameters:
            parameter.first = key
            parameter.second = parameters[key]
            parameters_map.insert(parameter)

        self.thisptr.setBoundary(name, type, parameters_map)

    def remove_boundary(self, char *name):
        self.thisptr.removeBoundary(name)

    # materials
    def add_material(self, char *name, parameters = {}):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter
        for key in parameters:
            parameter.first = key
            parameter.second = parameters[key]
            parameters_map.insert(parameter)

        self.thisptr.addMaterial(name, parameters_map)

    def set_material(self, char *name, parameters):
        cdef map[char*, double] parameters_map
        cdef pair[char*, double] parameter
        for key in parameters:
            parameter.first = key
            parameter.second = parameters[key]
            parameters_map.insert(parameter)

        self.thisptr.setMaterial(name, parameters_map)

    def remove_material(self, char *name):
        self.thisptr.removeMaterial(name)

# Geometry
cdef class Geometry:
    cdef PyGeometry *thisptr

    # Geometry()
    def __cinit__(self):
        self.thisptr = new PyGeometry()
    def __dealloc__(self):
        del self.thisptr

    # add_node(x, y)
    def add_node(self, double x, double y):
        self.thisptr.addNode(x, y)

    # remove_node(index)
    def remove_node(self, int index):
        self.thisptr.removeNode(index)

    # add_edge(x1, y1, x2, y2, angle, refinement, boundaries)
    def add_edge(self, double x1, double y1, double x2, double y2, double angle = 0.0, int refinement = 0, boundaries = {}):

        cdef map[char*, char*] boundaries_map
        cdef pair[char*, char *] boundary
        for key in boundaries:
            boundary.first = key
            boundary.second = boundaries[key]
            boundaries_map.insert(boundary)

        self.thisptr.addEdge(x1, y1, x2, y2, angle, refinement, boundaries_map)

    # remove_edge(index)
    def remove_edge(self, int index):
        self.thisptr.removeEdge(index)

    # add_label(x, y, area, order, materials)
    def add_label(self, double x, double y, double area = 0.0, int order = 1, materials = {}):

        cdef map[char*, char*] materials_map
        cdef pair[char*, char *] material
        for key in materials:
            material.first = key
            material.second = materials[key]
            materials_map.insert(material)

        self.thisptr.addLabel(x, y, area, order, materials_map)

    # remove_label(index)
    def remove_label(self, int index):
        self.thisptr.removeLabel(index)

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

    # select_none()
    def select_none(self):
        self.thisptr.selectNone()

    # mesh()
    def mesh(self):
        self.thisptr.mesh()

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

# version()
def version():
    return pyVersion()

# input()
def input(char *str):
    return pyInput(str)

# message()
def message(char *str):
    pyMessage(str)

# quit()
def quit():
    pyQuit()



def meshfilename():
    return pythonMeshFileName()

def solutionfilename():
    return pythonSolutionFileName()

# document

def opendocument(char *str):
    pythonOpenDocument(str)

def savedocument(char *str):
    pythonSaveDocument(str)

def closedocument():
    pythonCloseDocument()

# preprocessor

def deletenodepoint(double x, double y):
    pythonDeleteNodePoint(x, y)

def deleteedgepoint(double x1, double y1, double x2, double y2, double angle):
    pythonDeleteEdgePoint(x1, y1, x2, y2, angle)

def deletelabelpoint(double x, double y):
    pythonDeleteLabelPoint(x, y)

def selectnodepoint(double x, double y):
    pythonSelectNodePoint(x, y)

def selectedgepoint(double x, double y):
    pythonSelectEdgePoint(x, y)
    
def selectlabelpoint(double x, double y):
    pythonSelectLabelPoint(x, y)

def rotateselection(double x, double y, double angle, int copy = False):
    pythonRotateSelection(x, y, angle, int(copy))

def scaleselection(double x, double y, double scale, int copy = False):
    pythonScaleSelection(x, y, scale, int(copy))

def moveselection(double dx, double dy, int copy = False):
    pythonMoveSelection(dx, dy, int(copy))

def deleteselection():
    pythonDeleteSelection()

# solver

def solve():
    pythonSolve()

def solveadaptivestep():
    pythonSolveAdaptiveStep()

# postprocessor

def mode(char *str):
    pythonMode(str)

def postprocessormode(char *str):
    pythonPostprocessorMode(str)

def showscalar(char *type, char *variable = "default", char *component = "default", double rangemin = -123456, double rangemax = -123456):
    pythonShowScalar(type, variable, component, rangemin, rangemax)

def showgrid(int show):
    pythonShowGrid(int(show))

def showgeometry(int show):
    pythonShowGeometry(int(show))

def showinitialmesh(int show):
    pythonShowInitialMesh(int(show))

def showsolutionmesh(int show):
    pythonShowSolutionMesh(int(show))

def showcontours(int show):
    pythonShowContours(int(show))

def showvectors(int show):
    pythonShowVectors(int(show))

def timestep(int timestep):
    pythonSetTimeStep(timestep)

def timestepcount():
    return pythonTimeStepCount()

def saveimage(char *str, int w = 0, int h = 0):
    pythonSaveImage(str, w, h)

