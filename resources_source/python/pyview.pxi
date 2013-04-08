cdef extern from "../../agros2d-library/pythonlab/pyview.h":
    # PyView
    cdef cppclass PyView:
        void saveImageToFile(string &file, int width, int height)  except +

        void zoomBestFit()
        void zoomIn()
        void zoomOut()
        void zoomRegion(double x1, double y2, double x2, double y2)

    # PyViewConfig
    cdef cppclass PyViewConfig:
        void setGridShow(bool show)
        bool getGridShow()

        void setGridStep(double step)
        double getGridStep()

        void setAxesShow(bool show)
        bool getAxesShow()

        void setRulersShow(bool show)
        bool getRulersShow()

    # PyViewMeshAndSolve
    cdef cppclass PyViewMeshAndPost:
        void setField(string &fieldid) except +
        string getField() except +

        void setActiveTimeStep(int timeStep) except +
        int getActiveTimeStep() except +

        void setActiveAdaptivityStep(int adaptiveStep) except +
        int getActiveAdaptivityStep() except +

        void setActiveSolutionType(string &solutionType) except +
        string getActiveSolutionType() except +

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
        void setOrderViewPalette(string &palette) except +
        string getOrderViewPalette()

    # PyViewPost
    cdef cppclass PyViewPost:
        void setScalarViewVariable(string &variable) except +
        string getScalarViewVariable()
        void setScalarViewVariableComp(string &component) except +
        string getScalarViewVariableComp()
        void setScalarViewPalette(string &palette) except +
        string getScalarViewPalette()
        void setScalarViewPaletteQuality(string &quality) except +
        string getScalarViewPaletteQuality()

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
        void setContourVariable(string &variable) except +
        string getContourVariable()

        void setVectorShow(bool show)
        bool getVectorShow()
        void setVectorCount(int count) except +
        int getVectorCount()
        void setVectorScale(double scale) except +
        int getVectorScale()
        void setVectorVariable(string &variable) except +
        string getVectorVariable()
        void setVectorProportional(bool show)
        bool getVectorProportional()
        void setVectorColor(bool show)
        bool getVectorColor()

        void setParticleShow(bool show)
        bool getParticleShow()

    # PyViewPost3D
    cdef cppclass PyViewPost3D:
        void activate()

        void setPost3DMode(string &mode) except +
        string getPost3DMode()

    # PyViewParticleTracing
    cdef cppclass PyViewParticleTracing:
        void activate()

# ViewConfig
cdef class __ViewConfig__:
    cdef PyViewConfig *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewConfig()
    def __dealloc__(self):
        del self.thisptr

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

# ViewMeshAndPost
cdef class __ViewMeshAndPost__:
    cdef PyViewMeshAndPost *thisptrmp

    def __cinit__(self):
        self.thisptrmp = new PyViewMeshAndPost()
    def __dealloc__(self):
        del self.thisptrmp

    property field:
        def __get__(self):
            return self.thisptrmp.getField().c_str()
        def __set__(self, id):
            self.thisptrmp.setField(string(id))

    property time_step:
        def __get__(self):
            return self.thisptrmp.getActiveTimeStep()
        def __set__(self, time_step):
            self.thisptrmp.setActiveTimeStep(time_step)

    property adaptivity_step:
        def __get__(self):
            return self.thisptrmp.getActiveAdaptivityStep()
        def __set__(self, adaptivity_step):
            self.thisptrmp.setActiveAdaptivityStep(adaptivity_step)

    property solution_type:
        def __get__(self):
            return self.thisptrmp.getActiveSolutionType().c_str()
        def __set__(self, solution_type):
            self.thisptrmp.setActiveSolutionType(string(solution_type))

# ViewMesh
cdef class __ViewMesh__(__ViewMeshAndPost__):
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
            return self.thisptr.getOrderViewPalette().c_str()
        def __set__(self, palette):
            self.thisptr.setOrderViewPalette(string(palette))

# ViewPost
cdef class __ViewPost__(__ViewMeshAndPost__):
    cdef PyViewPost *thisptrp

    def __cinit__(self):
        self.thisptrp = new PyViewPost()
    def __dealloc__(self):
        del self.thisptrp

    property scalar_variable:
        def __get__(self):
            return self.thisptrp.getScalarViewVariable().c_str()
        def __set__(self, variable):
            self.thisptrp.setScalarViewVariable(string(variable))

    property scalar_component:
        def __get__(self):
            return self.thisptrp.getScalarViewVariableComp().c_str()
        def __set__(self, component):
            self.thisptrp.setScalarViewVariableComp(string(component))

    property scalar_palette:
        def __get__(self):
            return self.thisptrp.getScalarViewPalette().c_str()
        def __set__(self, palette):
            self.thisptrp.setScalarViewPalette(string(palette))

    property scalar_palette_quality:
        def __get__(self):
            return self.thisptrp.getScalarViewPaletteQuality().c_str()
        def __set__(self, quality):
            self.thisptrp.setScalarViewPaletteQuality(string(quality))

    property scalar_palette_steps:
        def __get__(self):
            return self.thisptrp.getScalarViewPaletteSteps()
        def __set__(self, steps):
            self.thisptrp.setScalarViewPaletteSteps(steps)

    property scalar_palette_filter:
        def __get__(self):
            return self.thisptrp.getScalarViewPaletteFilter()
        def __set__(self, filter):
            self.thisptrp.setScalarViewPaletteFilter(filter)

    property scalar_log_scale:
        def __get__(self):
            return self.thisptrp.getScalarViewRangeLog()
        def __set__(self, log):
            self.thisptrp.setScalarViewRangeLog(log)

    property scalar_log_base:
        def __get__(self):
            return self.thisptrp.getScalarViewRangeBase()
        def __set__(self, base):
            self.thisptrp.setScalarViewRangeBase(base)

    property scalar_color_bar:
        def __get__(self):
            return self.thisptrp.getScalarViewColorBar()
        def __set__(self, show):
            self.thisptrp.setScalarViewColorBar(show)

    property scalar_decimal_place:
        def __get__(self):
            return self.thisptrp.getScalarViewDecimalPlace()
        def __set__(self, place):
            self.thisptrp.setScalarViewDecimalPlace(place)

    property scalar_auto_range:
        def __get__(self):
            return self.thisptrp.getScalarViewRangeAuto()
        def __set__(self, range_auto):
            self.thisptrp.setScalarViewRangeAuto(range_auto)

    property scalar_range_min:
        def __get__(self):
            return self.thisptrp.getScalarViewRangeMin()
        def __set__(self, min):
            self.thisptrp.setScalarViewRangeMin(min)

    property scalar_range_max:
        def __get__(self):
            return self.thisptrp.getScalarViewRangeMax()
        def __set__(self, max):
            self.thisptrp.setScalarViewRangeMax(max)

# ViewPost2D
cdef class __ViewPost2D__(__ViewPost__):
    cdef PyViewPost2D *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewPost2D()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    property scalar:
        def __get__(self):
            return self.thisptr.getScalarViewShow()
        def __set__(self, show):
            self.thisptr.setScalarViewShow(show)

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
            return self.thisptr.getContourVariable().c_str()
        def __set__(self, variable):
            self.thisptr.setContourVariable(string(variable))

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
            return self.thisptr.getVectorVariable().c_str()
        def __set__(self, variable):
            self.thisptr.setVectorVariable(string(variable))

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
cdef class __ViewPost3D__(__ViewPost__):
    cdef PyViewPost3D *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewPost3D()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    property mode:
        def __get__(self):
            return self.thisptr.getPost3DMode().c_str()
        def __set__(self, mode):
            self.thisptr.setPost3DMode(string(mode))

# ViewParticleTracing
cdef class __ViewParticleTracing__:
    cdef PyViewParticleTracing *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewParticleTracing()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

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
    particle_tracing = __ViewParticleTracing__()

    def save_image(self, file, width = 0, height = 0):
        self.thisptr.saveImageToFile(string(file), width, height)

    def zoom_best_fit(self):
        self.thisptr.zoomBestFit()

    def zoom_in(self):
        self.thisptr.zoomIn()

    def zoom_out(self):
        self.thisptr.zoomOut()

    def zoom_region(self, x1, y1, x2, y2):
        self.thisptr.zoomRegion(x1, y1, x2, y2)

view = __View__()
