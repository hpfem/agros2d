from variant import ModelBase
from variant.optimization.genetic.method import ModelGenetic

class BoothsFunction(ModelGenetic):
    """ f(x,y) = (x + 2y - 7)**2 + (2x + y - 5)**2 """
    def declare(self):
        self.model_info.add_parameter('x', float)
        self.model_info.add_parameter('y', float)
        self.model_info.add_variable('F', float)
    
    def solve(self):
        try:
            self.F = (self.parameters['x'] + 2 * self.parameters['y'] - 7)**2 + \
                     (2 * self.parameters['x'] + self.parameters['y'] - 5)**2

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
    print(model.variables['F'], 0)