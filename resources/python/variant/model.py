__empty_svg__ = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

class ModelData:
    def __init__(self):
        self.defaults = dict()
        self.parameters = dict()
        self.variables = dict()
        self.info = dict()

        self.geometry_image = ""
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
        
    def load(self, filename):
        m = load_pickle(filename)
        
        self._data = m
                                
    def save(self, filename):
        save_pickle(self._data, filename)
        
def load_pickle(filename):
    import pickle
    
    with open(filename, 'rb') as infile:
        return pickle.load(infile)

def save_pickle(model, filename):        
    import pickle
    
    with open(filename, 'wb') as outfile:
        pickle.dump(model, outfile, pickle.HIGHEST_PROTOCOL)
            
def convert_xml_to_pickle(dir):
    import json
    import xml.etree.ElementTree as ET
    from os.path import basename, splitext
    import os
                
    # load all files in directory
    for file in os.listdir(dir):
        if file.endswith(".rst"):
            m = ModelData()            
                
            tree = ET.parse(dir + "/" + file)
            variant = tree.getroot()
        
            # only one result
            results = variant.findall('results')[0]
            result = results.findall('result')[0]
                        
            # solution
            solution = result.findall('solution')[0]
            m.solved = int(solution.attrib['solved'])
        
            # input
            input = result.findall('input')[0]
            for par in input.iter(tag='parameter'):             
                m.parameters[par.attrib["name"]] = json.loads(par.attrib["value"])
        
            # output
            output = result.findall('output')[0]
            for var in output.iter(tag='variable'): 
                m.variables[var.attrib["name"]] = json.loads(var.attrib["value"])
        
            # info
            info = result.findall('info')[0]
            for item in info.iter(tag='item'): 
                m.info[item.attrib["name"]] = json.loads(item.attrib["value"])
        
            # geometry
            geometry_image = solution.find('geometry_image')
            if (geometry_image is None):
                m.geometry_image = __empty_svg__
            else:
                m.geometry_image  = str(geometry_image.attrib["source"])
        
            # images
            images = solution.findall('images')[0]
            for image in images.findall('image'):
                m.images.append(str(image.attrib["source"]))
                
            # save pickle
            save_pickle(m, dir + "/" + splitext(basename(file))[0] + ".pickle")
                    
class ModelDict(object):
    def __init__(self):
        self._data = dict()

    def append(self, model):
        self._data.append(model)
    
    @property
    def models(self):
        """ Models """
        return self._data

    @models.setter
    def models(self, value):
        self._data = value
        
    def count(self):
        return len(self._data)
        
    def keys(self):
        return sorted(list(self._data.keys()))

    def list(self):
        lst = []
        for k,m in sorted(self._data.items()):
            lst.append({ 'key' : k, 'solved' : m.solved })
                
        return lst
        
    def read_pickles_from_directory(self, dir):
        import os
        
        # clear list
        self._data.clear()
        
        # load all files in directory
        for file in os.listdir(dir):
            if file.endswith(".pickle"):
                m = load_pickle(dir + "/" + file)
                self._data[file] = m

def create_model_dict(solutions_dir):   
    md = ModelDict()
    md.read_pickles_from_directory(solutions_dir)
    
    return md

if __name__ == '__main__':
    pass
    #convert_xml_to_pickle('/home/karban/Projects/agros2d-optilab/data/sweep/act/solutions')
    
    #md = create_model_dict('/home/karban/Projects/agros2d-optilab/resources/test/test_suite/optilab/genetic/solutions')
    #md = create_model_dict('/home/karban/Projects/agros2d-optilab/data/sweep/act/solutions')
    #print(md.count())   
    #print(md.list())