from variant import ModelBase

class QuadraticFunction(ModelBase):
    """ f(x) = a * x**2 + b * x + c """
    def declare(self):
        self.parameters.declare('a', float, default=1.0)
        self.parameters.declare('b', float, default=1.0)
        self.parameters.declare('c', float, default=1.0)
        self.parameters.declare('x', float)
        self.variables.declare('F', float)

    def solve(self):
        self.variables['F'] = self.parameters['a'] * self.parameters['x']**2 + \
                              self.parameters['b'] * self.parameters['x'] + self.parameters['c']

if __name__ == '__main__':
    model = QuadraticFunction()
    model.parameters['x'] = 3.0
    model.solve()
    print('F = {0}'.format(model.variables['F']))