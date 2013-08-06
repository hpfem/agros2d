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

        void setGridStep(double step) except +
        double getGridStep()

        void setAxesShow(bool show)
        bool getAxesShow()

        void setRulersShow(bool show)
        bool getRulersShow()

        void setPostFontFamily(string &family) except +
        string getPostFontFamily()
        void setPostFontPointSize(int size) except +
        int getPostFontPointSize()

        void setRulersFontFamily(string &family) except +
        string getRulersFontFamily()
        void setRulersFontPointSize(int size) except +
        int getRulersFontPointSize()

    # PyViewMeshAndSolve
    cdef cppclass PyViewMeshAndPost:
        void setActiveTimeStep(int timeStep) except +
        int getActiveTimeStep()

        void setActiveAdaptivityStep(int adaptiveStep) except +
        int getActiveAdaptivityStep()

        void setActiveSolutionType(string &solutionType) except +
        string getActiveSolutionType()

    # PyViewMesh
    cdef cppclass PyViewMesh:
        void activate() except +

        void setField(string &fieldid) except +
        string getField()

        void setInitialMeshViewShow(bool show) except +
        bool getInitialMeshViewShow()
        void setSolutionMeshViewShow(bool show) except +
        bool getSolutionMeshViewShow()

        void setOrderViewShow(bool show) except +
        bool getOrderViewShow()
        void setOrderViewColorBar(bool show) except +
        bool getOrderViewColorBar()
        void setOrderViewLabel(bool show) except +
        bool getOrderViewLabel()
        void setOrderViewPalette(string &palette) except +
        string getOrderViewPalette()

    # PyViewPost
    cdef cppclass PyViewPost:
        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +

        bool getBoolParameter(string &parameter) except +
        int getIntParameter(string &parameter) except +
        double getDoubleParameter(string &parameter) except +
        
        void setField(string &fieldid) except +
        string getField()

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
        void setScalarViewPaletteFilter(bool filter) except +
        bool getScalarViewPaletteFilter()

        void setScalarViewRangeLog(bool log) except +
        bool getScalarViewRangeLog()
        void setScalarViewRangeBase(double base) except +
        double getScalarViewRangeBase()

        void setScalarViewColorBar(bool show) except +
        bool getScalarViewColorBar()
        void setScalarViewDecimalPlace(int place) except +
        int getScalarViewDecimalPlace()

        void setScalarViewRangeAuto(bool autoRange) except +
        bool getScalarViewRangeAuto()
        void setScalarViewRangeMin(double min) except +
        double getScalarViewRangeMin()
        void setScalarViewRangeMax(double max) except +
        double getScalarViewRangeMax()

    # PyViewPost2D
    cdef cppclass PyViewPost2D:
        void activate() except +

        void setScalarViewShow(bool show) except +
        bool getScalarViewShow()

        void setContourShow(bool show) except +
        bool getContourShow()
        void setContourCount(int count) except +
        int getContourCount()
        void setContourVariable(string &variable) except +
        string getContourVariable()

        void setVectorShow(bool show) except +
        bool getVectorShow()
        void setVectorCount(int count) except +
        int getVectorCount()
        void setVectorScale(double scale) except +
        int getVectorScale()
        void setVectorVariable(string &variable) except +
        string getVectorVariable()
        void setVectorProportional(bool show) except +
        bool getVectorProportional()
        void setVectorColor(bool show) except +
        bool getVectorColor()
        void setVectorType(string &type) except +
        string getVectorType()
        void setVectorCenter(string &center) except +
        string getVectorCenter()

        void setParticleShow(bool show) except +
        bool getParticleShow()

    # PyViewPost3D
    cdef cppclass PyViewPost3D:
        void activate() except +

        void setPost3DMode(string &mode) except +
        string getPost3DMode()

    # PyViewParticleTracing
    cdef cppclass PyViewParticleTracing:
        void activate() except +

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

    property post_font_family:
        def __get__(self):
            return self.thisptr.getPostFontFamily().c_str()
        def __set__(self, family):
            self.thisptr.setPostFontFamily(string(family))

    property post_font_size:
        def __get__(self):
            return self.thisptr.getPostFontPointSize()
        def __set__(self, size):
            self.thisptr.setPostFontPointSize(size)

    property rulers_font_family:
        def __get__(self):
            return self.thisptr.getRulersFontFamily().c_str()
        def __set__(self, family):
            self.thisptr.setRulersFontFamily(string(family))

    property rulers_font_size:
        def __get__(self):
            return self.thisptr.getRulersFontPointSize()
        def __set__(self, size):
            self.thisptr.setRulersFontPointSize(size)


# ViewMeshAndPost
cdef class __ViewMeshAndPost__:
    cdef PyViewMeshAndPost *thisptrmp

    def __cinit__(self):
        self.thisptrmp = new PyViewMeshAndPost()
    def __dealloc__(self):
        del self.thisptrmp

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

    def disable(self):
      self.initial_mesh = False
      self.solution_mesh = False
      self.order = False

    property field:
        def __get__(self):
            return self.thisptr.getField().c_str()
        def __set__(self, id):
            self.thisptr.setField(string(id))

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
    cdef object scalar_view_parameters

    def __cinit__(self):
        self.thisptrp = new PyViewPost()
        self.scalar_view_parameters = Parameters(self.__get_scalar_view_parameters__,
                                                 self.__set_scalar_view_parameters__)
    def __dealloc__(self):
        del self.thisptrp

    property field:
        def __get__(self):
            return self.thisptrp.getField().c_str()
        def __set__(self, id):
            self.thisptrp.setField(string(id))

    property scalar_view_parameters:
        def __get__(self):
            return self.scalar_view_parameters

    def __get_scalar_view_parameters__(self):
        return {'variable' : self.thisptrp.getScalarViewVariable().c_str(),
                'component' : self.thisptrp.getScalarViewVariableComp().c_str(),
                'palette' : self.thisptrp.getScalarViewPalette().c_str(),
                'quality' : self.thisptrp.getScalarViewPaletteQuality().c_str(),
                'steps' : self.thisptr.getScalarViewPaletteSteps(),
                'filter' : self.thisptr.getBoolParameter(string('ProblemSetting::View_PaletteFilter')),
                'color_bar' : self.thisptr.getBoolParameter(string('ProblemSetting::View_ShowScalarColorBar')),
                'decimal_place' : self.thisptr.getIntParameter(string('ProblemSetting::View_ScalarDecimalPlace')),
                'auto_range' : self.thisptr.getBoolParameter(string('ProblemSetting::View_ScalarRangeAuto')),
                'range_min' : self.thisptr.getDoubleParameter(string('ProblemSetting::View_ScalarRangeMin')),
                'range_max' : self.thisptr.getDoubleParameter(string('ProblemSetting::View_ScalarRangeMax')),
                'log_scale' : self.thisptr.getBoolParameter(string('ProblemSetting::View_ScalarRangeLog')),
                'log_scale_base' : self.thisptr.getDoubleParameter(string('ProblemSetting::View_ScalarRangeBase'))}

    def __set_scalar_view_parameters__(self, parameters):
        # variable, component
        self.thisptrp.setScalarViewVariable(string(parameters['variable']))
        self.thisptrp.setScalarViewVariableComp(string(parameters['component']))

        # pallete, quality
        self.thisptrp.setScalarViewPalette(string(parameters['palette']))
        self.thisptrp.setScalarViewPaletteQuality(string(parameters['quality']))

        # steps, filter
        self.thisptrp.setScalarViewPaletteSteps(parameters['steps'])
        self.thisptrp.setParameter(string('ProblemSetting::View_PaletteFilter'), <bool>parameters['filter'])

        # color bar, decimal place
        self.thisptrp.setParameter(string('ProblemSetting::View_ShowScalarColorBar'), <bool>parameters['color_bar'])
        self.thisptrp.setScalarViewDecimalPlace(parameters['decimal_place'])

        # auto range, min, max
        self.thisptrp.setParameter(string('ProblemSetting::View_ScalarRangeAuto'), <bool>parameters['auto_range'])
        self.thisptr.setParameter(string('ProblemSetting::View_ScalarRangeMin'), <double>parameters['range_min'])
        self.thisptr.setParameter(string('ProblemSetting::View_ScalarRangeMax'), <double>parameters['range_max'])

        # log scale
        self.thisptr.setParameter(string('ProblemSetting::View_ScalarRangeLog'), <bool>parameters['log_scale'])

        # log base
        if (parameters['log_scale_base'] < 0.0 or parameters['log_scale_base'] == 1):
            raise IndexError("Value of 'log_scale_base' must be possitive and can not be equal to 1.")
        self.thisptr.setParameter(string('ProblemSetting::View_ScalarRangeBase'), <double>parameters['log_scale_base'])

# ViewPost2D
cdef class __ViewPost2D__(__ViewPost__):
    cdef PyViewPost2D *thisptr

    def __cinit__(self):
        self.thisptr = new PyViewPost2D()
    def __dealloc__(self):
        del self.thisptr

    def activate(self):
        self.thisptr.activate()

    def disable(self):
      self.scalar = False
      self.contours = False
      self.vectors = False

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

    property vectors_type:
        def __get__(self):
            return self.thisptr.getVectorType().c_str()
        def __set__(self, type):
            self.thisptr.setVectorType(string(type))

    property vectors_center:
        def __get__(self):
            return self.thisptr.getVectorCenter().c_str()
        def __set__(self, center):
            self.thisptr.setVectorCenter(string(center))

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
