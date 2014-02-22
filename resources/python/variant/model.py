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
        
    def load(self, filename):
        import xml.etree.ElementTree as ET
        
        tree = ET.parse(filename)
        variant = tree.getroot()
        
        self.solved = False
        
        input = variant.findall('input')[0]
        for par in input.findall('parameter'):
            self.parameters[par.attrib["param"]] = float(par.attrib["param_value"])

    def save(self, filename):
        import xml.etree.cElementTree as ET
        
        variant = ET.Element("variant:variant")        
        variant.set("xmlns:variant", "XMLOptVariant")
        variant.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
                    
        # input
        input = ET.SubElement(variant, "input")
        for key, value in self.parameters.items():
            parameter = ET.SubElement(input, "parameter")
            parameter.set("param", key)
            parameter.set("param_value", str(value))
        
        # output
        output = ET.SubElement(variant, "output")
        if (self.solved):
            for key, value in self.variables.items():
                variable = ET.SubElement(output, "variable")
                variable.set("var", key)
                variable.set("var_value", str(value))        

        # solution
        solution = ET.SubElement(variant, "solution")
        solution.set("solved", "1" if self.solved else "0")
            
        tree = ET.ElementTree(variant)
        tree.write(filename, xml_declaration = True, encoding='UTF-8')