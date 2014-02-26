class ModelBase:
    def __init__(self):
        self._parameters = dict()
        self._variables = dict()
        self._solved = False
        self._filename = ""
                                
    @property
    def parameters(self):
        """Input parameters"""
        return self._parameters

    @parameters.setter
    def parameters(self, values):
        self._parameters = values

    @property
    def variables(self):
        """Output variables"""
        return self._variables

    @variables.setter
    def variables(self, values):
        self._variables = values

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
            self.parameters[par.attrib["input_param"]] = par.attrib["input_unit"]
         
        results = variant.findall('results')[0]
        for result in results.findall('result'):
            model = ModelBase()

            solution = result.findall('solution')[0]
            model.solved = int(solution.attrib['solved'])
            
            # input
            input = result.findall('input')[0]            
            for par in input.findall('parameter'):
                model.parameters[par.attrib["param"]] = float(par.attrib["param_value"])
                
            # output
            output = result.findall('output')[0]
            if (model.solved):
                for var in output.findall('variable'):
                    model.variables[var.attrib["var"]] = var.attrib["var_value"]
                
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
            input_parameter.set("input_param", key)
            input_parameter.set("input_unit", value)
                                                   
        # results    
        results = ET.SubElement(variant, "results")                               
        for model in self.models:
            result = ET.SubElement(results, "result")   
                                                                            
            # input
            input = ET.SubElement(result, "input")
            for key, value in model.parameters.items():
                parameter = ET.SubElement(input, "parameter")
                parameter.set("param", key)
                parameter.set("param_value", str(value))
            
            # output
            output = ET.SubElement(result, "output")
            if (model.solved):
                for key, value in model.variables.items():
                    variable = ET.SubElement(output, "variable")
                    variable.set("var", key)
                    variable.set("var_value", str(value))        
                
            # solution
            solution = ET.SubElement(result, "solution")
            solution.set("solved", "1" if model.solved else "0")                      
                        
        tree = ET.ElementTree(variant)
        tree.write(filename, xml_declaration = True, encoding='UTF-8')
        
     