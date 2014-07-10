from variant import ModelBase

class Model(ModelBase):
    """ y = a*x**2 """
    def create(self):
        self.defaults['a'] = 1

    def solve(self):
        try:
            self.y = self.parameters['a'] * self.parameters['x']**2
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['y'] = self.y