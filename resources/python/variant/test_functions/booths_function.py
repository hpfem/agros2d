from variant.optimization.genetic.method import ModelGenetic

class BoothsFunction(ModelGenetic):
    """ f(x,y) = (x + 2y - 7)**2 + (2x + y - 5)**2 """

    def declare(self):
        self.declare_parameter('x', float)
        self.declare_parameter('y', float)
        self.declare_variable('F', float)
    
    def solve(self):
        try:
            self.F = (self.get_parameter('x') + 2 * self.get_parameter('y') - 7)**2 + \
                     (2 * self.get_parameter('x') + self.get_parameter('y') - 5)**2

            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F

if __name__ == '__main__':
    model = BoothsFunction()
    model.parameters['x'] = 1.0
    model.parameters['y'] = 3.0
    model.solve()
    model.process()
    print('F = {0}'.format(model.get_variable('F')))
