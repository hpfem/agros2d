#!/usr/bin/python

from xml.dom import minidom
import os
from expression_parser import NumericStringParser
# becouse of parametr processing
# import sys 

class WeakForm:
    def __init__(self):        
        self.id = ''
        self.type = ''
        self.integral_type = ''        
        self.coordinate_type = ''
        self.analysis_type = ''
        self.boundary_type = ''        
        self.i = 0
        self.j = 0
        self.expression = ''
        self.name = ''
        self.variables = []
        
    def get_temp_class_name(self):
        class_name =  'Custom' + self.type.capitalize() + 'Form'  \
		    + self.integral_type.capitalize()						
        return class_name	
       
    def get_class_name(self):
        class_name =  'Custom' + self.type.capitalize() + 'Form'  \
            + self.integral_type.capitalize() + '_' + self.boundary_type + '_' + str(self.i) \
            + '_'  + str(self.j)                        
        return class_name    
    
    def get_function_name(self):              
        function_name =  'custom_' + self.type + '_form_' + self.integral_type                                
        return function_name    

    def get_factory_code(self, factory_template):
        if (self.type == 'vector'):
            if(self.integral_type == 'vol'):
                string = factory_template.getElementsByTagName('condition_vector_vol')[0].childNodes[0].nodeValue
            else:    
                string = factory_template.getElementsByTagName('condition_vector_surf')[0].childNodes[0].nodeValue
        else:
            if(self.integral_type == 'vol'):
                string = factory_template.getElementsByTagName('condition_matrix_vol')[0].childNodes[0].nodeValue
            else:
                string = factory_template.getElementsByTagName('condition_matrix_surf')[0].childNodes[0].nodeValue
                     
        string = string.replace('class_name', self.id)
        string = string.replace('axi', 'axisymmetric')                        
        string = string.replace('row_index', str(self.i-1))                        
        string = string.replace('column_index', str(self.j-1))
        string = string.replace('boundary_type', self.boundary_type)        
        namespace = self.id.replace('_','')
        string = string.replace('namespace', namespace)
        function_name = self.get_class_name();        
        string = string.replace('FunctionName', function_name)                        
        factory_code = []
        factory_code.append(self.get_temp_class_name())
        factory_code.append(string)                        
        return factory_code
        
    def get_h_code(self, h_template):                                        
        h_code = ''   
        node = h_template.getElementsByTagName('variable_declaration')[0]
        variable_def_temp = node.childNodes[0].nodeValue                                                                                                                     
        for node in h_template.getElementsByTagName(self.get_function_name()):                        
            string = node.childNodes[0].nodeValue                                                                                                                                                                                  
            name = self.get_temp_class_name() 
            variable_defs = ''
            replaced_string = string.replace(name, name + '_' + self.boundary_type + '_' + str(self.i) \
                + '_'  + str(self.j))            
            for variable in self.variables:                                    
                variable_string = variable_def_temp.replace('variable_short', 
                                        variable.short_name)                    
                variable_string = variable_string.replace('variable', 
                                        variable.id)
                variable_defs += variable_string
                variable_defs = variable_defs.replace('material', variable.type)                                                                                        
            replaced_string = replaced_string.replace('//variable_declaration', 
                                                              str(variable_defs))                             
            h_code += replaced_string                                                                                                                           
        return h_code 
    
    def get_cpp_code(self, cpp_template):                                
        function_types = ['','_value', '_ord']
        cpp_code = ''        
        for function_type in function_types:         
            node = cpp_template.getElementsByTagName('variable_definition')[0]
            variable_def_temp = node.childNodes[0].nodeValue                                            
            for node in cpp_template.getElementsByTagName(self.get_function_name() + function_type):                        
                string = node.childNodes[0].nodeValue                                                                                                                                                                                                  
                name = self.get_temp_class_name()             
                replaced_string = string.replace(name, name + '_' + self.boundary_type + '_' + str(self.i) \
                + '_'  + str(self.j))            
                if function_type == '':
                    variable_defs = '' ;                    
                    for variable in self.variables:                    
                        variable_string = variable_def_temp.replace('variable_short', 
                                        variable.short_name)                    
                        variable_string = variable_string.replace('variable', 
                                        variable.id)
                        variable_defs += variable_string
                        variable_defs = variable_defs.replace('material', variable.type)                                                                                        
                    replaced_string = replaced_string.replace('//variable_definition', 
                                                              str(variable_defs))     
                if function_type == '_ord':
                    expression = self.parse_expression(self.expression, True)                     
                else:
                    expression = self.parse_expression(self.expression, False)                                                                                                                    
                replaced_string = replaced_string.replace('//expression', 
                                expression) + '\n\n'                
                cpp_code += replaced_string                                                                                                                           
        return cpp_code            
            
    def parse_expression(self, expression, without_variables):                        
        replaces = { 'PI': 'M_PI',
                     'f': 'Util::scene()->problemInfo()->frequency',                     
                     'x':'e->x[i]',
                     'r': 'e->x[i]',
                     'udr': 'u->dx[i]',
                     'vdr': 'v->dx[i]',
                     'udz': 'u->dy[i]',
                     'vdz': 'v->dy[i]',                     
                     'updr': 'u_ext[this->j]->dx[i]',                     
                     'updz': 'u_ext[this->j]->dy[i]',                     
                     'udx': 'u->dx[i]',
                     'vdx': 'v->dx[i]',
                     'udy': 'u->dy[i]',
                     'vdy': 'v->dy[i]',
                     'updx': 'u_ext[this->j]->dx[i]',
                     'updy': 'u_ext[this->j]->dy[i]',
                     'upval': 'u_ext[this->j]->val[i]',
                     'uval': 'u->val[i]',
                     'vval': 'v->val[i]', 
                     'uptval': 'ext->fn[this->i]->val[i]',
                     'deltat': 'Util::scene()->problemInfo()->timeStep.number()'                       
                     }            

        symbols = ['x', 'y', 'r', 'z', 'f', 'udr', 'udz', 'udx', 'udy',
                   'vdr', 'vdz', 'vdx', 'vdy', 'updr', 'updx', 'updy', 'updz',
                   'uval', 'vval', 'upval', 'deltat', 'uptval', 'PI']
                           
        variables = []               
        for variable in self.variables:        
            symbols.append(variable.short_name)
            variables.append(variable.short_name)
             
        for const in self.constants:
            symbols.append(const.id)
                
        parser = NumericStringParser(symbols, replaces, variables, without_variables)                        
        if not(expression.replace(' ','') == ''):
            expression_list = parser.parse(expression).asList()                                  
            parsed_exp = parser.get_expression(expression_list)                             
        else:
            parsed_exp =''
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
            for variable in self.variables:
                print variable.name
            for weakform in material.weakforms:
                print weakform.type, weakform.coordinate_type, weakform.integral_type
                print weakform.i, weakform.j, weakform.expression

        print 'Boundaries'                              
        for boundary in self.boundaries:                                          
            print 'Variables'            
            for variable in boundary.variables:
                print variable.name                
            print 'Weakforms'        
            for weakform in boundary.weakforms:
                print weakform.id, weakform.i, weakform.j, weakform.type, weakform.coordinate_type, weakform.integral_type                                                                   
                                        
    def extract_modules(self):
        module_types = []
        part_modules = []
        part_module = PartModule()          
        
        for material in self.materials:  
            for weakform in material.weakforms:                             
                part_module_id = self.id + '_' + material.name + '_' \
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
                    part_module.materials = self.materials
                    part_module.analysis = material.name                    
                    part_modules.append(part_module)                                
                weakform.variables = self.variables
                weakform.constants = self.constants
                weakform.id = part_module.id                
                part_module.weakforms.append(weakform)                
        
        for boundary in self.boundaries:                                    
                for weakform in boundary.weakforms: 
                    if (weakform.id in module_types):                                                                
                        if weakform.integral_type == 'surf':                             
                            index = module_types.index(weakform.id)                    
                            part_module = part_modules[index] 
                            weakform.variables = boundary.variables
                            weakform.constants = self.constants                                        
                            part_module.weakforms.append(weakform)               
        return part_modules;
        
                                                                                      
    def get_code(self, param_templates):
       templates = dict() 
       templates['.cpp'] = param_templates['weakform_cpp.tem']
       templates['.h'] = param_templates['weakform_h.tem']
       file_strings = dict()
       
       part_modules = self.extract_modules()                                                
       factory_codes = []                   
       for part_module in part_modules:                                    
            filename = (part_module.id)                                     
            for key in templates.iterkeys():                           
               file_string_name = filename + key
               node = templates[key].getElementsByTagName('head')[0]            
               string = node.childNodes[0].nodeValue                               
               file_strings[file_string_name] = string 
               node = templates[key].getElementsByTagName('includes')[0]            
               string = node.childNodes[0].nodeValue                                              
               string = string.replace('general_weakform', filename)           
               file_strings[file_string_name] += string  
               node = templates[key].getElementsByTagName('namespaces')[0]            
               string = node.childNodes[0].nodeValue                                              
               string = string.replace('general_weakform', filename)
               string = string.replace('_', '')
               file_strings[file_string_name] += string
            
               class_names = set([])                            
               
               for weakform in part_module.weakforms:                                     
                    if key == '.cpp':
                        class_names.add(weakform.get_class_name())
                        file_strings[file_string_name] += weakform.get_cpp_code(templates[key])            
                        factory_code =  weakform.get_factory_code(param_templates['weakform_factory_h.tem'])
                        factory_codes.append(factory_code)
                    if key == '.h':                        
                        file_strings[file_string_name] += weakform.get_h_code(templates[key])                                

               node = templates[key].getElementsByTagName('footer')[0]                            
               if key == '.cpp':                        
                   for class_name in class_names:
                       string = node.childNodes[0].nodeValue
                       string = string.replace('ClassName', class_name)                        
                       file_strings[file_string_name] += string             
               if key == '.h':       
                   string = node.childNodes[0].nodeValue                                     
                   file_strings[file_string_name] += string                                                                  
       return file_strings , factory_codes    
                            
 
    def write_code(self, weakform_dir, param_templates):                                                     
            weakform_pri_file = open(weakform_dir + 'weakform.pri', 'a')            
            files, conditions = self.get_code(param_templates)            

            for filename in files.iterkeys():                                            
                output_file = open(weakform_dir + filename , 'w')
                output_file.write(files[filename])            
                output_file.close()                            
                # append to weakform.pri                
                if filename[::-1][:4][::-1] == '.cpp':                
                    weakform_pri_file.write('SOURCES += ' + filename + '\n')            
            weakform_pri_file.close()
  
class XmlParser:
    
    def __init__(self, modules):       
       self.module_files = [] 
       for module in modules:
           self.module_files.append(module + '.xml')
       self.template_file_names = ['weakform_cpp.tem', 'weakform_h.tem', 'weakform_factory_h.tem']
       self.modules_dir = '../../../modules/'
       self.weakform_dir = './'
       self.weakform_factory_dir = '../'       
       self.modules = []
       self.templates = dict()        
       
       for template_file in self.template_file_names:        
           fread = open(template_file, 'r')
           template = minidom.parse(fread).documentElement        
           fread.close()    
           self.templates[template_file] = template                   
                                  
    def process(self):
       # print sys.argv
        # read tamplate files        
#        for filename in os.listdir('.'):
#            if filename[::-1][:4][::-1] == '.tem':            
                
        for module_file in self.module_files:        
            self.parse_xml_file(module_file)
            
        # remove pri file
        try:
            os.remove(self.weakform_dir + 'weakform.pri')
        except:
            pass
        
        files = []
        conditions = []
        
        for module in self.modules:                           
            module_files, module_conditions = module.get_code(self.templates)            
            module.write_code(self.weakform_dir, self.templates)                                       
            conditions.extend(module_conditions)                                      
            files.extend(module_files)
       
       # writes weakform_factory.h         
        factory_code_str = ''    
        key = 'weakform_factory_h.tem'        
        node = self.templates[key].getElementsByTagName('head')[0]  
        factory_code_str += node.childNodes[0].nodeValue      
       
        for module_file in files:        
            if module_file[::-1][:2][::-1] == '.h':                
                node = self.templates[key].getElementsByTagName('includes')[0]  
                string = node.childNodes[0].nodeValue 
                string = string.replace("general_weakform.h", module_file)
                factory_code_str += string

        weakform_temps = ['CustomMatrixFormVol','CustomVectorFormVol',
                           'CustomMatrixFormSurf','CustomVectorFormSurf']                    
        for weakform_temp in weakform_temps:                            
            weakform_string = ''
            for condition in conditions:                
                if condition[0] == weakform_temp:
                    weakform_string += condition[1]            
            node = self.templates[key].getElementsByTagName(weakform_temp)[0]                          
            string = node.childNodes[0].nodeValue      
            string = string.replace('//conditions', weakform_string)
            factory_code_str += string
                
        node = self.templates[key].getElementsByTagName('footer')[0]  
        factory_code_str += node.childNodes[0].nodeValue         
        factory_file = open(self.weakform_factory_dir+'weakform_factory.h', 'w')
        factory_file.write(factory_code_str)
        factory_file.close()
        
        
    def parse_xml_file(self, filename):
        # constants definition
        analysis_types = ['harmonic', 'transient', 'steadystate']
        coordinate_types  = ['axi', 'planar']        
        weakform_types = ['matrix', 'vector']
        
        
        # opens file and initializes xml parser    
        fread = open(self.modules_dir + filename, 'r')
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
                                else:
                                    weakform.j = 1
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
                        boundary.id = subsubnode.attributes['id'].value                                                                                                              
                        for element in  subsubnode.childNodes:                                               
                            if element.nodeName in weakform_types:                                                         
                                boundary_type = element.nodeName                                                                                                                                                              
                                if boundary_type in weakform_types :                                          
                                    for i in range(element.attributes.length):                                                                            
                                        coordinate_type = str(element.attributes.item(i).name)                                                                        
                                        if coordinate_type in coordinate_types:                                                                                        
                                            weakform = WeakForm()                                                                                            
                                            weakform.type = boundary_type
                                            weakform.i = int(element.attributes['i'].value)
                                            if element.hasAttribute('j'):                                        
                                                weakform.j = int(element.attributes['j'].value)
                                            else:
                                                weakform.j = 1
                                            weakform.id = module.id + '_'  + \
                                                analysis_type 
                                            weakform.coordinate_type = coordinate_type
                                            weakform.integral_type = 'surf'
                                            weakform.analysis_type = analysis_type                                                 
                                            weakform.expression = \
                                            str(element.attributes[coordinate_type].value)
                                            weakform.id += '_' + coordinate_type                                                                                                                                    
                                            boundary.weakforms.append(weakform)
                                            weakform.boundary_type = boundary.id
                                else:
                                    weakform = WeakForm()                                
                                    weakform.i = int(element.attributes['i'].value)
                                    if element.hasAttribute('j'):                                        
                                        weakform.j = int(element.attributes['j'].value)
                                    else:
                                        weakform.j = 1
                                    weakform.id = module.id + '_'  + \
                                        analysis_type                                     
                                    boundary.weakforms.append(weakform)                                
        module.boundaries.append(boundary)
        self.modules.append(module)