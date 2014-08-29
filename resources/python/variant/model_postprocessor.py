import pythonlab

from variant import ModelBase, ModelDict
import pylab as pl

class ModelPostprocessor():
    def __init__(self, model_dict):
        self._model_dict = model_dict
    
    def variables(self):
        super_dict = {}
        for val in self._model_dict.dict.values():
            for k, v in val.data.variables.items():
                super_dict.setdefault(k, []).append(v)
        
        return super_dict
        
    def variable_keys(self):
        # TODO: first item - do it better!
        if (len(self._model_dict.dict.values()) > 0):
            return list(next(iter(self._model_dict.dict.values())).data.variables.keys())
    
    def variable(self, name):
        return self.variables()[name]
        
    def ploty(self, name, xlabel="", ylabel=""):
        var = mp.variable(name)        
        pl.plot(range(len(var)), var)
        pl.grid(True)
                
        chart_file = pythonlab.tempname("png")
        pl.savefig(chart_file, dpi=60)
        pl.close()
        pythonlab.image(chart_file)    
        
    def bar(self, name, xlabel="", ylabel=""):
        var = mp.variable(name)        
        pl.bar(range(len(var)), var)
        pl.grid(True)
                
        chart_file = pythonlab.tempname("png")
        pl.savefig(chart_file, dpi=60)
        pl.close()
        pythonlab.image(chart_file)          
    
if __name__ == '__main__':
    from variant.test_functions import quadratic_function

    md = ModelDict(quadratic_function.QuadraticFunction)
    for x in range(10):
        model = quadratic_function.QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    md.solve()    
    # print(md.dict)
    
    mp = ModelPostprocessor(md)
    print(mp.variable_keys())
    print(mp.variables())
    print(mp.variable('F'))
    
    mp.ploty('F')
    mp.bar('F')    