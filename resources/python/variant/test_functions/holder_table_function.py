from variant.model import ModelBase
from variant.optimization.genetic.method import ModelGenetic

from math import sin, cos, exp, sqrt, pi

class HolderTableFunction(ModelGenetic):
    """ f(x,y) = -(sin(x) * cos(y) * exp((1 - (sqrt(x**2 + y**2)(pi))))) """
    def declare(self):
        self.model_info.add_parameter('x', float)
        self.model_info.add_parameter('y', float)
        self.model_info.add_variable('F', float)

    def solve(self):
        try:
            self.F = -abs(sin(self.parameters['x']) * cos(self.parameters['y']) * \
                     exp(abs(1 - (sqrt(self.parameters['x']**2 + self.parameters['y']**2)/(pi)))))
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F

if __name__ == '__main__':
    model = HolderTableFunction()
    model.parameters['x'] = 8.05502
    model.parameters['y'] = 9.66459
    model.solve()
    print(model.F, -19.2085)