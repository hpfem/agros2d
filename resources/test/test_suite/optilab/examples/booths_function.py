from variant import ModelBase

class BoothsFunction(ModelBase):
    """ f(x,y) = (x + 2y - 7)**2 + (2x + y - 5)**2 """
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
    model = HolderTableFunction()
    model.parameters['x'] = 1
    model.parameters['y'] = 3
    model.solve()
    print(model.F, 0)
