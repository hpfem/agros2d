#from variant.optimization.genetic.method import ModelGenetic
from variant.model import ModelBase

class QuadraticFunction(ModelBase):
    """ f(x) = a * x**2 + b * x + c """
    def declare(self):
        self.declare_parameter('a', float, default=1.0)
        self.declare_parameter('b', float, default=1.0)
        self.declare_parameter('c', float, default=1.0)
        self.declare_parameter('x', float)
        self.declare_variable('F', float)

    def solve(self):
        try:
            self.F = self.get_parameter('a') * self.get_parameter('x')**2 + \
                     self.get_parameter('b') * self.get_parameter('x') + self.get_parameter('c')

            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.set_variable('F', self.F)

if __name__ == '__main__':
    model = QuadraticFunction()
    model.set_parameter('x', 3.0)
    model.solve()
    model.process()
    print(model.get_variable('F'))