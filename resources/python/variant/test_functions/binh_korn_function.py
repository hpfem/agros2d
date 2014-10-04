from variant.optimization.genetic.method import ModelGenetic

class BinhKornFunction(ModelGenetic):
    """ f1(x,y) = 4 * x**2 + 4 * y**2; f2(x,y) = (x - 5)**2 + (y - 5)**2 """

    def declare(self):
        self.declare_parameter('x', float)
        self.declare_parameter('y', float)

        self.declare_variable('F1', float)
        self.declare_variable('F2', float)

    def solve(self):
        try:
            self.F1 = 4 * self.get_parameter('x')**2 + 4 * self.get_parameter('y')**2
            self.F2 = (self.get_parameter('x') - 5)**2 + (self.get_parameter('y') - 5)**2

            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.set_variable('F1', self.F1)
        self.set_variable('F2', self.F2)

if __name__ == '__main__':
    model = BinhKornFunction()
    model.set_parameter('x', 2.5)
    model.set_parameter('y', 1.5)
    model.solve()
    model.process()
    print('F1 = {0}, F2 = {1}'.format(model.get_variable('F1'), model.get_variable('F2')))