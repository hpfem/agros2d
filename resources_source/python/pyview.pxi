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
        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +

        bool getBoolParameter(string &parameter)
        int getIntParameter(string &parameter)
        double getDoubleParameter(string &parameter)

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
        void setParameter(string &parameter, bool value) except +
        bool getBoolParameter(string &parameter)
        int getIntParameter(string &parameter)

        void activate() except +

        void setField(string &fieldid) except +
        string getField()

        void setInitialMeshViewShow(bool show) except +
        bool getInitialMeshViewShow()
        void setSolutionMeshViewShow(bool show) except +
        bool getSolutionMeshViewShow()

        void setOrderViewShow(bool show) except +
        bool getOrderViewShow()
        void setOrderViewPalette(string &palette) except +
        string getOrderViewPalette()

        void setOrderComponent(int component) except +

    # PyViewPost
    cdef cppclass PyViewPost:
        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +

        bool getBoolParameter(string &parameter)
        int getIntParameter(string &parameter)
        double getDoubleParameter(string &parameter)

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

    # PyViewPost2D
    cdef cppclass PyViewPost2D:
        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +

        bool getBoolParameter(string &parameter)
        int getIntParameter(string &parameter)
        double getDoubleParameter(string &parameter)

        void activate() except +

        void setScalarViewShow(bool show) except +
        bool getScalarViewShow()
        void exportScalarVTK(string &filename)

        void setContourShow(bool show) except +
        bool getContourShow()
        void setContourVariable(string &variable) except +
        string getContourVariable()
        void exportContourVTK(string &filename)

        void setVectorShow(bool show) except +
        bool getVectorShow()
        void setVectorVariable(string &variable) except +
        string getVectorVariable()
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
    cdef object workspace_parameters

    def __cinit__(self):
        self.thisptr = new PyViewConfig()
        self.workspace_parameters = __Parameters__(self.__get_workspace_parameters__,
                                                   self.__set_workspace_parameters__)

    def __dealloc__(self):
        del self.thisptr

    property workspace_parameters:
        def __get__(self):
            return self.workspace_parameters.get_parameters()

    def __get_workspace_parameters__(self):
        return {'grid' : self.thisptr.getBoolParameter(string('View_ShowGrid')),
                'grid_step' : self.thisptr.getDoubleParameter(string('View_GridStep')),
                'axes' : self.thisptr.getBoolParameter(string('View_ShowAxes')),
                'rulers' : self.thisptr.getBoolParameter(string('View_ShowRulers')),
                'post_font_family' : self.thisptr.getPostFontFamily().c_str(),
                'rulers_font_family' : self.thisptr.getRulersFontFamily().c_str(),
                'post_font_size' : self.thisptr.getIntParameter(string('View_PostFontPointSize')),
                'rulers_font_size' : self.thisptr.getIntParameter(string('View_RulersFontPointSize')),
                'node_size' : self.thisptr.getIntParameter(string('View_NodeSize')),
                'edge_width' : self.thisptr.getIntParameter(string('View_EdgeWidth')),
                'label_size' : self.thisptr.getIntParameter(string('View_LabelSize')),
                '3d_angle' : self.thisptr.getDoubleParameter(string('View_ScalarView3DAngle')),
                '3d_height' : self.thisptr.getDoubleParameter(string('View_ScalarView3DHeight')),
                '3d_lighting' : self.thisptr.getBoolParameter(string('View_ScalarView3DLighting')),
                '3d_gradient_background' : self.thisptr.getBoolParameter(string('View_ScalarView3DBackground')),
                '3d_bounding_box' : self.thisptr.getBoolParameter(string('View_ScalarView3DBoundingBox')),
                '3d_edges' : self.thisptr.getBoolParameter(string('View_ScalarView3DSolidGeometry')),
                'scalar_view_deform' : self.thisptr.getBoolParameter(string('View_DeformScalar')),
                'contour_view_deform' : self.thisptr.getBoolParameter(string('View_DeformContour')),
                'vector_view_deform' : self.thisptr.getBoolParameter(string('View_DeformVector'))}

    def __set_workspace_parameters__(self, parameters):
        # grid, grid step
        self.thisptr.setParameter(string('View_ShowGrid'), <bool>parameters['grid'])
        positive_value(parameters['grid_step'], 'grid_step')
        self.thisptr.setParameter(string('View_GridStep'), <double>parameters['grid_step'])

        # axes, rulers
        self.thisptr.setParameter(string('View_ShowAxes'), <bool>parameters['axes'])
        self.thisptr.setParameter(string('View_ShowRulers'), <bool>parameters['rulers'])

        # fonts
        self.thisptr.setPostFontFamily(string(parameters['post_font_family']))
        self.thisptr.setRulersFontFamily(string(parameters['rulers_font_family']))
        value_in_range(parameters['post_font_size'], 6, 40, 'post_font_size')
        self.thisptr.setParameter(string('PostFontPointSize'), <int>parameters['post_font_size'])
        value_in_range(parameters['rulers_font_size'], 6, 40, 'rulers_font_size')
        self.thisptr.setParameter(string('RulersFontPointSize'), <int>parameters['rulers_font_size'])

        # elements size
        value_in_range(parameters['node_size'], 1, 20, 'node_size')
        self.thisptr.setParameter(string('View_NodeSize'), <int>parameters['node_size'])
        value_in_range(parameters['edge_width'], 1, 20, 'edge_width')
        self.thisptr.setParameter(string('View_NodeSize'), <int>parameters['edge_width'])
        value_in_range(parameters['label_size'], 1, 20, 'label_size')
        self.thisptr.setParameter(string('View_LabelSize'), <int>parameters['label_size'])

        # 3d view
        value_in_range(parameters['3d_angle'], 30.0, 360.0, '3d_angle')
        self.thisptr.setParameter(string('View_ScalarView'), <double>parameters['3d_angle'])
        value_in_range(parameters['3d_height'], 0.2, 10.0, '3d_height')
        self.thisptr.setParameter(string('View_ScalarView3DHeight'), <double>parameters['3d_height'])
        self.thisptr.setParameter(string('View_ScalarView3DLighting'), <bool>parameters['3d_lighting'])
        self.thisptr.setParameter(string('View_ScalarView3DBackground'), <bool>parameters['3d_gradient_background'])
        self.thisptr.setParameter(string('View_ScalarView3DBoundingBox'), <bool>parameters['3d_bounding_box'])
        self.thisptr.setParameter(string('View_ScalarView3DSolidGeometry'), <bool>parameters['3d_edges'])

        # deform shape
        self.thisptr.setParameter(string('View_DeformScalar'), <double>parameters['scalar_view_deform'])
        self.thisptr.setParameter(string('View_DeformContour'), <double>parameters['contour_view_deform'])
        self.thisptr.setParameter(string('View_DeformVector'), <double>parameters['vector_view_deform'])


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
    cdef object order_view_parameters

    def __cinit__(self):
        self.thisptr = new PyViewMesh()
        self.order_view_parameters = __Parameters__(self.__get_order_view_parameters__,
                                                    self.__set_order_view_parameters__)

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

    # order
    property order:
        def __get__(self):
            return self.thisptr.getOrderViewShow()
        def __set__(self, show):
            self.thisptr.setOrderViewShow(show)

    property order_view_parameters:
        def __get__(self):
            return self.order_view_parameters.get_parameters()

    def __get_order_view_parameters__(self):
        return {'palette' : self.thisptr.getOrderViewPalette().c_str(),
                'color_bar' : self.thisptr.getBoolParameter(string('View_ShowOrderColorBar')),
                'label' : self.thisptr.getBoolParameter(string('View_ShowOrderLabel')),
                'component' : self.thisptr.getIntParameter(string('View_OrderComponent'))}

    def __set_order_view_parameters__(self, parameters):
        # palette
        self.thisptr.setOrderViewPalette(string(parameters['palette']))

        # color bar, label
        self.thisptr.setParameter(string('View_ShowOrderColorBar'), <bool>parameters['color_bar'])
        self.thisptr.setParameter(string('View_ShowOrderLabel'), <bool>parameters['label'])

        # component
        self.thisptr.setOrderComponent(parameters['component'])

# ViewPost
cdef class __ViewPost__(__ViewMeshAndPost__):
    cdef PyViewPost *thisptrp
    cdef object scalar_view_parameters

    def __cinit__(self):
        self.thisptrp = new PyViewPost()
        self.scalar_view_parameters = __Parameters__(self.__get_scalar_view_parameters__,
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
            return self.scalar_view_parameters.get_parameters()

    def __get_scalar_view_parameters__(self):
        return {'variable' : self.thisptrp.getScalarViewVariable().c_str(),
                'component' : self.thisptrp.getScalarViewVariableComp().c_str(),
                'palette' : self.thisptrp.getScalarViewPalette().c_str(),
                'quality' : self.thisptrp.getScalarViewPaletteQuality().c_str(),
                'steps' : self.thisptrp.getIntParameter(string('View_PaletteSteps')),
                'filter' : self.thisptrp.getBoolParameter(string('View_PaletteFilter')),
                'color_bar' : self.thisptrp.getBoolParameter(string('View_ShowScalarColorBar')),
                'decimal_place' : self.thisptrp.getIntParameter(string('View_ScalarDecimalPlace')),
                'auto_range' : self.thisptrp.getBoolParameter(string('View_ScalarRangeAuto')),
                'range_min' : self.thisptrp.getDoubleParameter(string('View_ScalarRangeMin')),
                'range_max' : self.thisptrp.getDoubleParameter(string('View_ScalarRangeMax')),
                'log_scale' : self.thisptrp.getBoolParameter(string('View_ScalarRangeLog')),
                'log_scale_base' : self.thisptrp.getDoubleParameter(string('View_ScalarRangeBase'))}

    def __set_scalar_view_parameters__(self, parameters):
        # variable, component
        self.thisptrp.setScalarViewVariable(string(parameters['variable']))
        self.thisptrp.setScalarViewVariableComp(string(parameters['component']))

        # palette, quality
        self.thisptrp.setScalarViewPalette(string(parameters['palette']))
        self.thisptrp.setScalarViewPaletteQuality(string(parameters['quality']))

        # steps, filter
        value_in_range(parameters['steps'], 3, 256, 'steps')
        self.thisptrp.setParameter(string('View_PaletteSteps'), <double>parameters['steps'])
        self.thisptrp.setParameter(string('View_PaletteFilter'), <bool>parameters['filter'])

        # color bar, decimal place
        self.thisptrp.setParameter(string('View_ShowScalarColorBar'), <bool>parameters['color_bar'])
        value_in_range(parameters['decimal_place'], 0, 10, 'decimal_place')
        self.thisptrp.setParameter(string('View_ScalarDecimalPlace'), <bool>parameters['decimal_place'])

        # auto range, min, max
        self.thisptrp.setParameter(string('View_ScalarRangeAuto'), <bool>parameters['auto_range'])
        self.thisptrp.setParameter(string('View_ScalarRangeMin'), <double>parameters['range_min'])
        self.thisptrp.setParameter(string('View_ScalarRangeMax'), <double>parameters['range_max'])

        # log scale
        self.thisptrp.setParameter(string('View_ScalarRangeLog'), <bool>parameters['log_scale'])

        # log base
        if (parameters['log_scale_base'] < 0.0 or parameters['log_scale_base'] == 1):
            raise IndexError("Value of 'log_scale_base' must be possitive and can not be equal to 1.")
        self.thisptrp.setParameter(string('View_ScalarRangeBase'), <double>parameters['log_scale_base'])

# ViewPost2D
cdef class __ViewPost2D__(__ViewPost__):
    cdef PyViewPost2D *thisptr2d
    cdef object contour_view_parameters
    cdef object vector_view_parameters

    def __cinit__(self):
        self.thisptr2d = new PyViewPost2D()
        self.contour_view_parameters = __Parameters__(self.__get_contour_view_parameters__,
                                                      self.__set_contour_view_parameters__)
        self.vector_view_parameters = __Parameters__(self.__get_vector_view_parameters__,
                                                     self.__set_vector_view_parameters__)

    def __dealloc__(self):
        del self.thisptr2d

    def activate(self):
        self.thisptr2d.activate()

    def disable(self):
      self.scalar = False
      self.contours = False
      self.vectors = False

    # scalar
    property scalar:
        def __get__(self):
            return self.thisptr2d.getScalarViewShow()
        def __set__(self, show):
            self.thisptr2d.setScalarViewShow(show)

    def export_scalar_vtk(self, filename):
        """Export scalar view in VTK format."""
        self.thisptr2d.exportScalarVTK(filename)

    # contours
    property contours:
        def __get__(self):
            return self.thisptr2d.getContourShow()
        def __set__(self, show):
            self.thisptr2d.setContourShow(show)

    property contour_view_parameters:
        def __get__(self):
            return self.contour_view_parameters.get_parameters()

    def __get_contour_view_parameters__(self):
        return {'variable' : self.thisptr2d.getContourVariable().c_str(),
                'count' : self.thisptr2d.getIntParameter(string('View_ContoursCount')),
                'width' : self.thisptr2d.getDoubleParameter(string('View_ContoursWidth'))}

    def __set_contour_view_parameters__(self, parameters):
        # variable
        self.thisptr2d.setContourVariable(string(parameters['variable']))

        # count, width
        value_in_range(parameters['count'], 1, 100, 'count')
        self.thisptr2d.setParameter(string('View_ContoursCount'), <int>parameters['count'])
        value_in_range(parameters['width'], 0.1, 5.0, 'width')
        self.thisptr2d.setParameter(string('View_ContoursWidth'), <double>parameters['width'])

    def export_contour_vtk(self, filename):
        """Export contour view in VTK format."""
        self.thisptr2d.exportContourVTK(filename)

    # vectors
    property vectors:
        def __get__(self):
            return self.thisptr2d.getVectorShow()
        def __set__(self, show):
            self.thisptr2d.setVectorShow(show)

    property vector_view_parameters:
        def __get__(self):
            return self.vector_view_parameters.get_parameters()

    def __get_vector_view_parameters__(self):
        return {'variable' : self.thisptr2d.getVectorVariable().c_str(),
                'count' : self.thisptr2d.getIntParameter(string('View_VectorCount')),
                'scale' : self.thisptr2d.getDoubleParameter(string('View_VectorScale')),
                'proportional' : self.thisptr2d.getBoolParameter(string('View_VectorProportional')),
                'color' : self.thisptr2d.getBoolParameter(string('View_VectorColor')),
                'type' : self.thisptr2d.getVectorType().c_str(),
                'center' : self.thisptr2d.getVectorCenter().c_str()}

    def __set_vector_view_parameters__(self, parameters):
        # variable
        self.thisptr2d.setVectorVariable(string(parameters['variable']))

        # count, scale
        value_in_range(parameters['count'], 1, 500, 'count')
        self.thisptr2d.setParameter(string('View_VectorCount'), <int>parameters['count'])
        value_in_range(parameters['scale'], 0.1, 20.0, 'scale')
        self.thisptr2d.setParameter(string('View_VectorScale'), <bool>parameters['scale'])

        # proportional, color
        self.thisptr2d.setParameter(string('View_VectorProportional'), <bool>parameters['proportional'])
        self.thisptr2d.setParameter(string('View_VectorColor'), <bool>parameters['color'])

        # type, center
        self.thisptr2d.setVectorType(string(parameters['type']))
        self.thisptr2d.setVectorCenter(string(parameters['center']))

# ViewPost3D
cdef class __ViewPost3D__(__ViewPost__):
    cdef PyViewPost3D *thisptr3d

    def __cinit__(self):
        self.thisptr3d = new PyViewPost3D()
    def __dealloc__(self):
        del self.thisptr3d

    def activate(self):
        self.thisptr3d.activate()

    property mode:
        def __get__(self):
            return self.thisptr3d.getPost3DMode().c_str()
        def __set__(self, mode):
            self.thisptr3d.setPost3DMode(string(mode))

# ViewParticleTracing
cdef class __ViewParticleTracing__:
    cdef PyViewParticleTracing *thisptrpt

    def __cinit__(self):
        self.thisptrpt = new PyViewParticleTracing()
    def __dealloc__(self):
        del self.thisptrpt

    def activate(self):
        self.thisptrpt.activate()

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
