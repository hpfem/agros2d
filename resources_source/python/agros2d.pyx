# from hermes2d import *

# script functions
cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "../../src/pythonlabagros.h":
    # problem class
    cdef cppclass PyProblem:
        PyProblem(char*, char*, char*, char*, double, double, double)

        # name
        char *getName()
        void setName(char *name)

        # coordinate type
        char *getCoordinateType()
        void setCoordinateType(char *coordinateType)

        # mesh type
        char *getMeshType()
        void setMeshType(char *meshType)

        # matrix solver
        char *getMatrixSolver()
        void setMatrixSolver(char *matrixSolver)

        # frequency
        double getFrequency()
        void setFrequency(double frequency)

        # time step
        double getTimeStep()
        void setTimeStep(double timeStep)

        # time total
        double getTimeTotal()
        void setTimeTotal(double timeTotal)

        void solve()

    # problem class
    cdef cppclass PyField:
        PyField(char*, char*, int, int, double, char*)

        void solve()

    # version()
    char *pyVersion()


    void pythonMessage(char *str)
    char *pythonInput(char *str)
    void pythonQuit()

    char *pythonMeshFileName() except +
    char *pythonSolutionFileName() except +
    # Solution *pythonSolutionObject() except +

    void pythonNewDocument(char *name, char *type, char *physicfield,
                           int numberofrefinements, int polynomialorder, char *adaptivitytype,
                           double adaptivitysteps, double adaptivitytolerance,
                           double frequency,
                           char *analysistype, double timestep, double totaltime, double initialcondition,
                           char *linearitytype, double nonlineartolerance, int nonlinearsteps) except +
    void pythonOpenDocument(char *str) except +
    void pythonSaveDocument(char *str) except +
    void pythonCloseDocument()
    
    void pythonAddNode(double x, double y) except +
    void pythonAddEdge(double x1, double y1, double x2, double y2, char *boundary, double angle, int refine) except +
    void pythonAddLabel(double x, double y, char *material, double area, int order) except +

    void pythonDeleteNode(int index) except +
    void pythonDeleteNodePoint(double x, double y)
    void pythonDeleteEdge(int index) except +
    void pythonDeleteEdgePoint(double x1, double y1, double x2, double y2, double angle)
    void pythonDeleteLabel(int index) except +
    void pythonDeleteLabelPoint(double x, double y)

    void pythonSelectNone()
    void pythonSelectAll()

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

    void pythonZoomBestFit()
    void pythonZoomIn()
    void pythonZoomOut()
    void pythonZoomRegion(double x1, double y1, double x2, double y2)

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

# problem class
cdef class Problem:
    cdef PyProblem *thisptr

    def __cinit__(self, char *name, char *coordinate_type, char *mesh_type = "triangle", char *matrix_solver = "umfpack", double frequency=0.0, double time_step = 0.0, double time_total = 0.0):
        self.thisptr = new PyProblem(name, coordinate_type, mesh_type, matrix_solver, frequency, time_step, time_total)
    def __dealloc__(self):
        del self.thisptr

    # name
    property name:
        def __get__(self):
            return self.thisptr.getName()
        def __set__(self, name):
            self.thisptr.setName(name)

    # coordinate type
    property coordinate_type:
        def __get__(self):
            return self.thisptr.getCoordinateType()
        def __set__(self, coordinate_type):
            self.thisptr.setCoordinateType(coordinate_type)

    # mesh type
    property mesh_type:
        def __get__(self):
            return self.thisptr.getMeshType()
        def __set__(self, mesh_type):
            self.thisptr.setMeshType(mesh_type)

    # matrix solver
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

    # time step
    property time_step:
        def __get__(self):
            return self.thisptr.getTimeStep()
        def __set__(self, time_step):
            self.thisptr.setTimeStep(time_step)

    # time total
    property time_total:
        def __get__(self):
            return self.thisptr.getTimeTotal()
        def __set__(self, time_total):
            self.thisptr.setTimeTotal(time_total)

# field class
cdef class Field:
    cdef PyField *thisptr

    def __cinit__(self, char *field_id, char *analysis_type, int number_of_refinements = 1, int polynomial_order = 2, double initial_condition = 0.0, char *weak_forms = "compiled"):
        self.thisptr = new PyField(field_id, analysis_type, number_of_refinements, polynomial_order, initial_condition, weak_forms)
    def __dealloc__(self):
        del self.thisptr


    # field_id
    #property field_id:
    #    def __get__(self):
    #        return self.thisptr.getFieldId()
    #    def __set__(self, field_id):
    #        self.thisptr.setFieldId(field_id)

# version()
def version():
    return pyVersion()


def message(char *str):
    pythonMessage(str)

def input(char *str):
    return pythonInput(str)

def meshfilename():
    return pythonMeshFileName()

def solutionfilename():
    return pythonSolutionFileName()

def quit():
    pythonQuit()

# document

def newdocument(char *name, char *type, char *physicfield,
               int numberofrefinements = 0, int polynomialorder = 1, char *adaptivitytype = "disabled",
               double adaptivitysteps = 1, double adaptivitytolerance = 0,
               double frequency = 0,
               char *analysistype = "steadystate", double timestep = 0, double totaltime = 0, double initialcondition = 0,
               char *linearitytype = "newton", nonlineartolerance = 0.01, nonlinearsteps = 10):
    pythonNewDocument(name, type, physicfield,
                       numberofrefinements, polynomialorder, adaptivitytype,
                       adaptivitysteps, adaptivitytolerance,
                       frequency,
                       analysistype, timestep, totaltime, initialcondition,
                       linearitytype, nonlineartolerance, nonlinearsteps)

def opendocument(char *str):
    pythonOpenDocument(str)

def savedocument(char *str):
    pythonSaveDocument(str)

def closedocument():
    pythonCloseDocument()

# preprocessor

def addnode(double x, double y):
    pythonAddNode(x, y)

def addedge(double x1, double y1, double x2, double y2, char *boundary = "none", double angle = 0, int refine = 0):
    pythonAddEdge(x1, y1, x2, y2, boundary, angle, refine)

def addlabel(double x, double y, double area = 0, char *material = "none", int order = 0):
    pythonAddLabel(x, y, material, area, order)

def deletenode(int index):
    pythonDeleteNode(index)

def deletenodepoint(double x, double y):
    pythonDeleteNodePoint(x, y)

def deleteedge(int index):
    pythonDeleteEdge(index)

def deleteedgepoint(double x1, double y1, double x2, double y2, double angle):
    pythonDeleteEdgePoint(x1, y1, x2, y2, angle)

def deletelabel(int index):
    pythonDeleteLabel(index)

def deletelabelpoint(double x, double y):
    pythonDeleteLabelPoint(x, y)

def selectnone():
    pythonSelectNone()

def selectall():
    pythonSelectAll()

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

def mesh():
    pythonMesh()

def solve():
    pythonSolve()

def solveadaptivestep():
    pythonSolveAdaptiveStep()

# postprocessor
    
def zoombestfit():
    pythonZoomBestFit()

def zoomin():
    pythonZoomIn()

def zoomout():
    pythonZoomOut()

def zoomregion(double x1, double y1, double x2, double y2):
    pythonZoomRegion(x1, y1, x2, y2)

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

