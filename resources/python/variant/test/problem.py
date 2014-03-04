from variant import model

class Model(model.ModelBase):
    def create(self):
        self.a = self.parameters["a"]
        self.b = self.parameters["b"]
        self.c = self.parameters["c"]
        self.d = self.parameters["d"]
        self.e = self.parameters["e"]

    def solve(self):
        self.solved = True

    def process(self):
        self.variables["Func1"] = self.a + self.b + self.c + self.d + self.e

