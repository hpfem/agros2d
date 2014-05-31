from ast import literal_eval

class ModelBase(object):
    def __init__(self):
        self._defaults = dict()
        self._parameters = dict()
        self._variables = dict()
        self._info = dict()
        self._solved = False

        self._images = list()
        self._geometry_image = ""

    @property
    def parameters(self):
        """ Input parameters """
        return self._parameters

    @parameters.setter
    def parameters(self, value):
        self._defaults = value

    @property
    def defaults(self):
        """ Default values for parameters """
        return self._defaults

    @defaults.setter
    def defaults(self, value):
        self._defaults = value

    @property
    def variables(self):
        """ Output variables """
        return self._variables

    @variables.setter
    def variables(self, value):
        self._variables = value

    @property
    def info(self):
        """ Optional info """
        return self._info

    @info.setter
    def info(self, value):
        self._info = value

    @property
    def geometry_image(self):
        """ Geometry image """
        return self._geometry_image

    @geometry_image.setter
    def geometry_image(self, value):
        self._geometry_image = value

    @property
    def images(self):
        """ Optional images """
        return self._images

    @images.setter
    def images(self, value):
        self._images = value

    @property
    def solved(self):
        """ Solution state """
        return self._solved

    @solved.setter
    def solved(self, value):
        self._solved = value

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

        # TODO: only one result
        results = variant.findall('results')[0]
        result = results.findall('result')[0]

        # solution
        solution = result.findall('solution')[0]
        self.solved = int(solution.attrib['solved'])

        # input
        input = result.findall('input')[0]
        for par in input.findall('parameter'):
            self.parameters[par.attrib["name"]] = literal_eval(par.attrib["value"])

        # output
        output = result.findall('output')[0]
        for var in output.findall('variable'):
            self.variables[var.attrib["name"]] = literal_eval(var.attrib["value"])

        # info
        info = result.findall('info')[0]
        for item in info.findall('item'):
            self.info[item.attrib["name"]] = literal_eval(item.attrib["value"])

        # geometry
        geometry_image = solution.find('geometry_image')
        self.geometry_image  = str(geometry_image.attrib["source"])

        # images
        images = solution.findall('images')[0]
        for image in images.findall('image'):
            self.images.append(str(image.attrib["source"]))

    def save(self, filename):
        import xml.etree.cElementTree as ET

        variant = ET.Element("variant:variant")        
        variant.set("xmlns:variant", "XMLOptVariant")
        variant.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")

        # results
        results = ET.SubElement(variant, "results")
        result = ET.SubElement(results, "result")

        # solution
        solution = ET.SubElement(result, "solution")
        solution.set("solved", "1" if self.solved else "0")

        # input
        input = ET.SubElement(result, "input")
        for key, value in self.parameters.items():
            parameter = ET.SubElement(input, "parameter")
            parameter.set("name", key)

            if (type(value) == str):
                parameter.set("value", "'{0}'".format(str(value)))
            else:
                parameter.set("value", str(value))

        # output
        output = ET.SubElement(result, "output")
        for key, value in self.variables.items():
            variable = ET.SubElement(output, "variable")
            variable.set("name", key)

            if (type(value) == str):
                variable.set("value", "'{0}'".format(str(value)))
            else:
                variable.set("value", str(value))

        # info
        info = ET.SubElement(result, "info")
        for key, value in self.info.items():
            item = ET.SubElement(info, "item")
            item.set("name", key)

            if (type(value) == str):
                item.set("value", "'{0}'".format(str(value)))
            else:
                item.set("value", str(value))

        # geometry
        geometry_image = ET.SubElement(solution, "geometry_image")
        geometry_image.set("source", self.geometry_image)

        # images
        images = ET.SubElement(solution, "images")
        for image in self.images:
            item = ET.SubElement(images, "image")
            item.set("source", image)
     
        tree = ET.ElementTree(variant)
        tree.write(filename, xml_declaration = True, encoding='UTF-8')
