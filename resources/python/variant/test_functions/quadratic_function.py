from variant import ModelBase
from variant.optimization.genetic.method import ModelGenetic

class QuadraticFunction(ModelGenetic):
    """ f(x) = a * x**2 + b * x + c """
    def create(self):
        self.defaults['a'] = 1
        self.defaults['b'] = 0
        self.defaults['c'] = 0
        self.defaults['x'] = 2

    def solve(self):
        try:
            self.F = self.parameters['a'] * self.parameters['x']**2 + \
                     self.parameters['b'] * self.parameters['x'] + self.parameters['c']
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F
