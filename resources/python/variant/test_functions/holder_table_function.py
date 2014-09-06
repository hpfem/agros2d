from variant.optimization.genetic.method import ModelGenetic

from math import sin, cos, exp, sqrt, pi

class HolderTableFunction(ModelGenetic):
    """ f(x,y) = -(sin(x) * cos(y) * exp((1 - (sqrt(x**2 + y**2)(pi))))) """

    def declare(self):
        self.declare_parameter('x', float)
        self.declare_parameter('y', float)
        self.declare_variable('F', float)

    def solve(self):
        try:
            self.F = -abs(sin(self.get_parameter('x')) * cos(self.get_parameter('y')) * \
                     exp(abs(1 - (sqrt(self.get_parameter('x')**2 + self.get_parameter('y')**2)/(pi)))))
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F

if __name__ == '__main__':
    model = HolderTableFunction()
    model.set_parameter('x', 8.05502)
    model.set_parameter('y', 9.66459)
    model.solve()
    model.process()
    print('F = {0:1.4f}'.format(model.get_variable('F')))