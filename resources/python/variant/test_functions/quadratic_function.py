from variant.optimization.genetic.method import ModelGenetic

class QuadraticFunction(ModelGenetic):
    """ f(x) = a * x**2 + b * x + c """
    def declare(self):
        self.model_info.add_parameter('x', float)
        
        self.model_info.add_variable('F', float)
    
    def create(self):
        self.a = 1
        self.b = 1
        self.c = 1
    
    def solve(self):
        try:
            self.F = self.a * self.parameters['x']**2 + \
                     self.b * self.parameters['x'] + self.c
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F