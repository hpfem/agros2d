from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "../../src/pythonlab/pyparticletracing.h":
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

        void setIncludeRelativisticCorrection(int incl)
        int includeRelativisticCorrection()

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

        void setCustomForce(map[char*, double] force) except +
        void customForce(map[string, double] force) except +

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
        void times(vector[double] times)

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

    # relativistic correction
    property include_relativistic_correction:
        def __get__(self):
            return self.thisptr.includeRelativisticCorrection()
        def __set__(self, incl):
            self.thisptr.setIncludeRelativisticCorrection(incl)

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

    # custom force
    property custom_force:
        def __get__(self):
            out = dict()
            cdef map[string, double] force

            self.thisptr.customForce(force)
            it = force.begin()
            while it != force.end():
                out[deref(it).first.c_str()] = deref(it).second
                incr(it)

            return out

        def __set__(self, forces):
            cdef map[char*, double] forces_map
            cdef pair[char*, double] force

            for key in forces:
                force.first = key
                force.second = forces[key]
                forces_map.insert(force)

            self.thisptr.setCustomForce(forces_map)

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
