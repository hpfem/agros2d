from variant.model_dictionary import ModelDictionary
from variant.optimization.parameter import Parameters
from variant.optimization.functional import Functionals
#from variant.optimization.stoping_criterion import ImplicitStopingCriterion

class OptimizationMethod:
    def __init__(self, parameters, functionals, model_class):
        self._parameters = None
        self.parameters = parameters

        self._functionals = None
        self.functionals = functionals

        self.model_dict = ModelDictionary(model_class = model_class)

        #self.stopping_criterion = ImplicitStopingCriterion()

    @property
    def parameters(self):
        """List of parameters for optimization."""
        return self._parameters

    @parameters.setter
    def parameters(self, value):
        if isinstance(value, Parameters):
            self._parameters = value
        else:
            raise TypeError('Parameters must be instance of Parameters class.')

    @property
    def functionals(self):
        """Functionals for optimization."""
        return self._functionals
        
    @functionals.setter
    def functionals(self, value):
        if isinstance(value, Functionals):
            self._functionals = value
        else:
            raise TypeError('Functionals must be instance of variant.optimization.Functionals class.')
