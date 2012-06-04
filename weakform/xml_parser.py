#!/usr/bin/python
import module_xml as md
from copy import *
import couplings as cp
from xml.dom import minidom
import os
from expression_parser import NumericStringParser

class Config:     
    modules_dir = '../resources/modules/'
    couplings_dir = '../resources/couplings/'
    templates_dir = './templates/'
    weakform_dir = './src/'        
    factory_dir = './src/' 
    doc_dir = '../resources_source/doc/source/modules/'
   
    
    templates = ['template_weakform_cpp.xml', 'template_weakform_h.xml', 'template_weakform_factory_h.xml']
    project_file = 'weakform.pri'
  
    
class XmlParser:    
    def __init__(self, modules, couplings):               
        self.module_files = [] 
        self.coupling_files = []
        for module_name in modules:
            module_file = module_name + '.xml'             
            self.module_files.append(module_file)                  
            
        for coupling_name in couplings:
            coupling_file = coupling_name + '.xml'             
            self.coupling_files.append(coupling_file)
                                               
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
                                            
        for coupling_file in self.coupling_files:
            file_name =  coupling_file.split('.')[0] # remove suffix
            coupled_module_names = file_name.split('-')  #separates coupled modules                      
            
            coupled_constants = []            

            quantity_ids_planar = []
            quantity_ids_axi = []
            coupled_quantities_planar = []
            coupled_quantities_axi = [] 
           
            for module in self.modules:                 
                for module_name in coupled_module_names:                            
                    if(module.id == module_name):                                            
                        coupled_constants.extend(module.constants)                        
                        for volume in module.volumes:                            
                            for quantity in volume.quantities_planar:                                
                                if not(quantity.id in quantity_ids_planar):
                                    tmp_quantity = Quantity()
                                    tmp_quantity = copy(quantity)
                                    if module_name == coupled_module_names[0]:
                                        tmp_quantity.field  = 'Target'
                                    coupled_quantities_planar.append(tmp_quantity)                                    
                                    quantity_ids_planar.append(quantity.id)
                                    #print quantity.id                                
                                                                                                    
                            for quantity in volume.quantities_axi:                                
                                if not(quantity.id in quantity_ids_axi):
                                    tmp_quantity = Quantity()
                                    tmp_quantity = copy(quantity)
                                    if module_name == coupled_module_names[0]:
                                        tmp_quantity.field  = 'Target'
                                    coupled_quantities_axi.append(tmp_quantity)
                                    quantity_ids_axi.append(quantity.id)
                                    
                    

            #for quantity in coupled_quantities_planar:
                # print quantity.id
            # print "----------------------------------"                
            
            self.parse_xml_coupling(coupling_file)
            for module in self.modules:
                if module.id == (file_name).replace('-','_'):
                    for volume in module.volumes:
                        volume.quantities_planar = coupled_quantities_planar
                        volume.quantities_axi = coupled_quantities_axi
                        volume.constants = coupled_constants
            
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
        solver_types = ['linear', 'newton']
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
        quantities = []
        for ds_quantity in ds_module.volume.quantity:            
            quantity = Quantity()            
            quantity.id = ds_quantity.id
            quantity.name = ds_quantity.id
            quantity.short_name = ds_quantity.shortname
            quantity.type = 'material'            
            quantity.unit = ds_quantity.unit
            quantities.append(quantity)

        # modules        
        for ds_weakform in ds_module.volume.weakforms_volume.weakform_volume:
            volume = Volume()                 
            volume.name =  ds_weakform.analysistype
            for ds_quantity in ds_weakform.quantity:
                for module_quantity in quantities:
                    for coordinate_type in coordinate_types:
                        quantity = Quantity()
                        quantity.id = ds_quantity.id
                        quantity.short_name = ds_quantity.shortname
                        quantity.unit = ds_quantity.unit                

                        if (module_quantity.id == quantity.id):
                            quantity.name = module_quantity.name
                            quantity.short_name = module_quantity.short_name
                            quantity.type = 'material'
                            quantity.unit = module_quantity.unit
                            if coordinate_type == 'planar':                            
                                if (ds_quantity.nonlinearity_planar != None) and (ds_quantity.nonlinearity_planar != ''):
                                    quantity.expression = ds_quantity.nonlinearity_planar                                                                                                    
                                else:
                                    quantity.expression = None     
                                if not (quantity in volume.quantities_planar):    
                                    volume.quantities_planar.append(quantity)                                                                         
                            
                            if coordinate_type == 'axi':                                
                                if (ds_quantity.nonlinearity_axi != None) and (ds_quantity.nonlinearity_axi != ''):
                                    quantity.expression = ds_quantity.nonlinearity_axi                                    
                                else:
                                    quantity.expression = None                            
                                if not (quantity in volume.quantities_axi):
                                    volume.quantities_axi.append(quantity)                                    
                                             
            i  = 0
            n  = len(ds_weakform.matrix_form)
            for ds_matrix in ds_weakform.matrix_form + ds_weakform.vector_form:                 
                for coordinate_type in coordinate_types:   
                    for solver_type in solver_types:                                       
                        weakform = WeakForm()                            
                        weakform.integral_type = 'vol'                     
                        weakform.analysis_type = ds_weakform.analysistype                         
                        if (i < n):    
                            weakform.type = 'matrix'                             
                        else:
                            weakform.type = 'vector'                            
                        if coordinate_type == 'planar':
                            if (solver_type == 'linear'):                                                            
                                weakform.expression = ds_matrix.planar_linear
                            if (solver_type == 'newton'):
                                weakform.expression = ds_matrix.planar_newton
                        if coordinate_type == 'axi':
                            if (solver_type == 'linear'):                                        
                                weakform.expression = ds_matrix.axi_linear
                            if (solver_type == 'newton'):    
                                weakform.expression = ds_matrix.axi_newton
                        weakform.solver_type = solver_type
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
                    
        for ds_weakform in ds_module.surface.weakforms_surface.weakform_surface:             
            for ds_boundary in ds_weakform.boundary:                 
                for ds_quantity in ds_boundary.quantity:
                    quantity = Quantity()      
                    quantity.id = ds_quantity.id                    
                    weakform.quantities.append(quantity)                 
                n_essential = len(ds_boundary.essential_form)
                n_vector = n_essential + len(ds_boundary.vector_form)
                i = 0                 
                # print len(ds_boundary.matrix_form)
                for ds_vector in ds_boundary.essential_form + ds_boundary.vector_form + ds_boundary.matrix_form:                     
                    for coordinate_type in coordinate_types:
                        for solver_type in solver_types:                         
                            weakform = WeakForm()
                            if (i < n_essential):
                                weakform.type = 'essential'
                            if ((i >= n_essential) & (i < n_vector)):                                 
                                weakform.type = 'vector'   
                                weakform.j = ds_vector.j
                            if (i >= n_vector):
                                weakform.type = 'matrix'   
                                weakform.j = ds_vector.j
                            weakform.i = ds_vector.i 
                            # print weakform.type
                            if coordinate_type == 'planar':
                                if (solver_type == 'linear'):                                                            
                                    weakform.expression = ds_vector.planar_linear
                                if (solver_type == 'newton'):
                                    weakform.expression = ds_vector.planar_newton
                            if coordinate_type == 'axi':
                                if (solver_type == 'linear'):                                        
                                    weakform.expression = ds_vector.axi_linear
                                if (solver_type == 'newton'):    
                                    weakform.expression = ds_vector.axi_newton
                            weakform.solver_type = solver_type                        
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
        
    def parse_xml_coupling(self, filename):
        coordinate_types = ['planar', 'axi']
        solver_types = ['linear', 'newton']        
        module = Module()
        ds_coupling = cp.parse(Config.couplings_dir + filename)
        module.description = ds_coupling.general.description                
        module.id = (ds_coupling.general.id).replace('-','_')            
        module.source = ds_coupling.general.modules.source
        module.target = ds_coupling.general.modules.target

        # couplings         
        for ds_weakform in ds_coupling.volume.weakforms_volume.weakform_volume:            
            volume = Volume()
            volume.name = ds_weakform.sourceanalysis + '_' + ds_weakform.targetanalysis                    
            for coordinate_type in coordinate_types:
                for solver_type in solver_types:
                    for ds_matrix in ds_weakform.matrix_form:
                        weakform = WeakForm()
                        weakform.coupling_type = ds_weakform.couplingtype
                        weakform.source_analysis = ds_weakform.sourceanalysis
                        weakform.target_analysis = ds_weakform.targetanalysis
                        weakform.analysis_type = ds_weakform.sourceanalysis + '_' + ds_weakform.targetanalysis                      
                        weakform.type = 'matrix'
                        weakform.coordinate_type = coordinate_type
                        if coordinate_type == 'planar':
                            if (solver_type == 'linear'):                                                            
                                weakform.expression = ds_matrix.planar_linear
                            if (solver_type == 'newton'):
                                weakform.expression = ds_matrix.planar_newton
                        if coordinate_type == 'axi':
                            if (solver_type == 'linear'):                                        
                                weakform.expression = ds_matrix.axi_linear
                            if (solver_type == 'newton'):    
                                weakform.expression = ds_matrix.axi_newton
                        weakform.solver_type = solver_type
                        weakform.i = ds_matrix.i
                        weakform.j = ds_matrix.j                    
                        weakform.integral_type = 'vol'
                        volume.weakforms.append(weakform)
                    
                for ds_vector in ds_weakform.vector_form:
                    for solver_type in solver_types:
                        weakform = WeakForm()
                        weakform.coordinate_type = coordinate_type
                        weakform.coupling_type = ds_weakform.couplingtype
                        weakform.source_analysis = ds_weakform.sourceanalysis
                        weakform.source_analysis = ds_weakform.sourceanalysis
                        weakform.analysis_type = ds_weakform.sourceanalysis + '_' + ds_weakform.targetanalysis
                        weakform.integral_type = 'vol'
                        weakform.i = ds_vector.i
                        weakform.j = ds_vector.j                    
                        weakform.type = 'vector'
                        weakform.coordinate_type = coordinate_type
                        if coordinate_type == 'planar':
                            if (solver_type == 'linear'):                                                            
                                weakform.expression = ds_vector.planar_linear
                            if (solver_type == 'newton'):
                                weakform.expression = ds_vector.planar_newton
                        if coordinate_type == 'axi':
                            if (solver_type == 'linear'):                                        
                                weakform.expression = ds_vector.axi_linear
                            if (solver_type == 'newton'):    
                                weakform.expression = ds_vector.axi_newton
                        weakform.solver_type = solver_type
                        volume.weakforms.append(weakform)                                        

            module.volumes.append(volume)    
        self.modules.append(module)
        
        
class WeakForm:
    def __init__(self):        
        self.id = ''
        self.type = ''
        self.integral_type = ''        
        self.coupling_type = ''
        self.source_analysis = ''
        self.target_analysis = ''
        self.coordinate_type = ''
        self.analysis_type = ''
        self.boundary_type = ''        
        self.field = ''
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
            + '_'  + str(self.j) + '_' + self.analysis_type  + '_' + self.solver_type + '_' + self.coupling_type               
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
        
                     
        string = string.replace('class_name', self.id + '_' + self.solver_type + '_' + self.coupling_type)
        string = string.replace('axi', 'axisymmetric')                        
        if (self.type != 'essential'):
            string = string.replace('row_index', str(self.i))                        
            string = string.replace('column_index', str(self.j))
        else:
            string = string.replace('row_index', str(self.i))                        
            string = string.replace('column_index', str(self.j))

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
            self.boundary_type = self.boundary_type.replace(' ','_')                
            replaced_string = string.replace(name, self.get_class_name())                        
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
                replaced_string = string.replace(name, self.get_class_name())
                if function_type == '':
                    variable_defs = '' ;                    
                    for variable in self.variables:                    
                        variable_string = variable_def_temp.replace('variable_short', 
                                        variable.short_name)                    
                        variable_string = variable_string.replace('variable', 
                                        variable.id)
                        variable_string = variable_string.replace('Source', variable.field)
                        variable_defs += variable_string
                        variable_defs = variable_defs.replace('material', variable.type)                        
                                                                                                                                        
                    replaced_string = replaced_string.replace('//variable_definition', 
                                                              str(variable_defs))     
                if function_type == '_ord':
                    expression = self.parse_expression(self.expression, True, '')                     
                else:                    
                    expression = self.parse_expression(self.expression, False, '')                                   
                if self.expression == '':               
                    replaced_string = ''
                else:                    
                    replaced_string = replaced_string.replace('//expression', 
                                expression) + '\n\n'                
                cpp_code += replaced_string                                                                                                                           
        return cpp_code            
            
    def parse_expression(self, expression, without_variables, output):                        
        replaces = { 'PI': 'M_PI',
                     'f': 'Util::problem()->config()->frequency()',
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
                     'deltat': 'Util::problem()->config()->timeStep().number()',
                     'value1': 'u_ext[0]->val[i]',
                     'value2': 'u_ext[1]->val[i]',
                     'source0': 'ext->fn[0]->val[i]',
                     'source1': 'ext->fn[1]->val[i]',
                     'source0dx': 'ext->fn[0]->dx[i]',
                     'source0dy': 'ext->fn[0]->dy[i]',
                     'source0dr': 'ext->fn[0]->dx[i]',
                     'source0dz': 'ext->fn[0]->dy[i]',
                     'dx1': 'u_ext[0]->dx[i]',
                     'dx2': 'u_ext[1]->dx[i]',
                     'dy1': 'u_ext[0]->dy[i]',
                     'dy2': 'u_ext[1]->dy[i]',
                     'dr1': 'u_ext[0]->dx[i]',
                     'dr2': 'u_ext[1]->dx[i]',
                     'dz1': 'u_ext[0]->dy[i]',
                     'dz2': 'u_ext[1]->dy[i]'
                     }
        
        postprocessor_replaces = {
                     'x': 'e->x[i]',
                     'y': 'e->y[i]',
                     'r': 'e->x[i]',
                     'z': 'e->y[i]',
                     'value1': '(u_ext[0] ? u_ext[0]->val[i] : 0)',
                     'value2': '(u_ext[1] ? u_ext[1]->val[i] : 0)',
                     'dx1': '(u_ext[0] ? u_ext[0]->dx[i] : 0)',
                     'dx2': '(u_ext[1] ? u_ext[1]->dx[i] : 0)',
                     'dy1': '(u_ext[0] ? u_ext[0]->dy[i] : 0)',
                     'dy2': '(u_ext[1] ? u_ext[1]->dy[i] : 0)',
                     'dr1': '(u_ext[0] ? u_ext[0]->dx[i] : 0)',
                     'dr2': '(u_ext[1] ? u_ext[1]->dx[i] : 0)',
                     'dz1': '(u_ext[0] ? u_ext[0]->dy[i] : 0)',
                     'dz2': '(u_ext[1] ? u_ext[1]->dy[i] : 0)'                                  
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
                   'uval', 'vval', 'upval', 'deltat', 'uptval', 'PI',
                   'value1', 'value2', 'dx1', 'dx2', 'dy1', 'dy2', 'dr1', 'dr2', 'dz1', 'dz2', 'source0',
                   'source1', 'source0dx', 'source0dy', 'source0dr', 'source0dz']
                           
        variables = []
        variables_derivatives = []
        parser = NumericStringParser(symbols, postprocessor_replaces, variables, 
                                     variables_derivatives, without_variables)                        

        for variable in self.variables:        
            symbols.append(variable.short_name)
            symbols.append("d" + variable.short_name)                        
            variables_derivatives.append("d" + variable.short_name)            
            parsed_exp = None            
            if variable.expression != None:                                
                expression_list = parser.parse(variable.expression).asList()                
                parsed_exp =  '(' + parser.translate_to_cpp(expression_list) + ')'
                #variable.expression =  parsed_exp                                                         
            variables.append([variable.short_name, parsed_exp])
            
            
            
        parsed_exp = ''
        
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
        self.quantities_planar = []
        self.quantities_axi = []
        
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
        self.expression = None
        self.field = 'Source'
        
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
        self.target = ''
        self.source = ''
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
                
                if weakform.coordinate_type == 'axi':
                    weakform.variables = volume.quantities_axi                                                    
                
                if weakform.coordinate_type == 'planar':
                    weakform.variables = volume.quantities_planar
                                                                         
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
                        class_names.add(weakform.get_class_name())
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
                suffix = filename.split('.')[-1]
                if suffix == 'cpp':                
                    weakform_pri_file.write('SOURCES += ' + Config.weakform_dir + filename + '\n')
                if suffix == 'h':            
                    weakform_pri_file.write('HEADERS += ' + Config.weakform_dir + filename + '\n')
                    
            weakform_pri_file.write('HEADERS += ' + Config.weakform_dir + 'weakform_factory.h' + '\n')
            weakform_pri_file.close()

if __name__ == '__main__':
    # coupling_parser = XmlParser(['acoustic', 'current', 'elasticity', 'electrostatic', 'heat', 'magnetic', 'rf'], ['current-heat', 'heat-elasticity', 'magnetic-heat'])
    coupling_parser = XmlParser(['heat', 'acoustic'], [])
    coupling_parser.process()
