from libcpp.vector cimport vector
from libcpp cimport bool

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

cdef extern from "../../agros2d-library/pythonlab/pyparticletracing.h":
    cdef cppclass PyParticleTracing:
        PyParticleTracing()

        void setInitialPosition(double x, double y) except +
        void getInitialPosition(double x, double y)

        void setInitialVelocity(double x, double y)
        void getInitialVelocity(double x, double y)

        int getNumberOfParticles()
        void setNumberOfParticles(int particles)  except +

        double getStartingRadius()
        void setStartingRadius(double radius) except +

        void setParticleMass(double mass) except +
        double getParticleMass()

        void setParticleCharge(double charge)
        double getParticleCharge()

        void setDragForceDensity(double rho) except +
        double getDragForceDensity()
        void setDragForceReferenceArea(double area) except +
        double getDragForceReferenceArea()
        void setDragForceCoefficient(double coeff) except +
        double getDragForceCoefficient()

        void setCustomForce(double x, double y, double z) except +
        void getCustomForce(double x, double y, double z) except +

        void setIncludeRelativisticCorrection(bool incl)
        bool getIncludeRelativisticCorrection()

        void setButcherTableType(string &tableType) except +
        string getButcherTableType()
        void setMaximumTolerance(double tolerance) except +
        double getMaximumTolerance()
        void setMaximumNumberOfSteps(int steps) except +
        int getMaximumNumberOfSteps()
        void setMinimumStep(double step) except +
        double getMinimumStep()

        void setReflectOnDifferentMaterial(bool reflect)
        bool getReflectOnDifferentMaterial()
        void setReflectOnBoundary(bool reflect)
        bool getReflectOnBoundary()
        void setCoefficientOfRestitution(double coeff)
        double getCoefficientOfRestitution()

        void solve() except +

        int length()
        void positions(vector[double] x, vector[double] y, vector[double] z)
        void velocities(vector[double] x, vector[double] y, vector[double] z)
        void times(vector[double] times)

cdef class __ParticleTracing__:
    cdef PyParticleTracing *thisptr

    def __cinit__(self):
        self.thisptr = new PyParticleTracing()
    def __dealloc__(self):
        del self.thisptr

    # number of particles
    property number_of_particles:
        def __get__(self):
            return self.thisptr.getNumberOfParticles()
        def __set__(self, particles):
            self.thisptr.setNumberOfParticles(particles)

    # particles dispersion
    property particles_dispersion:
        def __get__(self):
            return self.thisptr.getStartingRadius()
        def __set__(self, dispersion):
            self.thisptr.setStartingRadius(dispersion)

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

    # times
    def times(self):
        outtime = list()
        cdef vector[double] time
        self.thisptr.times(time)
        for i in range(self.thisptr.length()):
            outtime.append(time[i])
        return outtime

    # initial position
    property initial_position:
        def __get__(self):
            cdef double x = 0.0
            cdef double y = 0.0
            self.thisptr.getInitialPosition(x, y)
            return x, y
        def __set__(self, xy):
            self.thisptr.setInitialPosition(xy[0], xy[1])

    # initial velocity
    property initial_velocity:
        def __get__(self):
            cdef double x = 0.0
            cdef double y = 0.0
            self.thisptr.getInitialVelocity(x, y)
            return x, y
        def __set__(self, xy):
            self.thisptr.setInitialVelocity(xy[0], xy[1])

    # mass
    property mass:
        def __get__(self):
            return self.thisptr.getParticleMass()
        def __set__(self, mass):
            self.thisptr.setParticleMass(mass)

    # charge
    property charge:
        def __get__(self):
            return self.thisptr.getParticleCharge()
        def __set__(self, charge):
            self.thisptr.setParticleCharge(charge)

    # relativistic correction
    property include_relativistic_correction:
        def __get__(self):
            return self.thisptr.getIncludeRelativisticCorrection()
        def __set__(self, incl):
            self.thisptr.setIncludeRelativisticCorrection(incl)

    # reflect on different material
    property reflect_on_different_material:
        def __get__(self):
            return self.thisptr.getReflectOnDifferentMaterial()
        def __set__(self, reflect):
            self.thisptr.setReflectOnDifferentMaterial(reflect)

    # reflect on boundary
    property reflect_on_boundary:
        def __get__(self):
            return self.thisptr.getReflectOnBoundary()
        def __set__(self, reflect):
            self.thisptr.setReflectOnBoundary(reflect)

    property coefficient_of_restitution:
        def __get__(self):
            return self.thisptr.getCoefficientOfRestitution()
        def __set__(self, coeff):
            self.thisptr.setCoefficientOfRestitution(coeff)

    # drag force
    property drag_force_density:
        def __get__(self):
            return self.thisptr.getDragForceDensity()
        def __set__(self, rho):
            self.thisptr.setDragForceDensity(rho)

    property drag_force_reference_area:
        def __get__(self):
            return self.thisptr.getDragForceReferenceArea()
        def __set__(self, area):
            self.thisptr.setDragForceReferenceArea(area)

    property drag_force_coefficient:
        def __get__(self):
            return self.thisptr.getDragForceCoefficient()
        def __set__(self, coeff):
            self.thisptr.setDragForceCoefficient(coeff)

    # custom force
    property custom_force:
        def __get__(self):
            cdef double x = 0.0
            cdef double y = 0.0
            cdef double z = 0.0
            self.thisptr.getCustomForce(x, y, z)
            return x, y, z
        def __set__(self, xyz):
            self.thisptr.setCustomForce(xyz[0], xyz[1], xyz[2])

    # butcher table type
    property butcher_table_type:
        def __get__(self):
            return self.thisptr.getButcherTableType().c_str()
        def __set__(self, table_type):
            self.thisptr.setButcherTableType(string(table_type))

    # maximum number of steps
    property maximum_number_of_steps:
        def __get__(self):
            return self.thisptr.getMaximumNumberOfSteps()
        def __set__(self, steps):
            self.thisptr.setMaximumNumberOfSteps(steps)

    # tolerance
    property tolerance:
        def __get__(self):
            return self.thisptr.getMaximumTolerance()
        def __set__(self, tolerance):
            self.thisptr.setMaximumTolerance(tolerance)

    # minimum step
    property minimum_step:
        def __get__(self):
            return self.thisptr.getMinimumStep()
        def __set__(self, step):
            self.thisptr.setMinimumStep(step)

particle_tracing = __ParticleTracing__()
