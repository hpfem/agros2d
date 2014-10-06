from variant import ModelBase
from math import sin, cos, exp, sqrt, pi

class HolderTableFunction(ModelBase):
    """ f(x,y) = -(sin(x) * cos(y) * exp((1 - (sqrt(x**2 + y**2)(pi))))) """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

    def solve(self):
        self.variables['F'] = -abs(sin(self.parameters['x']) * cos(self.parameters['y']) * \
                              exp(abs(1 - (sqrt(self.parameters['x']**2 + self.parameters['y']**2)/(pi)))))

if __name__ == '__main__':
    model = HolderTableFunction()
    model.parameters['x'] = 8.05502
    model.parameters['y'] = 9.66459
    model.solve()
    print('F = {0:1.4f}'.format(model.variables['F']))