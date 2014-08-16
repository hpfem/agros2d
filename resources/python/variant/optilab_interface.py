from variant import ModelDict
from variant import ModelBase

_md = None

def _md_models(problem_dir):  
    print(problem_dir)      
    global _md
    
    _md = ModelDict()
    _md.directory = problem_dir
    _md.load(ModelBase)
    
    lst = []
    for k, m in sorted(_md.dict.items()):
        lst.append({ 'key' : k, 'solved' : m.solved })
                
    return lst  
    
def _md_model(key):
    global _md
    
    return _md.dict[key]
    
def _open_in_agros2d(file_name):    
    import os.path    
    import sys; 
    sys.path.insert(0, os.path.abspath(os.path.join(file_name, os.pardir, os.pardir)))

    import problem
    p = problem.Model()
    p.load(file_name)
    p.create()
    
    return p
    
def _solve_in_agros2d(file_name):
    p = _open_in_agros2d(file_name)
    p.solve()
    p.save(file_name)