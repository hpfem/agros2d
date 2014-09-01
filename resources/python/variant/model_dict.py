import os
import subprocess
import re

import pythonlab

from variant.model import ModelBase
from collections import OrderedDict

class ModelDict:
    """General class for management of models set."""

    def __init__(self, model_class = ModelBase, models = None):
        """Initialization of model dictionary.
        
        ModelDict(models=None, directory=None)
        
        Keyword arguments:
        models -- list or dictionary in style {name : model} placed models (default is None)
        directory -- current working directory (default is None)
        """

        self._dict = OrderedDict()
        self._directory = None
        self.model_class = model_class

        if (models and isinstance(models, dict)):
            for name, model in models.items():
                self.add_model(model, name)

        if (models and isinstance(models, list)):
            for model in models:
                self.add_model(model)
            
    @property
    def dict(self):
        """Return dictionary of placed models."""
        return self._dict

    @property
    def directory(self):
        """Current working directory."""

        if (not self._directory):
            raise RuntimeError('Temporary directory is not defined.')

        return self._directory

    @directory.setter
    def directory(self, value):
        if not os.path.isdir(value):
            try:
                os.makedirs(value)
            except OSError:
                raise IOError('Directory "{0}" can not be created.'.format(value))
    
        self._directory = os.path.abspath(value)

    def models(self):
        """Return list of models."""
        return list(self._dict.values())

    def solved_models(self):
        """Return list of solved models."""
        models = []
        for model in list(self._dict.values()):
            if model.solved: models.append(model)
        return models

    def model_file_name(self, name):
        if (not self._directory):
            raise RuntimeError('Temporary directory is not defined.')
            
        return '{0}/{1}.pickle'.format(self._directory, name)

    """
    def find_last_model_index(self):
        files = self.find_files('{0}/model_.*.pickle'.format(self._directory))
        for file_name in files:
            name, extension = os.path.basename(file_name).split(".")
            index = int(name.split("_")[1])
            if (index >= self._name_index):
                self._model_index = index
    """

    def add_model(self, model, name=None):
        """Add new model to dictionary.

        add_model(model, name=None, resume=False)

        Keyword arguments:
        model -- model object inherited from ModelBase class
        name -- name of model used as dictionary key and file name (default means automatic model name)
        resume -- resume in model index counting for model name (default is False)
        """

        if name in self._dict.keys():
            raise KeyError('Model with key "{0}" already exist.'.format(name))

        # if (resume and not name):
        #     self._model_index = self.find_last_model_index() + 1

        if not name:
            if hasattr(self, '_model_index'):
                self._model_index += 1
            else:
                self._model_index = 0

            name = 'model_{0:06d}'.format(self._model_index)

        self._dict[name] = model

    def delete_model(self, name):
        """Delete existing model in dictionary.

        delete_model(name)

        Keyword arguments:
        name -- name of model used as dictionary key
        """

        if name in self._dict.keys():
            del self._dict[name]

    def find_files(self, mask):
        """Find and return list of model files in working directory.

        find_files(mask)

        Keyword arguments:
        mask -- regular expression for directory or files
        """

        files = []
        if os.path.isdir(mask):
            files = os.listdir(mask)
        else:
            files_list = os.listdir(os.path.dirname(mask))
            expression  = r'^{0}$'.format(os.path.basename(mask))
            for file_name in files_list:
                if bool(re.match(expression, file_name)): files.append(file_name)

        return files

    def load(self, directory=None, mask=''):
        """Load models from directory.

        load(model_class, mask='')

        Keyword arguments:
        model_class -- class inherited from ModelBase class
        mask -- regular expression for directory or files
        """

        # set directory
        if directory: self._directory = directory

        if not mask:
            mask = self._directory
        elif not os.path.dirname(mask):
            mask = '{0}/{1}'.format(self._directory, mask)
        elif (os.path.abspath(os.path.dirname(mask)) != self._directory and
              len(self.models())):
            raise RuntimeError('Mask do not match with current working directory.')

        files = self.find_files(mask)
        for file_name in files:
            model = self.model_class()
            model.load('{0}/{1}'.format(self._directory, file_name))
            name, extension = os.path.basename(file_name).split(".")
            self._dict[name] = model

    def save(self, directory=None):
        """Save models to current working directory."""
        
        # set directory
        if directory: self._directory = directory
        
        for name, model in self._dict.items():
            model.save(self.model_file_name(name))

    def solve(self, mask='', recalculate=False, save=False):
        """Solve model in directory.

        solve(mask='', recalculate=False)

        Keyword arguments:
        mask -- regular expression for model keys (in default solve all models)
        recalculate -- recalculate solved models (default is False)
        save -- save solved models to current working directory
        """

        models = {}
        if not mask:
            models = self._dict
        else:
            expression  = r'^{0}$'.format(mask)
            for name, model in self._dict.items():
                if bool(re.match(expression, name)): models[name] = model

        for name, model in models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            model.create()
            model.solve()
            model.process()

            if save: model.save(self.model_file_name(name))

    def find_model_by_parameters(self, parameters):
        """Find and return model in dictionary by parameters.

        find_model_by_parameters(parameters)

        Keyword arguments:
        parameters -- list of model parameters
        """

        for name, model in self._dict.items():
            if (model.parameters == parameters): return model

    def update(self):
        """Update dictionary from models in current working dictionary."""
        for name in list(self._dict.keys()):
            self._dict[name].load(self.model_file_name(name))

    def clear(self):
        """Clear dictionary."""
        self._dict.clear()

    def load_from_zip(self, filename):
        import zipfile as zf
        import json        
                    
        with zf.ZipFile(filename, 'r') as zipfile:
            # read description
            desc = json.loads(zipfile.read('problem.desc').decode())
            
            # read problem
            problem = zipfile.read('problem.py').decode()
            exec(problem)
            
            self.model_class = eval(desc["model"])
                                    
            for fn in sorted(zipfile.namelist()):
                if (fn.endswith('pickle')):   
                    model = self.model_class()
                    model.deserialize(zipfile.read(fn))
                    self._dict[os.path.splitext(fn)[0]] = model          
            
            zipfile.close()
        
    def save_to_zip(self, problem, filename):
        import zipfile as zf
        import json        
                    
        with zf.ZipFile(filename, 'w', zf.ZIP_DEFLATED) as zipfile:           
            # add models
            for name, model in self._dict.items():               
                zipfile.writestr(name + '.pickle', model.serialize())
                
            # add problem
            zipfile.write(problem, arcname='problem.py')
            
            # add description
            desc = dict()
            desc["model"] = self.model_class.__name__
            zipfile.writestr('problem.desc', json.dumps(desc))
            
            zipfile.close()
                  
class ModelDictExternal(ModelDict):
    """Class inherited from ModelDict allows use external solver for models (default solver is agros2d_solver)."""

    def __init__(self):
        ModelDict.__init__(self)
        self.solver = "agros2d_solver"
        self.solver_parameters = ['-l', '-c']
        self._output = []

    @property
    def output(self):
        """Output of external solver."""
        return self._output

    def solve(self, mask='', recalculate=False):
        """Solve model placed in current working directory.

        solve(mask='', recalculate=False)

        Keyword arguments:
        mask -- regular expression for model keys (in default solve all models)
        recalculate -- recalculate solved models (default is False)
        """

        if len(self._dict):
            self.save()

        models = {}
        if not mask:
            models = self._dict
        else:
            expression  = r'^{0}$'.format(mask)
            for name, model in self._dict.items():
                if bool(re.match(expression, name)): models[name] = model

        for name, model in models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            code = "import sys; sys.path.insert(0, '{0}');".format(os.path.dirname(self._directory))
            code += "from problem import {0}; model = {0}();".format(type(model).__name__)
            code += "model.load('{0}/{1}.pickle');".format(self._directory, name)
            code += "model.create(); model.solve(); model.process();"
            code += "model.save('{0}/{1}.pickle')".format(self._directory, name)
            command = ['{0}'.format(self.solver)] + self.solver_parameters + ['{0}'.format(code)]

            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            self._output.append(process.communicate())
            model.load('{0}/{1}.pickle'.format(self._directory, name))

if __name__ == '__main__':
    from variant.test_functions import quadratic_function

    md = ModelDict(quadratic_function.QuadraticFunction)
    for x in range(10):
        model = quadratic_function.QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    md.save(pythonlab.tempname())
    md.clear()
    md.load()
    md.solve(save=True)    
    #print(md.dict)
    
    fn = pythonlab.tempname() + ".opt"
    md.save_to_zip(problem = pythonlab.datadir() + "resources/python/variant/test_functions/quadratic_function.py",
                   filename = fn)

    # remove solutions
    import shutil
    shutil.rmtree(md.directory)
    
    mdn = ModelDict()
    mdn.load_from_zip(fn)
    #print(mdn.dict)

    # remove temp opt
    os.remove(fn)