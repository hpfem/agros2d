def _md_problems(problem_dir):
    from variant import ModelDict
    from variant import ModelBase
    
    md = ModelDict()
    md.directory = problem_dir
    md.load(ModelBase)
    
    lst = []
    for k, m in sorted(md._models.items()):
        lst.append({ 'key' : k, 'solved' : m.solved })
                
    return lst  