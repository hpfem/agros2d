__empty_svg__ = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import pickle
from os.path import dirname, isdir
from os import makedirs

class Parameters(dict):
    def __init__(self, defaults):
        dict.__init__(self)
        self._defaults = defaults

    def __getitem__(self, key):
        if key in dict.keys(self):
            return dict.__getitem__(self, key)
        elif key in self._defaults.keys():
            return self._defaults[key]
        else:
            raise KeyError(key)

class ModelData:
    def __init__(self):
        self.defaults = dict()
        self.parameters = Parameters(self.defaults)
        self.variables = dict()
        self.info = dict()

        self.geometry_image = __empty_svg__
        self.images = list()

        self.solved = False

class ModelBase(object):
    def __init__(self):
        self._data = ModelData()

    @property
    def parameters(self):
        """ Input parameters """
        return self._data.parameters

    @parameters.setter
    def parameters(self, value):
        self._data.parameters = value

    @property
    def defaults(self):
        """ Default values for parameters """
        return self._data.defaults

    @defaults.setter
    def defaults(self, value):
        self._data.defaults = value

    @property
    def variables(self):
        """ Output variables """
        return self._data.variables

    @variables.setter
    def variables(self, value):
        self._data.variables = value

    @property
    def info(self):
        """ Optional info """
        return self._data.info

    @info.setter
    def info(self, value):
        self._data.info = value

    @property
    def geometry_image(self):
        """ Geometry image """
        return self._data.geometry_image

    @geometry_image.setter
    def geometry_image(self, value):
        self._data.geometry_image = value

    @property
    def images(self):
        """ Optional images """
        return self._data.images

    @images.setter
    def images(self, value):
        self._data.images = value

    @property
    def solved(self):
        """ Solution state """
        return self._data.solved

    @solved.setter
    def solved(self, value):
        self._data.solved = value

    def create(self):
        pass

    def solve(self):
        pass

    def process(self):
        pass    
        
    def load(self, file_name):
        """ Load model data from file """
        with open(file_name, 'rb') as infile:
            self._data = pickle.load(infile)
                                
    def save(self, file_name):
        """ Save model data from file """
        directory = dirname(file_name)
        if not isdir(directory):
            makedirs(directory)

        with open(file_name, 'wb') as outfile:
            pickle.dump(self._data, outfile, pickle.HIGHEST_PROTOCOL)

    def clear(self):
        self._data = ModelData()

def create_model_dict(directory):   
    md = ModelDict()
    md.load('{0}/*.pickle'.format(directory))
    return md

def convert_xml_to_pickle(directory):
    import xml.etree.ElementTree as ET
    from glob import glob
    from json import loads
    from os.path import basename, splitext
                
    for file_name in glob('{0}/*.rst'.format(directory)):
        model_data = ModelData()
        tree = ET.parse(directory + "/" + file)
        variant = tree.getroot()
        results = variant.findall('results')[0]
        result = results.findall('result')[0]
                        
        # solution
        solution = result.findall('solution')[0]
        model_data.solved = int(solution.attrib['solved'])
        
        # input
        input = result.findall('input')[0]
        for par in input.iter(tag='parameter'):             
            model_data.parameters[par.attrib["name"]] = loads(par.attrib["value"])
        
        # output
        output = result.findall('output')[0]
        for var in output.iter(tag='variable'): 
            model_data.variables[var.attrib["name"]] = loads(var.attrib["value"])
        
        # info
        info = result.findall('info')[0]
        for item in info.iter(tag='item'): 
            model_data.info[item.attrib["name"]] = loads(item.attrib["value"])
        
        # geometry
        geometry_image = solution.find('geometry_image')
        if (geometry_image is None):
            model_data.geometry_image = __empty_svg__
        else:
            model_data.geometry_image  = str(geometry_image.attrib["source"])
        
        # images
        images = solution.findall('images')[0]
        for image in images.findall('image'):
            model_data.images.append(str(image.attrib["source"]))

        # save
        save_pickle(m, )
        with open(file_name, 'wb') as outfile:
            pickle.dump(model_data, '{0}/{1}.pickle'.format(directory, splitext(basename(file))[0]), pickle.HIGHEST_PROTOCOL)

if __name__ == '__main__':
    #import pythonlab
    #convert_xml_to_pickle(pythonlab.datadir('/data/sweep/act/solutions'))
    #md = create_model_dict(pythonlab.datadir('/resources/test/test_suite/optilab/genetic/solutions'))
    pass
