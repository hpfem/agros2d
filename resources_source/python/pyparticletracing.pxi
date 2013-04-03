cdef extern from "../../agros2d-library/pythonlab/pyparticletracing.h":
    cdef cppclass PyParticleTracing:
        PyParticleTracing()

        void getInitialPosition(vector[double] &position)
        void setInitialPosition(vector[double] &position) except +

        void getInitialVelocity(vector[double] &velocity)
        void setInitialVelocity(vector[double] &velocity)

        void setNumberOfParticles(int particles)  except +
        int getNumberOfParticles()

        void setStartingRadius(double radius) except +
        double getStartingRadius()

        double getParticleMass()
        void setParticleMass(double mass) except +

        double getParticleCharge()
        void setParticleCharge(double charge)

        double getDragForceDensity()
        void setDragForceDensity(double rho) except +
        double getDragForceReferenceArea()
        void setDragForceReferenceArea(double area) except +
        double getDragForceCoefficient()
        void setDragForceCoefficient(double coeff) except +

        void getCustomForce(vector[double] &force) except +
        void setCustomForce(vector[double] &force) except +

        bool getIncludeRelativisticCorrection()
        void setIncludeRelativisticCorrection(bool incl)

        string getButcherTableType()
        void setButcherTableType(string &tableType) except +
        double getMaximumRelativeError()
        void setMaximumRelativeError(double tolerance) except +
        int getMaximumNumberOfSteps()
        void setMaximumNumberOfSteps(int steps) except +
        double getMinimumStep()
        void setMinimumStep(double step) except +

        bool getReflectOnDifferentMaterial()
        void setReflectOnDifferentMaterial(bool reflect)
        bool getReflectOnBoundary()
        void setReflectOnBoundary(bool reflect)
        double getCoefficientOfRestitution()
        void setCoefficientOfRestitution(double coeff)

        void solve() except +

        int length()
        void positions(vector[double] &x, vector[double] &y, vector[double] &z)
        void velocities(vector[double] &x, vector[double] &y, vector[double] &z)
        void times(vector[double] &times)

cdef vector[double] list_to_double_vector(list):
    cdef vector[double] vector
    for item in list:
        vector.push_back(item)

    return vector

cdef class __ParticleTracing__:
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
        cdef vector[double] x, y, z
        self.thisptr.positions(x, y, z)
        return double_vector_to_list(x), double_vector_to_list(y), double_vector_to_list(z)

    # velocities - x, y, z or r, z, phi
    def velocities(self):
        cdef vector[double] vx, vy, vz
        self.thisptr.velocities(vx, vy, vz)
        return double_vector_to_list(vx), double_vector_to_list(vy), double_vector_to_list(vz)
    # times
    def times(self):
        cdef vector[double] time
        self.thisptr.times(time)
        return double_vector_to_list(time)

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

    # initial position
    property initial_position:
        def __get__(self):
            cdef vector[double] position
            self.thisptr.getInitialPosition(position)
            return double_vector_to_list(position)
        def __set__(self, position):
            self.thisptr.setInitialPosition(list_to_double_vector(position))

    # initial velocity
    property initial_velocity:
        def __get__(self):
            cdef vector[double] velocity
            self.thisptr.getInitialVelocity(velocity)
            return double_vector_to_list(velocity)
        def __set__(self, velocity):
            self.thisptr.setInitialVelocity(list_to_double_vector(velocity))

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
        def __set__(self, density):
            self.thisptr.setDragForceDensity(density)

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
            cdef vector[double] force
            self.thisptr.getCustomForce(force)
            return double_vector_to_list(force)
        def __set__(self, force):
            self.thisptr.setCustomForce(list_to_double_vector(force))

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

    # maximum relative error
    property maximum_relative_error:
        def __get__(self):
            return self.thisptr.getMaximumRelativeError()
        def __set__(self, tolerance):
            self.thisptr.setMaximumRelativeError(tolerance)

    # minimum step
    property minimum_step:
        def __get__(self):
            return self.thisptr.getMinimumStep()
        def __set__(self, step):
            self.thisptr.setMinimumStep(step)

particle_tracing = __ParticleTracing__()
