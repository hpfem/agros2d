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

        double getTimeStep()
        void setTimeStep(double timeStep) except +

        double getTimeTotal()
        void setTimeTotal(double timeTotal) except +

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

        char *getWeakForms()
        void setWeakForms(char*) except +

        void addBoundary(char*, char*, map[char*, double]) except +
        void setBoundary(char*, char*, map[char*, double]) except +
        void removeBoundary(char*)

        void addMaterial(char*, map[char*, double]) except +
        void setMaterial(char*, map[char*, double]) except +
        void removeMaterial(char*)

        void solve()

        void localValues(double x, double y, map[string, double] results) except +
        void surfaceIntegrals(vector[int], map[string, double] results) except +
        void volumeIntegrals(vector[int], map[string, double] results) except +

    # PyGeometry
    cdef cppclass PyGeometry:
        PyGeometry()

        void activate()

        void addNode(double, double) except +
        void addEdge(double, double, double, double, double, int, map[char*, char*]) except +
        void addLabel(double, double, double, int, map[char*, char*]) except +

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

    # PyViewMesh
    cdef cppclass PyViewMesh:
        void activate()
        void refresh()

        void setInitialMeshViewShow(int show)  except +
        int getInitialMeshViewShow()
        void setSolutionMeshViewShow(int show) except +
        int getSolutionMeshViewShow()

        void setOrderViewShow(int show) except +
        int getOrderViewShow()
        void setOrderViewColorBar(int show) except +
        int getOrderViewColorBar()
        void setOrderViewLabel(int show) except +
        int getOrderViewLabel()
        void setOrderViewPalette(char *palette) except +
        char* getOrderViewPalette()

    # PyViewPost2D
    cdef cppclass PyViewPost2D:
        void activate()
        void refresh()

        void setScalarViewShow(bool show) except +
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
        void setScalarViewPaletteFilter(bool filter) except +
        bool getScalarViewPaletteFilter()

        void setScalarViewRangeLog(bool log) except +
        bool getScalarViewRangeLog()
        void setScalarViewRangeBase(double base) except +
        double getScalarViewRangeBase()

        void setScalarViewColorBar(int show) except +
        int getScalarViewColorBar()
        void setScalarViewDecimalPlace(int place) except +
        int getScalarViewDecimalPlace()

        void setScalarViewRangeAuto(int autoRange) except +
        int getScalarViewRangeAuto()
        void setScalarViewRangeMin(double min) except +
        double getScalarViewRangeMin()
        void setScalarViewRangeMax(double max) except +
        double getScalarViewRangeMax()

        void setContourShow(bool show) except +
        bool getContourShow()
        void setContourCount(int count) except +
        int getContourCount()
        void setContourVariable(char *variable) except +
        char *getContourVariable()

        void setVectorShow(bool show) except +
        bool getVectorShow()
        void setVectorCount(int count) except +
        int getVectorCount()
        void setVectorScale(double scale) except +
        int getVectorScale()
        void setVectorVariable(char *variable) except +
        char *getVectorVariable()
        void setVectorProportional(bool show) except +
        bool getVectorProportional()
        void setVectorColor(bool show) except +
        bool getVectorColor()

    # PyViewPost3D
    cdef cppclass PyViewPost3D:
        void activate()
        void refresh()

    char *pyVersion()
    void pyQuit()

    char *pyInput(char *str)
    void pyMessage(char *str)

    void pyOpenDocument(char *str) except +
    void pySaveDocument(char *str) except +
    void pyCloseDocument()

    void pySaveImage(char *str, int w, int h) except +

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
    def add_label(self, double x, double y, double area = 0.0, int order = 0, materials = {}):

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

# View
cdef class __ViewConfig__:
    cdef PyViewConfig *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewConfig()
    def __dealloc__(self):
        del self.thisptr

    property field:
        def __get__(self):
            return self.thisptr.getField()
        def __set__(self, fieldid):
            self.thisptr.setField(fieldid)

cdef class __ViewMesh__:
    cdef PyViewMesh *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewMesh()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    def refresh(self):
        self.thisptr.refresh()

    property solution_mesh_view_show:
        def __get__(self):
            return self.thisptr.getSolutionMeshViewShow()
        def __set__(self, show):
            self.thisptr.setSolutionMeshViewShow(show)

    property initial_mesh_view_show:
        def __get__(self):
            return self.thisptr.getInitialMeshViewShow()
        def __set__(self, show):
            self.thisptr.setInitialMeshViewShow(show)

    property order_view_show:
        def __get__(self):
            return self.thisptr.getOrderViewShow()
        def __set__(self, show):
            self.thisptr.setOrderViewShow(show)

    property order_view_color_bar:
        def __get__(self):
            return self.thisptr.getOrderViewColorBar()
        def __set__(self, show):
            self.thisptr.setOrderViewColorBar(show)

    property order_view_label:
        def __get__(self):
            return self.thisptr.getOrderViewLabel()
        def __set__(self, show):
            self.thisptr.setOrderViewLabel(show)

    property order_view_palette:
        def __get__(self):
            return self.thisptr.getOrderViewPalette()
        def __set__(self, palette):
            self.thisptr.setOrderViewPalette(palette)

cdef class __ViewPost2D__:
    cdef PyViewPost2D *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewPost2D()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    def refresh(self):
        self.thisptr.refresh()

    # scalar view
    property scalar_view_show:
        def __get__(self):
            return self.thisptr.getScalarViewShow()
        def __set__(self, show):
            self.thisptr.setScalarViewShow(show)

    property scalar_view_variable:
        def __get__(self):
            return self.thisptr.getScalarViewVariable()
        def __set__(self, variable):
            self.thisptr.setScalarViewVariable(variable)

    property scalar_view_component:
        def __get__(self):
            return self.thisptr.getScalarViewVariableComp()
        def __set__(self, component):
            self.thisptr.setScalarViewVariableComp(component)

    property scalar_view_palette:
        def __get__(self):
            return self.thisptr.getScalarViewPalette()
        def __set__(self, palette):
            self.thisptr.setScalarViewPalette(palette)

    property scalar_view_palette_quality:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteQuality()
        def __set__(self, quality):
            self.thisptr.setScalarViewPaletteQuality(quality)

    property scalar_view_palette_steps:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteSteps()
        def __set__(self, steps):
            self.thisptr.setScalarViewPaletteSteps(steps)

    property scalar_view_palette_filter:
        def __get__(self):
            return self.thisptr.getScalarViewPaletteFilter()
        def __set__(self, filter):
            self.thisptr.setScalarViewPaletteFilter(filter)

    property scalar_view_log_scale:
        def __get__(self):
            return self.thisptr.getScalarViewRangeLog()
        def __set__(self, log):
            self.thisptr.setScalarViewRangeLog(log)

    property scalar_view_log_base:
        def __get__(self):
            return self.thisptr.getScalarViewRangeBase()
        def __set__(self, base):
            self.thisptr.setScalarViewRangeBase(base)

    property scalar_view_color_bar:
        def __get__(self):
            return self.thisptr.getScalarViewColorBar()
        def __set__(self, show):
            self.thisptr.setScalarViewColorBar(show)

    property scalar_view_decimal_place:
        def __get__(self):
            return self.thisptr.getScalarViewDecimalPlace()
        def __set__(self, place):
            self.thisptr.setScalarViewDecimalPlace(place)

    property scalar_view_auto_range:
        def __get__(self):
            return self.thisptr.getScalarViewRangeAuto()
        def __set__(self, range_auto):
            self.thisptr.setScalarViewRangeAuto(range_auto)

    property scalar_view_range_min:
        def __get__(self):
            return self.thisptr.getScalarViewRangeMin()
        def __set__(self, min):
            self.thisptr.setScalarViewRangeMin(min)

    property scalar_view_range_max:
        def __get__(self):
            return self.thisptr.getScalarViewRangeMax()
        def __set__(self, max):
            self.thisptr.setScalarViewRangeMax(max)

    # contour
    property contour_show:
        def __get__(self):
            return self.thisptr.getContourShow()
        def __set__(self, show):
            self.thisptr.setContourShow(show)

    property contour_count:
        def __get__(self):
            return self.thisptr.getContourCount()
        def __set__(self, count):
            self.thisptr.setContourCount(count)

    property contour_variable:
        def __get__(self):
            return self.thisptr.getContourVariable()
        def __set__(self, variable):
            self.thisptr.setContourVariable(variable)

    # vector
    property vector_show:
        def __get__(self):
            return self.thisptr.getVectorShow()
        def __set__(self, show):
            self.thisptr.setVectorShow(show)

    property vector_count:
        def __get__(self):
            return self.thisptr.getVectorCount()
        def __set__(self, count):
            self.thisptr.setVectorCount(count)

    property vector_scale:
        def __get__(self):
            return self.thisptr.getVectorScale()
        def __set__(self, count):
            self.thisptr.setVectorScale(count)

    property vector_variable:
        def __get__(self):
            return self.thisptr.getVectorVariable()
        def __set__(self, variable):
            self.thisptr.setVectorVariable(variable)

    property vector_proportional:
        def __get__(self):
            return self.thisptr.getVectorProportional()
        def __set__(self, show):
            self.thisptr.setVectorProportional(show)

    property vector_color:
        def __get__(self):
            return self.thisptr.getVectorColor()
        def __set__(self, show):
            self.thisptr.setVectorColor(show)

cdef class __ViewPost3D__:
    cdef PyViewPost3D *thisptr

    def activate(self):
        self.thisptr.activate()

    def refresh(self):
        self.thisptr.refresh()

    def __cinit__(self):
        self.thisptr = new PyViewPost3D()
    def __dealloc__(self):
        del self.thisptr

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
