from variant.optimization.genetic.method import ModelGenetic

class QuadraticFunction(ModelGenetic):
    """ f(x) = a * x**2 + b * x + c """
    def declare(self):
        self.declare_parameter('a', float, default=1.0)
        self.declare_parameter('b', float, default=1.0)
        self.declare_parameter('c', float, default=1.0)
        self.declare_parameter('x', float)
        self.declare_variable('F', float)

    def solve(self):
        try:
            self.F = self.parameters['a'] * self.parameters['x']**2 + \
                     self.parameters['b'] * self.parameters['x'] + self.parameters['c']
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F

if __name__ == '__main__':
    model = QuadraticFunction()
    model.parameters['x'] = 3.0
    model.solve()
    model.process()
    #print(model.variables['F'])
