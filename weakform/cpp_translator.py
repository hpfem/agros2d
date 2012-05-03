# -*- coding: utf-8 -*-
"""
Created on Thu Apr 19 13:46:14 2012

@author: david
"""

from xml.dom import minidom
from expression_parser import NumericStringParser
           
class File:
    
    def __init__(self, file_name, file_name_type, file_suffix, template):
        self.file_name = file_name
        self.file_name_type = file_name_type        
        self.file_suffix = file_suffix        
        self.template = template
        self.structure = []
        self.variables = []
        self.templates = set()
        
    def read_template(self, item, template):                
        node = template.getElementsByTagName(item[0])[0]                    
        
        self.templates.add(node.childNodes[0].nodeValue)        
        for subitem in item[3]:            
            self.read_template(subitem, template)
            
        
    def read_templates(self):
        if self.template != '':        
            fread = open(self.template, 'r')
            template = minidom.parse(fread).documentElement        
            fread.close()      
     
        for item in self.structure:                        
            self.read_template(item, template)            
        
        for template in self.templates:
            print template
             

class CppTranslator:   
    
    replaces = {    'PI': 'M_PI',
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
                     'deltat': 'Util::problem()->config()->timeStep().number()'}            
 
    def __init__(self):
        self.template_dir = './templates/'        
        weakform_cpp_file = File('weakform', 'repeated', 'cpp', self.template_dir + 'template_weakform_cpp.xml')        
        weakform_cpp_file.structure = [['head', 'unique',[],[]], 
                                       ['includes', 'unique',[],[]], 
                                       ['namespaces', 'unique',[],[]],                                        
                                       ['custom_matrix_form_vol', 'repeated', ['variable_definition'], []],
                                       ['custom_matrix_form_vol_value', 'repeated', ['expression'], []],
                                       ['custom_matrix_form_vol_ord', 'repeated', ['expression'], []],
                                       ['custom_matrix_form_vol_clone', 'repeated', ['expression'], []], 
                                       ['custom_vector_form_vol', 'repeated', ['variable_definition'], []],
                                       ['custom_vector_form_vol_value', 'repeated', ['expression'], []],
                                       ['custom_vector_form_vol_ord', 'repeated', ['expression'], []],
                                       ['custom_vector_form_vol_clone', 'repeated', ['expression'], []], 
                                       ['custom_matrix_form_surf', 'repeated', ['variable_definition'], []],
                                       ['custom_matrix_form_surf_value', 'repeated', ['expression'], []],
                                       ['custom_matrix_form_surf_ord', 'repeated', ['expression'], []],
                                       ['custom_matrix_form_surf_clone', 'repeated', ['expression'], []], 
                                       ['custom_vector_form_surf', 'repeated', ['variable_definition'], []],
                                       ['custom_vector_form_surf_value', 'repeated', ['expression'], []],
                                       ['custom_vector_form_surf_ord', 'repeated', ['expression'], []],
                                       ['custom_vector_form_surf_clone', 'repeated', ['expression'], []], 
                                       ['footer', 'unique', [], []]]
        weakform_cpp_file.variables = ['expression', 'class_name']
        
        weakform_h_file = File('weakform', 'repeated', 'h', self.template_dir + 'template_weakform_h.xml')
        weakform_h_file.structure = [['head', 'unique',[], []], 
                                       ['includes', 'unique',[], []], 
                                       ['namespaces', 'unique',[], []], 
                                       ['custom_matrix_form_vol', 'repeated', ['variable_declaration'],  []],
                                       ['custom_vector_form_vol', 'repeated', ['variable_declaration'],  []],
                                       ['custom_matrix_form_surf', 'repeated', ['variable_declaration'], []],
                                       ['custom_vector_form_surf', 'repeated', ['variable_declaration'], []],
                                       ['footer', 'unique', [], []]]                        
        weakform_h_file.variables = [['variable_declaration', 'template']]
                        
        weakform_factory_h_file = File('weakform_factory', 'unique', 'h', self.template_dir + 'template_weakform_factory_h.xml')        
        weakform_factory_h_file.variables = [['class_name', 'row_index', 'column_index', 'boundary_type']]                             
        weakform_factory_h_file.structure = [['head', 'unique',[], []], 
                                       ['includes', 'unique',[], []],                                       
                                       ['CustomEssentialFormSurf', 'unique', [], 
                                            [['condition_exact_solution', 'repeated',['class_name', 'row_index'], []]]],
                                       ['CustomMatrixFormVol', 'unique', [], 
                                            [['condition_matrix_vol', 'repeated', ['class_name', 'row_index', 'column_index'], []]]],
                                       ['CustomVectorFormVol', 'unique', [], 
                                            [['condition_vector_vol', 'repeated', ['class_name', 'row_index', 'column_index'],[]]]],
                                       ['CustomMatrixFormSurf', 'unique', [], 
                                            [['condition_matrix_surf', 'repeated', ['class_name', 'row_index', 'column_index', 'boundary_type'],[]]]],
                                       ['CustomVectorFormSurf', 'unique', [], 
                                            [['condition_vector_surf', 'repeated', ['class_name', 'row_index', 'column_index', 'boundary_type'],[]]]],                                       
                                       ['footer', 'unique',[], []]]       
                        
        project_file = File('weakform', 'unique', 'pri', self.template_dir + 'template_weakform_pri.xml')
                
        project_file.structure = [['add_sources', 'repeated', ['source_filename'], []],
                                  ['add_headers', 'repeated', ['header_filename'], []],
                                  ['add_weakform_factory', 'unique', [], []]]  

        
        
        self.templates = [weakform_cpp_file, weakform_h_file, weakform_factory_h_file, project_file]
        
        for template in self.templates:
            template.read_templates()
 
if __name__ == '__main__': 
    cpp_translator = CppTranslator();

    