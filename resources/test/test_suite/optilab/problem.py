from variant import ModelBase

class Model(ModelBase):
    def solve(self):
        self.sqr = self.parameters['a'] ** self.parameters['b']

    def process(self):
        self.variables['sqr'] = self.sqr
