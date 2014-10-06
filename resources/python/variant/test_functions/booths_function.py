from variant import ModelBase

class BoothsFunction(ModelBase):
    """ f(x,y) = (x + 2y - 7)**2 + (2x + y - 5)**2 """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)
    
    def solve(self):
            self.variables['F'] = (self.parameters['x'] + 2 * self.parameters['y'] - 7)**2 + \
                                  (2 * self.parameters['x'] + self.parameters['y'] - 5)**2

if __name__ == '__main__':
    model = BoothsFunction()
    model.parameters['x'] = 1.0
    model.parameters['y'] = 3.0
    model.solve()
    print('F = {0}'.format(model.variables['F']))
