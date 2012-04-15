#!/usr/bin/python
import module_xml as md
from xml.dom import minidom
import os
from expression_parser import NumericStringParser

class Config:     
    modules_dir = '../resources/modules/'
    weakform_dir = './src/'
    factory_dir = './src/' 
    doc_dir = '../resources_source/doc/source/modules/'
    templates_dir = './templates/'
    
    templates = ['template_weakform_cpp.xml', 'template_weakform_h.xml', 'template_weakform_factory_h.xml']
    project_file = 'weakform.pri'

class XmlParser:    
    def __init__(self, modules):               
        self.module_files = [] 
        for module_name in modules:
            module_file = module_name + '.xml'             
            self.module_files.append(module_file)                    
                                       
        self.modules = []
        self.templates = dict()        
        
        # ToDo: Prepare xsd for templates and use generateDS
        for template_file in Config.templates:        
            try:                              
                fread = open(Config.templates_dir + template_file, 'r')
                template = minidom.parse(fread).documentElement        
                fread.close()    
                self.templates[template_file] = template
            except IOError as (errno, strerror):
                #print 'I/O error({0}): {1} "{2}".'.format(errno, strerror, 														  template_file)
                raise
                                  
    def process(self):                              
        for module_file in self.module_files:        
            self.parse_xml_file(module_file)            
            
        # create src directory
        try:
            os.mkdir(Config.weakform_dir)
        except:
            pass

        # remove pri file
        try:
            os.remove(Config.weakform_dir + Config.project_file)
        except:
            pass
        
        files = []
        conditions = []
        
        for module in self.modules:                           
            module_files, module_conditions = module.get_code(self.templates)             
            module.write_code(Config.weakform_dir, self.templates)                                       
            conditions.extend(module_conditions)                                      
            files.extend(module_files)
       
        # writes weakform_factory.h         
        factory_code_str = ''    
        key = 'template_weakform_factory_h.xml'        
        node = self.templates[key].getElementsByTagName('head')[0]  
        factory_code_str += node.childNodes[0].nodeValue      
       
        for module_file in files:                    
            if module_file[::-1][:2][::-1] == '.h':                
                node = self.templates[key].getElementsByTagName('includes')[0]  
                string = node.childNodes[0].nodeValue 
                string = string.replace("general_weakform.h", module_file)
                factory_code_str += string

        weakform_temps = ['CustomMatrixFormVol','CustomVectorFormVol',
                           'CustomMatrixFormSurf','CustomVectorFormSurf', 'CustomEssentialFormSurf']                    
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
        factory_file = open(Config.factory_dir+'weakform_factory.h', 'w')        
        factory_file.write(factory_code_str)
        factory_file.close()
        
        
    def gen_doc(self):                   
        index_string = '.. toctree::\n    :maxdepth: 2\n\n'
        for module in self.modules:            
            index_string += '    ' + module.id + '.rst' + '\n'                
            doc_file = open(Config.doc_dir + module.id + '.rst', 'w') 
            doc_file_str = module.gen_doc()            
            doc_file.write(doc_file_str)
            doc_file.close()         
        index_file = open(Config.doc_dir + 'index.rst', 'w')
        index_file.write(index_string)            
        index_file.close()
        
    def parse_xml_file(self, filename):
         # constants definitions
         coordinate_types = ['planar', 'axi']; 
         weakform_types = ['matrix', 'vector', 'essential']              
         # opens file and initializes xml parser            
         ds_module = md.parse(Config.modules_dir + filename)
                     
         # parses module general information             
         module = Module()       
         module.description = ds_module.general.description        
         module.id = ds_module.general.id
         # parses module constans
         for constant in ds_module.constants.constant:
            const = Constant()            
            const.id = constant.id
            const.value = constant.value
            module.constants.append(const)
                    
         # parses volume information
         # parses quantities
         for ds_quantity in ds_module.volume.quantity:            
            quantity = Quantity()            
            quantity.id = ds_quantity.id
            quantity.name = ds_quantity.id
            quantity.short_name = ds_quantity.shortname
            quantity.type = 'material'            
            quantity.unit = ds_quantity.unit
            module.quantities.append(quantity)
        
         for ds_weakform in ds_module.volume.weakforms.weakform:
             volume = Volume()                 
             volume.name =  ds_weakform.analysistype      
             i  = 0
             n  = len(ds_weakform.matrix)
             for ds_matrix in ds_weakform.matrix + ds_weakform.vector:                 
                 for coordinate_type in coordinate_types:                                          
                         weakform = WeakForm()                            
                         weakform.integral_type = 'vol'                     
                         weakform.analysis_type = ds_weakform.analysistype                         
                         if (i < n):    
                             weakform.type = 'matrix'                             
                         else:
                             weakform.type = 'vector'                            
                         if coordinate_type == 'planar':                            
                             weakform.expression = ds_matrix.planar                        
                         if coordinate_type == 'axi':    
                             weakform.expression = ds_matrix.axi                                                      
                         weakform.coordinate_type = coordinate_type                                                     
                         weakform.i = ds_matrix.i
                         weakform.j = ds_matrix.j    
                         volume.weakforms.append(weakform)                
                 i = i + 1
             module.volumes.append(volume)
                
         surface = Surface()
         surface.id = ds_module.general.id
         surface.name = ds_weakform.analysistype                  
         for ds_quantity in ds_module.surface.quantity:
            quantity = Quantity()            
            quantity.id = ds_quantity.id
            quantity.name = ds_quantity.id
            quantity.short_name = ds_quantity.shortname
            quantity.type = 'boundary'            
            quantity.unit = ds_quantity.unit
            surface.quantities.append(quantity)            
                    
         for ds_weakform in ds_module.surface.weakforms.weakform:             
             for ds_boundary in ds_weakform.boundary:                 
                 for ds_quantity in ds_boundary.quantity:
                    quantity = Quantity()      
                    quantity.id = ds_quantity.id                    
                    weakform.quantities.append(quantity)                 
                 n_essential = len(ds_boundary.essential)
                 n_vector = n_essential + len(ds_boundary.vector)
                 i = 0                 
                 for ds_vector in ds_boundary.essential + ds_boundary.vector + ds_boundary.matrix:                     
                     for coordinate_type in coordinate_types:                         
                         weakform = WeakForm()
                         if (i < n_essential) :
                             weakform.type = 'essential'
                         if ((i >= n_essential) & (i < n_vector)):                                 
                             weakform.type = 'vector'   
                             weakform.j = ds_vector.j
                         if (i >= n_vector):
                             weakform.type = 'matrix'   
                             weakform.j = ds_vector.j
                         weakform.i = ds_vector.i        
                         if coordinate_type == 'planar':                            
                             weakform.expression = ds_vector.planar
                         if coordinate_type == 'axi':
                             weakform.expression = ds_vector.axi                             
                         weakform.coordinate_type = coordinate_type
                         weakform.analysis_type = ds_weakform.analysistype
                         weakform.integral_type = 'surf'
                         weakform.boundary_type = ds_boundary.id                         
                         weakform.id = surface.id + '_' + weakform.analysis_type + '_' \
                             + weakform.coordinate_type                           
                         surface.weakforms.append(weakform)                         
                     i = i + 1
         module.surfaces.append(surface)            
         self.modules.append(module)
        
        
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
        self.quantities = []
        
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
        if (self.type == 'essential'):
            string = factory_template.getElementsByTagName('condition_exact_solution')[0].childNodes[0].nodeValue                                
        elif (self.type == 'vector'):            
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
        string = string.replace('FunctionName', function_name + '_' + self.analysis_type)
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
            self.boundary_type = self.boundary_type.replace(' ','_')                
            replaced_string = string.replace(name, self.get_class_name() + '_' + self.analysis_type)            
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
        function_types = ['','_value', '_ord', '_clone', '_derivatives']
        cpp_code = ''        
        for function_type in function_types:         
            node = cpp_template.getElementsByTagName('variable_definition')[0]
            variable_def_temp = node.childNodes[0].nodeValue                                            
            for node in cpp_template.getElementsByTagName(self.get_function_name() + function_type):                        
                string = node.childNodes[0].nodeValue                                                                                                                                                                                                  
                name = self.get_temp_class_name()             
                replaced_string = string.replace(name, self.get_class_name()  + '_' + self.analysis_type)
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
                    expression = self.parse_expression(self.expression, True, '')                     
                else:
                    expression = self.parse_expression(self.expression, False, '')                                                                                                                                                                  
                if self.expression =='':               
                    replaced_string = ''
                else:                    
                    replaced_string = replaced_string.replace('//expression', 
                                expression) + '\n\n'                
                cpp_code += replaced_string                                                                                                                           
        return cpp_code            
            
    def parse_expression(self, expression, without_variables, output):                        
        replaces = { 'PI': 'M_PI',
                     'f': 'Util::scene()->problemInfo()->frequency',                     
                     'x': 'e->x[i]',
                     'y': 'e->y[i]',
                     'r': 'e->x[i]',
                     'z': 'e->y[i]',
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
        
        
        latex_replaces = { '*': '\\cdot ',
                     'PI': '\\pi',
                     'EPS0': '\\varepsilon_0',
                     'epsr': '\\varepsilon_r',
                     'f': 'f',                     
                     'udx': '\\frac{\\partial u^{l}}{\\partial x}',
                     'udy': '\\frac{\\partial u^{l}}{\\partial y}',           
                     'udr': '\\frac{\\partial u^{l}}{\\partial r}',
                     'udz': '\\frac{\\partial u^{l}}{\\partial z}',
                     'vdx': '\\frac{\\partial v^{l}}{\\partial x}',
                     'vdy': '\\frac{\\partial v^{l}}{\\partial y}',           
                     'vdr': '\\frac{\\partial v^{l}}{\\partial r}',
                     'vdz': '\\frac{\\partial v^{l}}{\\partial z}',           
                     'uval': 'u',
                     'vval': 'v',
                     'upval': 'u^{l-1}',
                     'updx': '\\frac{\\partial u^{l-1}}{\\partial x}',
                     'updy': '\\frac{\\partial u^{l-1}}{\\partial y}',
                     'updr': '\\frac{\\partial u^{l-1}}{\\partial r}',
                     'updz': '\\frac{\\partial u^{l-1}}{\\partial z}',
                     'deltat': '\\delta t'                       
                     }            

                
        symbols = ['x', 'y', 'r', 'z', 'f', 'udr', 'udz', 'udx', 'udy',
                   'vdr', 'vdz', 'vdx', 'vdy', 'updr', 'updx', 'updy', 'updz',
                   'uval', 'vval', 'upval', 'deltat', 'uptval', 'PI']
                           
        variables = []
        variables_derivatives = []
        for variable in self.variables:        
            symbols.append(variable.short_name)
            symbols.append("d" + variable.short_name)
            variables.append(variable.short_name)
            variables_derivatives.append("d" + variable.short_name)
             
        for const in self.constants:
            symbols.append(const.id)        
        if output == 'latex':            
            if not(expression.replace(' ','') == ''):
                parser = NumericStringParser(symbols, latex_replaces, variables, 
                                       variables_derivatives, without_variables)                        
                expression_list = parser.parse(expression).asList()                                  
                parsed_exp = parser.translate_to_latex(expression_list)
            else:
                parsed_exp =''                             
        else:
            parser = NumericStringParser(symbols, replaces, variables, 
                                         variables_derivatives, without_variables)                        
            if not(expression.replace(' ','') == ''):
                expression_list = parser.parse(expression).asList()                                  
                parsed_exp = parser.translate_to_cpp(expression_list)                             
            else:
                parsed_exp =''
            parsed_exp = '(' + parsed_exp + ');'                                          
        return parsed_exp

class Volume:
    def __init__(self):
        self.id = ''        
        self.name = ''
        self.type = ''                
        self.weakforms = []        

class Surface:
    def __init__(self):
        self.id = ''
        self.name = ''
        self.type = ''        
        self.weakforms = []
        self.quantities = []
#                        
class Quantity:
    def __init__(self):
        self.id = ''
        self.type = ''        
        self.name = ''
        self.short_name = ''
        self.units = ''  
        
    def write_cpp_code(self):
        pass
#                
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
#        
class Module:
    def __init__(self):
        self.id = ''
        self.name = ''
        self.description = ''        
        self.volumes = [] 
        self.surfaces = []
        self.constants = []
        self.quantities = []
        
        
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
        for volume in self.volumes:
            print '\n--------------------------------'        
            print volume.name            
            print '--------------------------------'            
            print '\nmatrix forms:'            
            for variable in self.quantities:
                print variable.name
            for weakform in volume.weakforms:
                print (weakform.type, weakform.coordinate_type, 
                      weakform.integral_type)
                print weakform.i, weakform.j, weakform.expression

        print 'Boundaries'                              
        for surface in self.surfaces:                                          
            print 'Quantities'            
            for quantity in surface.quantities:
                print quantity.name
            print 'Weakforms'        
            for weakform in surface.weakforms:
                print (weakform.id, weakform.i, weakform.j, weakform.type, weakform.boundary_type, 
                       weakform.coordinate_type, weakform.expression, weakform.integral_type)                                                                   
                                        
    def underline(self, string, character):
        under_string = string + '\n' + character*len(string) + '\n'
        return under_string                           
    
    def gen_doc(self):                
        doc_string = self.underline(self.name, '*')
        doc_string += self.underline('Description ', '=')\
                      + self.description + '\n\n'
        i = 0                                
        doc_string +=  self.underline('Constants', '=')        
        for constant in self.constants:
            doc_string += str(constant.id) + '  ' + str(constant.value) + '\n'
            i += 1                 
            doc_string += '\n' + self.underline('Analysis', '=') + '\n'        
        for material in self.materials:
            doc_string +='\n' + self.underline(material.name.capitalize(),'-') + '\n'            
            doc_string += '\n' + self.underline('Domain weak forms:', '^') + '\n'            
#            for variable in self.variables:
#                doc_string += variable.name + '\n'
            
            for weakform in material.weakforms:
                doc_string += weakform.type + weakform.coordinate_type 
                doc_string += weakform.integral_type + '\n\n'
                doc_string += '.. math:: \n\n'                         
                doc_string += '    ' \
                              + weakform.parse_expression(weakform.expression, False,'latex') + '\n\n'                       
#        
        doc_string += '\n' + self.underline('Boundary conditions:', '-') + '\n'
        for boundary in self.boundaries:                                          
            doc_string += '\n' + self.underline('Variables:', '^') + '\n'
            for variable in boundary.variables:
                doc_string +=variable.short_name + '  ' + variable.name + '  [' + \
                              variable.unit + '] \n\n'                  
            doc_string += '\n' + self.underline('Weakforms', '^') + '\n'        
            for weakform in boundary.weakforms:
                doc_string += '.. math:: \n\n'                
                doc_string += '    ' + weakform.parse_expression(weakform.expression, False,'latex') + '\n\n'
        return doc_string
                                        
    def extract_modules(self):
        module_types = []
        part_modules = []
        part_module = PartModule()          
        
        for volume in self.volumes:  
            for weakform in volume.weakforms:                             
                part_module_id = self.id + '_' + volume.name + '_' \
                    + weakform.coordinate_type + '_' + weakform.analysis_type                
                if (part_module_id in module_types):                    
                    index = module_types.index(part_module_id)                                          
                    part_module = part_modules[index]
                                         
                else:
                    module_types.append(part_module_id)                    
                    part_module = PartModule()                    
                    part_module.name = self.name
                    part_module.id = self.id + '_' + volume.name + '_' \
                    + weakform.coordinate_type
                    part_module.description = self.description
                    part_module.coordinate_type = weakform.coordinate_type
                    part_module.constants = self.constants                    
                    part_module.volumes = self.volumes
                    part_module.analysis = volume.name                    
                    part_modules.append(part_module)                                
                weakform.variables = self.quantities
                weakform.constants = self.constants
                weakform.id = part_module.id                
                part_module.weakforms.append(weakform)                        
       
        for surface in self.surfaces:                                    
                for weakform in surface.weakforms:                                         
                    if (weakform.id + '_' + weakform.analysis_type in module_types):                                                                
                        if weakform.integral_type == 'surf':                            
                            index = module_types.index(weakform.id + '_' + weakform.analysis_type)                    
                            part_module = part_modules[index] 
                            weakform.variables = surface.quantities
                            weakform.constants = self.constants                                        
                            part_module.weakforms.append(weakform)               
        
        print len(part_modules)
        
        for part_module in part_modules:
            print "--------------------------"                            
            print part_module.id
            print "--------------------------"                
            for weakform in part_module.weakforms:
                print weakform.id, weakform.type, weakform.integral_type
            
        return part_modules;
        
                                                                                          
    
    def get_code(self, param_templates):
        templates = dict() 
        templates['.cpp'] = param_templates['template_weakform_cpp.xml']
        templates['.h'] = param_templates['template_weakform_h.xml']
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
                        class_names.add(weakform.get_class_name() + '_' + weakform.analysis_type)
                        file_strings[file_string_name] += weakform.get_cpp_code(templates[key])            
                        factory_code =  weakform.get_factory_code(param_templates['template_weakform_factory_h.xml'])
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
                    weakform_pri_file.write('SOURCES += ' + Config.weakform_dir + filename + '\n')            
           
            weakform_pri_file.write('HEADERS += ' + Config.weakform_dir + 'weakform_factory.h' + '\n')
            weakform_pri_file.close()

if __name__ == '__main__':    
    #parser = XmlParser(['heat', 'electrostatic', 'magnetic', 'current', 'acoustic', 'elasticity', 'rf_te'])        
    parser = XmlParser(['heat'])    
    parser.process()
