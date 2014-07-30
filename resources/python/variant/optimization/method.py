from variant import ModelDict
from variant.optimization.parameter import OptimizationParameter
from variant.optimization.functional import Functionals

class OptimizationMethod:
    def __init__(self, parameters, functionals, model_class):
        self._parameters = list()
        self.parameters = parameters
        # TODO: Parameters is really bad name!

        self._functionals = None
        self.functionals = functionals

        self.model_dict = ModelDict()
        self.model_class = model_class

    @property
    def parameters(self):
        """List of parameters for optimization."""
        return self._parameters

    def _add_parameter(self, parameter):
        if (parameter.__class__.__base__ == OptimizationParameter):
            self._parameters.append(parameter)
        else:
            raise TypeError('Parameter must be instance of variant.optimization.OptimizationParameter class.')

    @parameters.setter
    def parameters(self, value):
        if isinstance(value, list):
            for item in value:
              self._add_parameter(item)
        else:
            raise TypeError('Parameters must be collected in list.')

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

    """
    def find_existing_model(self, model):
        Check whether model is allready contained in model dictionary
        contained = False
        for oldModel in population:
            allSame = True
            for key in oldModel.parameters.keys():
                # non equality test should be done differently
                # but even this should be better than nothing
                if oldModel.parameters[key] != newModel.parameters[key]:
                    allSame = False
                    break
            if allSame:
                contained = True
                break
                
        return contained
    """
