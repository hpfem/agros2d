from ast import literal_eval

class ModelBase:
    def __init__(self):
        self._parameters = dict()
        self._variables = dict()
        self._solved = False

    @property
    def parameters(self):
        """Input parameters"""
        return self._parameters

    @parameters.setter
    def parameters(self, values):
        self._pameters = values

    @property
    def variables(self):
        """Output variables"""
        return self._variables

    @variables.setter
    def variables(self, values):
        self._variables = values
        
    @property
    def functional(self):
        """ Functional """
        return self.variables["_functional"]

    @functional.setter
    def functional(self, value):
        self.variables["_functional"] = value

    @property
    def solved(self):
        """Problem is solved"""
        return self._solved
        
    @solved.setter
    def solved(self, solv):
        self._solved = solv  
        
    def create(self):
        pass

    def solve(self):
        pass

    def process(self):
        pass           
        
    def load(self, filename):
        model_dict = ModelDict()       

        model_dict.load(filename)
        self.parameters = model_dict.models[0].parameters
        self.variables = model_dict.models[0].variables
        self.solved = model_dict.models[0].solved        
        self.fileName = filename

    def save(self, filename):
        model_dict = ModelDict()         

        model_dict.models.append(self)
        model_dict.save(filename)

    def getPopulationFrom(self):
        """From which population present in the case of genetic algorithms"""
        try:
            return self.variables["_population_from"]
        except KeyError:
            return -1

    def setPopulationFrom(self, value):
        self.variables["_population_from"] = value
        
    def getPopulationTo(self):
        """To which population present in the case of genetic algorithms"""
        try:
            return self.variables["_population_to"]
        except KeyError:
            return -1

    def setPopulationTo(self, value):
        self.variables["_population_to"] = value
       
#class ModelGenetic(ModelBase):

#    def __init__(self):
#        ModelBase.__init__(self)

#    @property
#    def populationFrom(self):
#        """From which population present in the case of genetic algorithms"""
#        try:
#            return self.variables["_population_from"]
#        except KeyError:
#            return -1

    # setter does not work
#    @populationFrom.setter
#    def populationFrom(self, value):
#        self.variables["_population_from"] = value

#    @property
#    def populationTo(self):
#        """To which population present in the case of genetic algorithms"""
#        try:
#            return self.variables["_population_to"]
#        except KeyError:
#            return -1

#    # setter does not work
#    @populationTo.setter
#    def populationTo(self, value):
#        print "setting population to ", value
#        self.variables["_population_to"] = value

#    def getPopulationFrom(self):
#        """From which population present in the case of genetic algorithms"""
#        try:
#            return self.variables["_population_from"]
#        except KeyError:
#            return -1

 #   def setPopulationFrom(self, value):
#        self.variables["_population_from"] = value
        
#    def getPopulationTo(self):
#        """To which population present in the case of genetic algorithms"""
#        try:
#            return self.variables["_population_to"]
#        except KeyError:
#            return -1

#    def setPopulationTo(self, value):
#        self.variables["_population_to"] = value
        
                
class ModelDict:
    def __init__(self):
        self._models = []
        self._parameters = dict()
        self._filename = ""
                                
    @property
    def models(self):
        return self._models

    @models.setter
    def models(self, values):
        self._models = values
        
    @property
    def parameters(self):
        """Input parameters"""
        return self._parameters

    @parameters.setter
    def parameters(self, values):
        self._parameters = values
                
    def load(self, filename):
        import xml.etree.ElementTree as ET
        
        tree = ET.parse(filename)
        variant = tree.getroot()  
        
        problem = variant.findall('problem')[0]
        for par in problem.findall('input_param'):
            self.parameters[par.attrib["name"]] = par.attrib["unit"]
         
        results = variant.findall('results')[0]
        for result in results.findall('result'):
            model = ModelBase()

            solution = result.findall('solution')[0]
            model.solved = int(solution.attrib['solved'])
            
            # input
            input = result.findall('input')[0]            
            for par in input.findall('parameter'):
                try:
                    model.parameters[par.attrib["name"]] = float(par.attrib["value"])
                except ValueError:
                    model.parameters[par.attrib["name"]] = literal_eval(par.attrib["value"])
                
            # output
            output = result.findall('output')[0]
            
            # todo: create some other field - info
            #if (model.solved):
            for var in output.findall('variable'):
                try:
                    model.variables[var.attrib["name"]] = float(var.attrib["value"])
                except ValueError:
                    model.variables[var.attrib["name"]] = literal_eval(var.attrib["value"])
                    
                
            self.models.append(model)

    def save(self, filename):
        import xml.etree.cElementTree as ET
        
        variant = ET.Element("variant:variant")        
        variant.set("xmlns:variant", "XMLOptVariant")
        variant.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")

        # problem
        problem = ET.SubElement(variant, "problem")                           
        for key, value in self.parameters.items():                           
            input_parameter = ET.SubElement(problem, "input_parameter")
            input_parameter.set("param", key)
            input_parameter.set("unit", value)
                                                   
        # results    
        results = ET.SubElement(variant, "results")                               
        for model in self.models:
            result = ET.SubElement(results, "result")   
                                                                            
            # input
            input = ET.SubElement(result, "input")
            for key, value in model.parameters.items():
                parameter = ET.SubElement(input, "parameter")
                parameter.set("name", key)
                parameter.set("value", str(value))
            
            # todo: create some other field - info
            
            # output
            output = ET.SubElement(result, "output")
            #if (model.solved):
            for key, value in model.variables.items():
                variable = ET.SubElement(output, "variable")
                variable.set("name", key)
                variable.set("value", str(value))        
                
            # solution
            solution = ET.SubElement(result, "solution")
            solution.set("solved", "1" if model.solved else "0")                      
                        
        tree = ET.ElementTree(variant)
        tree.write(filename, xml_declaration = True, encoding='UTF-8')
        
     