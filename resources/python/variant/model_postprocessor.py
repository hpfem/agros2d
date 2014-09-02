import pythonlab
import pylab as pl

from variant.model_dict import ModelDict
from variant.model_filter import ModelFilter

class ModelPostprocessor():
    def __init__(self, model_dict):
        self._model_dict = model_dict

        # model
        m = self._model_dict.model_class()

        # cache parameters
        self._parameters = {}
        for model in self._model_dict.models():
            for parameter, value in model.parameters.items():
                self._parameters.setdefault(parameter, []).append(value)

        # cache parameters
        parameters = m.model_info.parameters
        self._parameter_keys = sorted(list(parameters.keys()))        
        self._parameter_keys_scalar = sorted([key for key, value in parameters.items() if (value['type'] == int or value['type'] == float)])

        # cache variables
        self._variables = {}
        for model in self._model_dict.models():
            for variable, value in model.variables.items():
                self._variables.setdefault(variable, []).append(value)

        # cache variables
        variables = m.model_info.variables
        self._variable_keys = sorted(list(variables.keys()))        
        self._variable_keys_scalar = sorted([key for key, value in variables.items() if (value['type'] == int or value['type'] == float)])

    @property
    def model_dict(self):
        """Return model dict"""
        return self._model_dict

    def parameters(self):        
        return self._variables
        
    def parameter_keys(self, only_scalars = False):
        if (only_scalars == True):
            return self._parameter_keys_scalar
        else:
            return self._parameter_keys
    
    def parameter(self, name):
        return self._parameters[name]
        
    def variables(self):        
        return self._variables
        
    def variable_keys(self, only_scalars = False):
        if (only_scalars == True):
            return self._variable_keys_scalar
        else:
            return self._variable_keys
    
    def variable(self, name):
        return self._variables[name]        
        
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

    print("Dict")
    mp = ModelPostprocessor(md)
    print(mp.variable_keys())
    print(mp.variables())
    print(mp.variable('F'))
    
    # mp.ploty('F')
    # mp.bar('F')

    mf = ModelFilter()
    mf.add_parameter_range('x', 1, 3)

    print("Filter")
    mp = ModelPostprocessor(mf.filter(md))
    print(mp.variable_keys())
    print(mp.variables())
    print(mp.variable('F'))
    
    # mp.ploty('F')
    # mp.bar('F')