from variant import ModelBase
from math import sin, cos, exp, sqrt, pi

class HolderTableFunction(ModelBase):
    """ f(x,y) = -(sin(x) * cos(y) * exp((1 - (sqrt(x**2 + y**2)(pi))))) """
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
