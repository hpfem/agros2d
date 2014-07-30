from variant import ModelDict
from variant import ModelBase

_md = None

def _md_models(problem_dir):        
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