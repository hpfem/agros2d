#!/usr/bin/python

from xml.dom import minidom
import os

class WeakForm:
    def __init__(self):        
        self.id = ''
        self.type = ''
        self.integral_type = ''        
        self.coordinate_type = ''
        self.analysis_type = ''
        self.i = 0
        self.j = 0
        self.expression = ''
        self.variables = []
        
    def get_temp_class_name(self):
        class_name =  'Custom' + self.type.capitalize() + 'Form'  \
		    + self.integral_type.capitalize()						
        return class_name	
       
    def get_class_name(self):
        class_name =  'Custom' + self.type.capitalize() + 'Form'  \
            + self.integral_type.capitalize() + '_' + str(self.i) \
            + '_'  + str(self.j)                        
        return class_name    
    
    def get_function_name(self):              
        function_name =  'custom_' + self.type + '_form_' + self.integral_type                                
        return function_name    

    def write_h_code(self, h_template):                                        
        h_code = ''                                                                             
        for node in h_template.getElementsByTagName(self.get_function_name()):                        
            string = node.childNodes[0].nodeValue                                                                                                                                                                                  
            name = self.get_temp_class_name()             
            replaced_string = string.replace(name, name + '_' + str(self.i) \
                + '_'  + str(self.j))            
            replaced_string = replaced_string.replace('//expression', 
                self.parse_expression(self.expression)) + '\n\n'             
            h_code += replaced_string                                                                                                                           
        return h_code 
    
    def write_cpp_code(self, cpp_template):                                
        function_types = ['','_value', '_ord']
        cpp_code = ''        
        for function_type in function_types:         
            node = cpp_template.getElementsByTagName('variable_definition')[0]
            variable_def_temp = node.childNodes[0].nodeValue                                            
            for node in cpp_template.getElementsByTagName(self.get_function_name() + function_type):                        
                string = node.childNodes[0].nodeValue                                                                                                                                                                                                  
                name = self.get_temp_class_name()             
                replaced_string = string.replace(name, name + '_' + str(self.i) \
                + '_'  + str(self.j))            
                replaced_string = replaced_string.replace('//expression', 
                            self.parse_expression(self.expression)) + '\n\n'             
                variable_defs = ''            
                for variable in self.variables:                    
                    variable_string = variable_def_temp.replace('variable_short', 
                                    variable.short_name)                    
                    variable_string = variable_string.replace('variable', 
                                    variable.id)
                    variable_defs += variable_string
                    variable_defs = variable_defs.replace('material', variable.type)                                                                                        
                replaced_string = replaced_string.replace('//variable_definition', 
                                                          str(variable_defs))                
                cpp_code += replaced_string                                                                                                                           
        return cpp_code            
            
    def parse_expression(self, expression):
        operators = ['*', '+', '/', '-', '=']        
       
        variables = self.variables
        parsed_exp = expression               
        replaces = { ' ': '', 
                     '*x*':'*e->x[i]*',
                     '*r*': '*e->x[i]*',
                     'udr': 'u->dx[i]',
                     'vdr': 'v->dx[i]',
                     'udz': 'u->dy[i]',
                     'vdz': 'v->dy[i]',                     
                     'updr': 'u_ext[this->i]->dx[i]',                     
                     'updz': 'u_ext[this->i]->dy[i]',                     
                     'udx': 'u->dx[i]',
                     'vdx': 'v->dx[i]',
                     'udy': 'u->dy[i]',
                     'vdy': 'v->dy[i]',
                     'updx': 'u_ext[this->i]->dx[i]',
                     'updy': 'u_ext[this->i]->dy[i]',
                     'upval': 'u_ext[this->i]->val[i]',
                     'uval': 'u->val[i]',
                     'vval': 'v->val[i]' }            
         
        for key, value in replaces.iteritems():            
            parsed_exp = parsed_exp.replace(key, value)

        for variable in variables:
            parsed_exp = parsed_exp.replace(variable.short_name, variable.short_name + '.value()')
        
        for operator in operators:        
            parsed_exp = parsed_exp.replace(operator, ' ' + operator + ' ')
        parsed_exp = parsed_exp.replace(' - >', '->')

        parsed_exp = 'result += wt[i] * (' + parsed_exp + ');'                  
        return parsed_exp

class Material:
    def __init__(self):
        self.id = ''        
        self.name = ''
        self.type = ''                
        self.weakforms = []        

class Boundary:
    def __init__(self):
        self.id = ''
        self.name = ''
        self.type = ''        
        self.weakforms = []
        self.variables = []
                        
class Variable:
    def __init__(self):
        self.id = ''
        self.type = ''        
        self.name = ''
        self.short_name = ''
        self.units = ''  
        
    def write_cpp_code(self):
        pass
                
class Constant:
    def __init__(self):        
        self.id = ' '
        self.value = 0

class PartModule:
    def __init__(self):        
        self.id = ''
        self.name = ''
        self.description = ''        
        self.analysis = ''
        self.coordinate_type = '' 
        self.constants = []        
        self.weakforms = []        
        self.used_weakforms = set([])     
        self.forms_number = 0
        
class Module:
    def __init__(self):        
        self.id = ''
        self.name = ''
        self.description = ''        
        self.materials = [] 
        self.boundaries = []
        self.constants = []
        self.variables = []
        
        
    def info(self):
        print 'ID: ', self.id
        print 'Name: ', self.name
        print 'Description: ', self.description
        i = 0                                
        print  '\nConstants:'        
        print '--------------------------------'                
        for constant in self.constants:
            print constant.id, constant.value
            i += 1                 
        print '\nAnalysis:'        
        print '--------------------------------'        
        for material in self.materials:
            print '\n--------------------------------'        
            print material.name            
            print '--------------------------------'            
            print '\nmatrix forms:'            
            for variable in module.variables:
                print variable.name
            for weakform in material.weakforms:
                print weakform.type, weakform.coordinate_type, weakform.integral_type
                print weakform.i, weakform.j, weakform.expression

        print 'Boundaries'                              
        for boundary in self.boundaries:                                          
            print 'Variables'            
            for variable in boundary.variables:
                print variable.name                
            for weakform in boundary.weakforms:
                print weakform.id, weakform.expression
                                
                                        
    def write_code(self):                                                     
        module_types = []
        part_modules = []
        part_module = PartModule()        
        for material in self.materials:  
            for weakform in material.weakforms:                             
                part_module_id = module.id + '_' + material.name + '_' \
                    + weakform.coordinate_type                                
                if (part_module_id in module_types):
                    index = module_types.index(part_module_id)                                          
                    part_module = part_modules[index]
                                         
                else:
                    module_types.append(part_module_id)                    
                    part_module = PartModule()                    
                    part_module.name = self.name
                    part_module.id = part_module_id
                    part_module.description = self.description
                    part_module.coordinate_type = weakform.coordinate_type
                    part_module.constants = self.constants                    
                    part_module.materials = self.materials = []                    
                    part_module.analysis = material.name                    
                    part_modules.append(part_module)                                
                weakform.variables = module.variables
                part_module.weakforms.append(weakform)                
        
        for boundary in self.boundaries:                        
            for weakform in boundary.weakforms: 
                if (weakform.id in module_types):                                                                
                    index = module_types.index(weakform.id)                    
                    part_module = part_modules[index] 
                    weakform.variables = boundary.variables
                    part_module.weakforms.append(weakform)
                                  
        # opens xml templates        
                
        fread = open('weakform_cpp.tem', 'r')
        cpp_template = minidom.parse(fread).documentElement        
        fread.close()        
        fread = open('weakform_h.tem', 'r')
        h_template = minidom.parse(fread).documentElement        
        fread.close()                     
        fread = open('weakform_factory_h.tem', 'r')       
        factory_template = minidom.parse(fread).documentElement
        fread.close()

        factory_file_str = ''        
        node = factory_template.getElementsByTagName('head')[0]            
        string = node.childNodes[0].nodeValue        
        factory_file_str += string                                                                    

        
        for part_module in part_modules:                                    
            cpp_file_str = ''            
            h_file_str = ''            
            filename = (part_module.id)    
            
            node = cpp_template.getElementsByTagName('head')[0]            
            string = node.childNodes[0].nodeValue                
            cpp_file_str += string             

            node = h_template.getElementsByTagName('head')[0]            
            string = node.childNodes[0].nodeValue                
            h_file_str += string             
                                                    
            node = cpp_template.getElementsByTagName('includes')[0]            
            string = node.childNodes[0].nodeValue                                              
            string = string.replace('general_weakform', filename)           
            cpp_file_str += string    
            node = h_template.getElementsByTagName('includes')[0]            
            string = node.childNodes[0].nodeValue                                              
            string = string.replace('general_weakform', filename)
            h_file_str += string    
            
            node = cpp_template.getElementsByTagName('namespaces')[0]            
            string = node.childNodes[0].nodeValue                                              
            string = string.replace('general_weakform', filename)
            string = string.replace('_', '')
            cpp_file_str += string

            node = h_template.getElementsByTagName('namespaces')[0]            
            string = node.childNodes[0].nodeValue                                              
            string = string.replace('general_weakform', filename)
            string = string.replace('_', '')            
            h_file_str += string + '\n'
                                
            class_names = set([])            
            for weakform in part_module.weakforms:                 
                class_names.add(weakform.get_class_name())
                weakform.write_cpp_code(cpp_template)
                cpp_file_str += weakform.write_cpp_code(cpp_template)            
                h_file_str += weakform.write_h_code(h_template)
            
            node = cpp_template.getElementsByTagName('footer')[0]                            
            for class_name in class_names:
                string = node.childNodes[0].nodeValue
                string = string.replace('ClassName', class_name) 
                cpp_file_str += string         

            node = h_template.getElementsByTagName('footer')[0]
            string = node.childNodes[0].nodeValue
            h_file_str += string
            
            source_file = open(weakform_dir + filename + '.cpp', 'w')
            source_file.write(cpp_file_str)
            source_file.close()        
            
            header_file = open(weakform_dir + filename + '.h', 'w')
            header_file.write(h_file_str)            
            header_file.close()
            
            # append to weakform.pri
            weakform_pri_file = open(weakform_dir + 'weakform.pri', 'a')
            weakform_pri_file.write('SOURCES += ' + filename + '.cpp\n')
            weakform_pri_file.close()
        
        factory_file = open(weakform_dir + 'weakform_factory.h', 'w')       
        factory_file.write(factory_file_str)        
        factory_file.close()
            
def parse_xml_file(filename, modules):
    # constants definition
    analysis_types = ['harmonic', 'transient', 'steadystate']
    coordinate_types  = ['axi', 'planar']    
    boundaries_types = ['essential', 'natural', 'vector']
    weakform_types = ['matrix', 'vector']
    
    
    # opens file and initializes xml parser    
    fread = open(modules_dir + filename, 'r')
    xmldoc = minidom.parse(fread).documentElement
    fread.close()
    
    module = Module()
    #parses general information
    nodes = xmldoc.getElementsByTagName('general')
    for subnode in nodes:
        if subnode.attributes != None:
            module.id =  subnode.attributes['id'].value
            module.name = subnode.attributes['name'].value
            
    nodes = xmldoc.getElementsByTagName('description')
    subnode = nodes[0]
    module.description = subnode.childNodes[0].data
        
    # parses constans element
    # ToDo: Make controls
    nodes = xmldoc.getElementsByTagName('constants')
    for subnode in nodes[0].childNodes:
        if subnode.attributes != None:                          
            const = Constant()
            const.id = str(subnode.attributes['id'].value)
            const.value = float(subnode.attributes['value'].value)
            module.constants.append(const)
                
    # parses material element
    # ToDo: Maket checks and controls    
    
    # parses weak-forms    
    nodes = xmldoc.getElementsByTagName('material')    
    for subnode in nodes[0].childNodes:
        if subnode.nodeName == 'item':           
            variable = Variable()            
            variable.id = subnode.attributes['id'].value
            variable.name = str(subnode.attributes['name'].value)
            variable.short_name = str(subnode.attributes['shortname'].value)
            variable.type = 'material'            
            variable.units = str(subnode.attributes['unit'].value)            
            module.variables.append(variable)
    
    for subnode in nodes[0].childNodes:        
        if subnode.nodeName in analysis_types:
            material = Material()    
            analysis_type = subnode.nodeName    
            material.name =  analysis_type      
                  
            for subsubnode in subnode.childNodes:
                if subsubnode.nodeName in weakform_types:                    
                    for i in range(subsubnode.attributes.length):                        
                        coordinate_type = str(subsubnode.attributes.item(i).name)                                                                        
                        if coordinate_type in coordinate_types:                                                                                   
                            weakform = WeakForm()                            
                            weakform.coordinate_type = coordinate_type                            
                            weakform.analysis_type = analysis_type                                   
                            weakform.type = subsubnode.nodeName                                
                            weakform.integral_type = 'vol'    
                            weakform.expression = \
                                str(subsubnode.attributes[coordinate_type].value)                            
                            weakform.i = int(subsubnode.attributes['i'].value)                                        
                            if subsubnode.hasAttribute('j'):
                                weakform.j = int(subsubnode.attributes['j'].value)                                 
                            material.weakforms.append(weakform)
            module.materials.append(material)
            
    nodes = xmldoc.getElementsByTagName('boundary')         
       
    boundary = Boundary()            
    for subnode in nodes[0].childNodes:       
        if subnode.nodeName == 'item':
            variable = Variable()
            variable.id = subnode.attributes['id'].value
            variable.name = subnode.attributes['name'].value            
            variable.type = 'boundary'             
            variable.short_name = subnode.attributes['shortname'].value                       
            variable.unit = subnode.attributes['unit'].value
            boundary.variables.append(variable)          
    
    for subnode in nodes[0].childNodes:        
        if subnode.nodeName in analysis_types:
            analysis_type = subnode.nodeName             
            for subsubnode in subnode.childNodes:
                if subsubnode.nodeName == 'boundary':
                    boundary.name = subsubnode.attributes['name'].value                                                              
                    for element in  subsubnode.childNodes:                                               
                        if element.nodeName in boundaries_types:                                                         
                            boundary_type = element.nodeName                              
                            if boundary_type in weakform_types :      
                                for i in range(element.attributes.length):                                                                            
                                    coordinate_type = str(element.attributes.item(i).name)                                                                        
                                    if coordinate_type in coordinate_types:
                                        weakform = WeakForm()
                                        weakform.type = boundary_type
                                        weakform.i = int(element.attributes['i'].value)
                                        weakform.id = module.id + '_'  + \
                                            analysis_type 
                                        weakform.coordinate_type = coordinate_type
                                        weakform.integral_type = 'surf'
                                        weakform.analysis_type = analysis_type                                                 
                                        weakform.expression = \
                                        str(element.attributes[coordinate_type].value)
                                        weakform.id += '_' + coordinate_type
                                        boundary.weakforms.append(weakform)
                            else:
                                weakform = WeakForm()                                
                                weakform.i = int(element.attributes['i'].value)
                                weakform.id = module.id + '_'  + \
                                    analysis_type 
                                boundary.weakforms.append(weakform)                                
    module.boundaries.append(boundary)
    modules.append(module)

#control output    

module_files = ['electrostatic.xml', 'current.xml']
modules_dir = '../../../modules/'
weakform_dir = './'


#for weakform in list_dir:
#    if weakform[::-1][:4][::-1] == '.xml':
#        module_files.append(weakform)         

modules = []
for module_file in module_files:        
    parse_xml_file(module_file, modules)

# remove pri file
try:
    os.remove(weakform_dir + 'weakform.pri')
except:
    pass

for module in modules:    
    #module.info()
    module.write_code()    
    
