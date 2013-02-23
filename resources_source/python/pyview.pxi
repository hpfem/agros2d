from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "../../src/pythonlab/pyview.h":
    # PyView
    cdef cppclass PyView:
        void saveImageToFile(char *file, int width, int height)  except +

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

    # PyViewPost
    cdef cppclass PyViewPost:
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

    # PyViewPost2D
    cdef cppclass PyViewPost2D:
        void activate()

        void setScalarViewShow(bool show)
        bool getScalarViewShow()

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

        void setParticleShow(bool show)
        bool getParticleShow()

    # PyViewPost3D
    cdef cppclass PyViewPost3D:
        void activate()

        void setPost3DMode(char *mode) except +
        char *getPost3DMode()

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

# ViewPost
cdef class __ViewPost__:
    cdef PyViewPost *thisptr

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

# ViewPost2D
cdef class __ViewPost2D__(__ViewPost__):
    cdef PyViewPost2D *thisptr2d

    def __cinit__(self):
        self.thisptr2d = new PyViewPost2D()
    def __dealloc__(self):
        del self.thisptr2d

    def activate(self):
        self.thisptr2d.activate()

    # scalar
    property scalar:
        def __get__(self):
            return self.thisptr2d.getScalarViewShow()
        def __set__(self, show):
            self.thisptr2d.setScalarViewShow(show)

    # contour
    property contours:
        def __get__(self):
            return self.thisptr2d.getContourShow()
        def __set__(self, show):
            self.thisptr2d.setContourShow(show)

    property contours_count:
        def __get__(self):
            return self.thisptr2d.getContourCount()
        def __set__(self, count):
            self.thisptr2d.setContourCount(count)

    property contours_variable:
        def __get__(self):
            return self.thisptr2d.getContourVariable()
        def __set__(self, variable):
            self.thisptr2d.setContourVariable(variable)

    # vector
    property vectors:
        def __get__(self):
            return self.thisptr2d.getVectorShow()
        def __set__(self, show):
            self.thisptr2d.setVectorShow(show)

    property vectors_count:
        def __get__(self):
            return self.thisptr2d.getVectorCount()
        def __set__(self, count):
            self.thisptr2d.setVectorCount(count)

    property vectors_scale:
        def __get__(self):
            return self.thisptr2d.getVectorScale()
        def __set__(self, count):
            self.thisptr2d.setVectorScale(count)

    property vectors_variable:
        def __get__(self):
            return self.thisptr2d.getVectorVariable()
        def __set__(self, variable):
            self.thisptr2d.setVectorVariable(variable)

    property vectors_proportional:
        def __get__(self):
            return self.thisptr2d.getVectorProportional()
        def __set__(self, show):
            self.thisptr2d.setVectorProportional(show)

    property vectors_color:
        def __get__(self):
            return self.thisptr2d.getVectorColor()
        def __set__(self, show):
            self.thisptr2d.setVectorColor(show)

# ViewPost3D
cdef class __ViewPost3D__(__ViewPost__):
    cdef PyViewPost3D *thisptr3d

    def activate(self):
        self.thisptr3d.activate()

    def __cinit__(self):
        self.thisptr3d = new PyViewPost3D()
    def __dealloc__(self):
        del self.thisptr3d

    # mode
    property mode:
        def __get__(self):
            return self.thisptr3d.getPost3DMode()
        def __set__(self, mode):
            self.thisptr3d.setPost3DMode(mode)

# View
cdef class __View__:
    cdef PyView *thisptr

    def __cinit__(self):
        self.thisptr = new PyView()
    def __dealloc__(self):
        del self.thisptr

    config = __ViewConfig__()

    mesh = __ViewMesh__()
    post2d = __ViewPost2D__()
    post3d = __ViewPost3D__()

    def save_image(self, char *file, int width = 0, int height = 0):
        self.thisptr.saveImageToFile(file, width, height)

view = __View__()
