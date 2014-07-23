import xml.etree.ElementTree as ET
import pickle
import pythonlab

from variant import ModelData, ModelBase, ModelDict
from glob import glob
from json import loads
from os.path import basename, splitext

def create_model_dict(directory):   
    md = ModelDict()
    md.load(ModelBase, directory)
    return md

def convert_xml_to_pickle(directory):
    for file_name in glob('{0}/*.rst'.format(directory)):
        model_data = ModelData()
        tree = ET.parse('{0}/{1}'.format(directory, file_name))
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
        if (geometry_image is not None):
            model_data.info['_geometry'] = str(geometry_image.attrib["source"])
        
        # images
        images = solution.findall('images')[0]
        model_data.info['images'] = []
        for image in images.findall('image'):
            model_data.info['images'].append(str(image.attrib["source"]))

        # save
        with open('{0}/{1}.pickle'.format(directory, splitext(basename(file_name))[0]), 'wb') as outfile:
            pickle.dump(model_data, outfile, pickle.HIGHEST_PROTOCOL)

if __name__ == '__main__':
    """
    convert_xml_to_pickle(pythonlab.datadir('/data/sweep/act/solutions'))
    md = _create_model_dict(pythonlab.datadir('/resources/test/test_suite/optilab/genetic/solutions'))
    """
    pass