class OptimizationMethod:
    def __init__(self, parameters, functionals):
        self._parameters = parameters
        self._functionals = functionals
        
    @property
    def parameters(self):
        """Parameters"""
        return self._parameters
        
    @parameters.setter
    def parameters(self, value):
        self._parameters = value
        
    @property
    def directory(self):
        """Working directory"""
        return self._directory

    @directory.setter
    def directory(self, value):
        self._directory = value
                    
    @property
    def functionals(self):
        """Functionals"""
        return self._functionals      
                   
         
    def isContained(self, population, newModel):
        """ checks whether newModel is allready contained in population """
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
