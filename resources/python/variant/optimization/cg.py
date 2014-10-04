from variant.optimization import ContinuousParameter, OptimizationMethod
from copy import deepcopy

TypeState = 0
TypeGradient = 1
TypeLineSearch = 2

class CGInfo:
    @staticmethod
    def type(member):
        return member.info["type"]

    @staticmethod
    def setType(member, value):
        member.info["type"] = value
        
    @staticmethod
    def iteration(member):
        return member.info["iteration"]

    @staticmethod
    def setIteration(member, value):
        member.info["iteration"] = value
        
    @staticmethod
    def changedParameter(member):
        return member.info["changed_parameter"]

    @staticmethod
    def setchangedParameter(member, value):
        member.info["changed_parameter"] = value
        
    @staticmethod
    def change(member):
        return member.info["change"]

    @staticmethod
    def setChange(member, value):
        member.info["change"] = value

class CGOptimization(OptimizationMethod):
       
    def findState(self, models):
        maxIter = -1
        for model in models:
            if CGInfo.type(model) == TypeState:
                iter = CGInfo.iteration(model)
                if iter > maxIter:
                    maxIter = iter
                    state = model
                    
        return state
        
                
    def gradient(self):        
        models = self.modelSetManager.load_all()
        state = self.findState(models)

        if self.gradientOrder == 1:
            changes = [1]
        elif self.gradientOrder == 2:
            changes = [-1, 1]
        else:
            assert False
            
        toCalculate = 0;
        for parameter in self.parameters:
            for change in changes:
                perturbed_state = deepcopy(state)
                perturbed_state.parameters[parameter.name] += change * self.eps * parameter.intervalLength()
                CGInfo.setType(perturbed_state, TypeGradient)
                CGInfo.setChangedParameter(perturbed_state, parameter.name)
                CGInfo.setChange(perturbed_state, change)
                
                toCalculate.append(perturbed_state)
        
        # save and solve all models 
        newModels = models[:]
        newModels.extend(toCalculate)
        self.modelSetManager.save_all(newModels)
        self.modelSetManager.solve_all()
        
        # load solved models
        newModels = self.modelSetManager.load_all()
        
        # delete models and save only previously saved (we do not want to store perturbations used for gradient calculation only
        self.modelSetManager.delete_all()
        self.modelSetManager.save_all(models)
        
        perturbations = dict()
        for parameter in self.parameters:
            perturbations[parameter.name] = dict()
            
        centralValue = self.functionals.evaluate(state)
        for model in newModels:
            if CGInfo.type(model) == TypeGradient:
                perturbations[CGInfo.changedParameter(model)][CGInfo.change(model)] = self.functionals.evaluate(model)
            
        gradient = dict()    
        for paramName in perturbations.keys():
            if self.gradientOrder == 1:
                gradient[paramName] = float(perturbations[paramName][1] - centralValue) / self.eps
            elif self.gradientOrder == 2:
                gradient[paramName] = float(perturbations[paramName][1] - perturbations[paramName][-1]) / (2 * self.eps)
            else:
                assert False
                
        return gradient
