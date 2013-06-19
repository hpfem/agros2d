cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyProblem:
        PyProblem(bool clear)

        void clear()
        void clearSolution() except +
        void refresh()

        string getCoordinateType()
        void setCoordinateType(string &coordinateType) except +

        string getMeshType()
        void setMeshType(string &meshType) except +

        double getFrequency()
        void setFrequency(double frequency) except +

        string getTimeStepMethod()
        void setTimeStepMethod(string &timeStepMethod) except +

        int getTimeMethodOrder()
        void setTimeMethodOrder(int timeMethodOrder) except +

        double getTimeMethodTolerance()
        void setTimeMethodTolerance(double timeMethodTolerance) except +

        double getTimeTotal()
        void setTimeTotal(double timeTotal) except +

        int getNumConstantTimeSteps()
        void setNumConstantTimeSteps(int timeSteps) except +

        string getCouplingType(string &sourceField, string &targetField) except +
        void setCouplingType(string &sourceField, string &targetField, string &type) except +

        void mesh() except +
        void solve() except +
        void solveAdaptiveStep() except +

        double timeElapsed() except +
        void timeStepsLength(vector[double] &steps) except +

cdef class __Problem__:
    cdef PyProblem *thisptr

    def __cinit__(self, clear = False):
        self.thisptr = new PyProblem(clear)

    def __dealloc__(self):
        del self.thisptr

    def clear(self):
        """Clear problem."""
        self.thisptr.clear()

    def clear_solution(self):
        """Clear solution."""
        self.thisptr.clearSolution()

    def refresh(self):
        """Refresh preprocessor and postprocessor."""
        self.thisptr.refresh()

    property coordinate_type:
        def __get__(self):
            return self.thisptr.getCoordinateType().c_str()
        def __set__(self, coordinate_type):
            self.thisptr.setCoordinateType(string(coordinate_type))

    property mesh_type:
        def __get__(self):
            return self.thisptr.getMeshType().c_str()
        def __set__(self, mesh_type):
            self.thisptr.setMeshType(string(mesh_type))

    property frequency:
        def __get__(self):
            return self.thisptr.getFrequency()
        def __set__(self, frequency):
            self.thisptr.setFrequency(frequency)

    property time_step_method:
        def __get__(self):
            return self.thisptr.getTimeStepMethod().c_str()
        def __set__(self, time_step_method):
            self.thisptr.setTimeStepMethod(string(time_step_method))

    property time_method_order:
        def __get__(self):
            return self.thisptr.getTimeMethodOrder()
        def __set__(self, time_method_order):
            self.thisptr.setTimeMethodOrder(time_method_order)

    property time_method_tolerance:
        def __get__(self):
            return self.thisptr.getTimeMethodTolerance()
        def __set__(self, time_method_tolerance):
            self.thisptr.setTimeMethodTolerance(time_method_tolerance)

    property time_total:
        def __get__(self):
            return self.thisptr.getTimeTotal()
        def __set__(self, time_total):
            self.thisptr.setTimeTotal(time_total)

    property time_steps:
        def __get__(self):
            return self.thisptr.getNumConstantTimeSteps()
        def __set__(self, time_steps):
            self.thisptr.setNumConstantTimeSteps(time_steps)

    def get_coupling_type(self, source_field, target_field):
        """Return type of coupling.

        get_coupling_type(source_field, target_field)

        Keyword arguments:
        source_field -- source field id
        target_field -- target field id
        """
        return self.thisptr.getCouplingType(string(source_field), string(target_field)).c_str()

    def set_coupling_type(self, source_field, target_field, type):
        """Set type of coupling.

        set_coupling_type(source_field, target_field, type)

        Keyword arguments:
        source_field -- source field id
        target_field -- target field id
        type -- coupling type
        """
        self.thisptr.setCouplingType(string(source_field), string(target_field), string(type))

    def mesh(self):
        """Area discretization."""
        self.thisptr.mesh()

    def solve(self):
        """Solve problem."""
        self.thisptr.solve()

    def solve_adaptive_step(self):
        """Solve one adaptive step."""
        self.thisptr.solveAdaptiveStep()

    def elapsed_time(self):
        """Return elapsed time in seconds."""
        return self.thisptr.timeElapsed()

    def time_steps_length(self):
        """Return a list of time steps length."""
        cdef vector[double] steps_vector
        self.thisptr.timeStepsLength(steps_vector)

        steps = list()
        for i in range(steps_vector.size()):
            steps.append(steps_vector[i])

        return steps

    def time_steps_total(self):
        """Return a list of time steps."""
        steps = self.time_steps_length()
        time = [0.0]
        for step in steps:
            time.append(time[-1] + step)

        return time

__problem__ = __Problem__()
def problem(clear = False):
    if (clear):
        __problem__.clear()
    return __problem__
