from ast import literal_eval

class ModelBase:
    def __init__(self):
        self._parameters = dict()
        self._variables = dict()
        self._info = dict()
        self._geometry = ""
        self._solved = False

    @property
    def parameters(self):
        """ Input parameters """
        return self._parameters

    @parameters.setter
    def parameters(self, values):
        self._pameters = values

    @property
    def variables(self):
        """ Output variables """
        return self._variables

    @variables.setter
    def variables(self, values):
        self._variables = values

    @property
    def info(self):
        """ Optional info """
        return self._info

    @info.setter
    def info(self, values):
        self._info = values

    @property
    def geometry(self):
        """ Optional geometry SVG image """
        return self._geometry

    @geometry.setter
    def image(self, geometry):
        self._geometry = geometry

    @property
    def solved(self):
        """ Problem is solved """
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
        rr = variant.findall('results')

        # TODO: only one result
        results = variant.findall('results')[0]
        result = results.findall('result')[0]

        solution = result.findall('solution')[0]
        self.solved = int(solution.attrib['solved'])
        self.geometry = solution.attrib['geometry']

        # input
        input = result.findall('input')[0]
        for par in input.findall('parameter'):
            try:
                self.parameters[par.attrib["name"]] = float(par.attrib["value"])
            except ValueError:
                self.parameters[par.attrib["name"]] = literal_eval(par.attrib["value"])

        # output
        output = result.findall('output')[0]
        for var in output.findall('variable'):
            try:
                self.variables[var.attrib["name"]] = float(var.attrib["value"])
            except ValueError:
                self.variables[var.attrib["name"]] = literal_eval(var.attrib["value"])

        # info
        info = result.findall('info')[0]
        for item in info.findall('item'):
            try:
                self.info[item.attrib["name"]] = float(item.attrib["value"])
            except ValueError:
                self.info[item.attrib["name"]] = literal_eval(item.attrib["value"])

    def save(self, filename):
        import xml.etree.cElementTree as ET

        variant = ET.Element("variant:variant")        
        variant.set("xmlns:variant", "XMLOptVariant")
        variant.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")

        # results
        results = ET.SubElement(variant, "results")
        result = ET.SubElement(results, "result")

        # input
        input = ET.SubElement(result, "input")
        for key, value in self.parameters.items():
            parameter = ET.SubElement(input, "parameter")
            parameter.set("name", key)
            parameter.set("value", str(value))

        # output
        output = ET.SubElement(result, "output")
        for key, value in self.variables.items():
            variable = ET.SubElement(output, "variable")
            variable.set("name", key)
            variable.set("value", str(value))

        # info
        info = ET.SubElement(result, "info")
        for key, value in self.info.items():
            item = ET.SubElement(info, "item")
            item.set("name", key)
            item.set("value", str(value))

        # solution
        solution = ET.SubElement(result, "solution")
        solution.set("solved", "1" if self.solved else "0")  
        solution.set("geometry", self.geometry)
                    
        tree = ET.ElementTree(variant)
        tree.write(filename, xml_declaration = True, encoding='UTF-8')
