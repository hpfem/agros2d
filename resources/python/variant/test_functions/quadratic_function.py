from variant.optimization.genetic.method import ModelGenetic

class QuadraticFunction(ModelGenetic):
    """ f(x) = a * x**2 + b * x + c """
    def declare(self):
        self.model_info.add_parameter('a', float)
        self.model_info.add_parameter('b', float)
        self.model_info.add_parameter('c', float)
        self.model_info.add_parameter('x', float)
        
        self.model_info.add_variable('F', float)
    
    def solve(self):
        try:
            self.F = self.parameters['a'] * self.parameters['x']**2 + \
                     self.parameters['b'] * self.parameters['x'] + self.parameters['c']
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F