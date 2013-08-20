# import libraries
import pythonlab
import agros2d
from math import *

# interactive help hack
__pydoc_help__ = help
def __a2d_help__(obj = agros2d):
    return __pydoc_help__(obj)

help = __a2d_help__

# test
def test(text, value, normal, error = 0.03):
    if ((normal == 0.0) and (abs(value) < 1e-14)):
        return True
    test = abs((value - normal)/value) < error
    if (not test):    
        print(text + ": Agros2D: " + str(value) + ", correct: " + str(normal) + ", error: " + "{:.4f}".format(abs(value - normal)/value*100) + "%)")
    return test

setattr(agros2d, "test", test)

import unittest as ut

class Agros2DTestCase(ut.TestCase):
    def __init__(self, methodName='runTest'):
        ut.TestCase.__init__(self, methodName)

    def value_test(self, text, value, normal, error = 0.03):
        if ((normal == 0.0) and (abs(value) < 1e-14)):
            self.assertTrue(True)
            return
        test = abs((value - normal)/value) < error
        str = str = "{0}: Agros2D = {1}, correct = {2}, error = {3:.4f} %".format(text, value, normal, abs(value - normal)/value*100)
        self.assertTrue(test, str)

setattr(agros2d, "Agros2DTestCase", Agros2DTestCase)

class Agros2DTestResult(ut.TestResult):
    def __init__(self):
        ut.TestResult.__init__(self)

    def startTest(self, test):
        from time import time
        
        ut.TestResult.startTest(self, test)
        self.time = time()
        # print("{0}".format(test.id().ljust(60, "."))),

    def addSuccess(self, test):
        from time import time
        
        ut.TestResult.addSuccess(self, test)
        self.time -= time()
        print("{0}".format(test.id().ljust(60, "."))),
        print("{0:08.2f}".format(-self.time * 1000).rjust(15, " ") + " ms " +
              "{0}".format("OK".rjust(10, ".")))

    def addError(self, test, err):
        ut.TestResult.addError(self, test, err)
        print("{0}".format(test.id().ljust(60, "."))),
        print("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("ERROR".rjust(10, ".")))        
        print(err[1])

    def addFailure(self, test, err):
        ut.TestResult.addFailure(self, test, err)
        print("{0}".format(test.id().ljust(60, "."))),
        print("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("FAILURE".rjust(10, ".")))        
        print(err[1])       

setattr(agros2d, "Agros2DTestResult", Agros2DTestResult)

def agros2d_material_eval(keys):
    values = []
    for i in range(len(keys)):
        values.append(agros2d_material(keys[i]))

    return values

def memory_chart(last_seconds = 0):
    time, usage = agros2d.memory_usage()

    if (last_seconds == 0):
        pythonlab.chart(time, usage, "t (s)", "Mem (MB)")
    else:
        pythonlab.chart(time[-(last_seconds+1):], usage[-(last_seconds+1):], "t (s)", "Mem (MB)")

setattr(agros2d, "memory_chart", memory_chart)

def vtk_geometry_actor(filename):
    import vtk
    
    reader_poly = vtk.vtkPolyDataReader()
    reader_poly.SetFileName(filename)        
    geometry = reader_poly.GetOutput()

    # create the mapper that corresponds the objects of the vtk file into graphics elements
    geometry_mapper = vtk.vtkDataSetMapper()
    geometry_mapper.SetInput(geometry)
    
    # actor
    geometry_actor = vtk.vtkActor()
    geometry_actor.SetMapper(geometry_mapper)
    geometry_actor.GetProperty().SetColor(0, 0, 0)
    geometry_actor.GetProperty().SetLineWidth(1.8)
    
    return geometry_actor
    
setattr(agros2d, "vtk_geometry_actor", vtk_geometry_actor)                   
    
def vtk_contours_actor(filename, count = 10, color = False):
    import vtk
    
    # read the source file.
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()
    output = reader.GetOutput()
    scalar_range = output.GetScalarRange()

     # contours
    contours = vtk.vtkContourFilter()
    contours.SetInputConnection(reader.GetOutputPort()) 
    contours.GenerateValues(count, scalar_range) 

    # map the contours to graphical primitives 
    contMapper = vtk.vtkPolyDataMapper() 
    contMapper.SetInput(contours.GetOutput()) 
    contMapper.SetScalarVisibility(color) # colored contours
    contMapper.SetScalarRange(scalar_range)
    
    # create an actor for the contours 
    contActor = vtk.vtkActor() 
    contActor.SetMapper(contMapper) 
    contActor.GetProperty().SetColor(0.2, 0.2, 0.2)
    contActor.GetProperty().SetLineWidth(1)
    
    return contActor
     
setattr(agros2d, "vtk_contours_actor", vtk_contours_actor)                   
                                               
def vtk_scalar_actor(filename):
    import vtk
    colobar_agros2d =  [[ 0.125500, 0.162200, 0.960000 ],
                        [ 0.132175, 0.167787, 0.954839 ],
                        [ 0.138849, 0.173373, 0.949678 ],
                        [ 0.145524, 0.178960, 0.944518 ],
                        [ 0.152198, 0.184547, 0.939357 ],
                        [ 0.158873, 0.190133, 0.934196 ],
                        [ 0.165547, 0.195720, 0.929035 ],
                        [ 0.172222, 0.201307, 0.923875 ],
                        [ 0.178896, 0.206893, 0.918714 ],
                        [ 0.185571, 0.212480, 0.913553 ],
                        [ 0.192245, 0.218067, 0.908392 ],
                        [ 0.198920, 0.223653, 0.903231 ],
                        [ 0.205594, 0.229240, 0.898071 ],
                        [ 0.212269, 0.234827, 0.892910 ],
                        [ 0.218943, 0.240413, 0.887749 ],
                        [ 0.225618, 0.246000, 0.882588 ],
                        [ 0.232292, 0.251587, 0.877427 ],
                        [ 0.238967, 0.257173, 0.872267 ],
                        [ 0.245641, 0.262760, 0.867106 ],
                        [ 0.252316, 0.268347, 0.861945 ],
                        [ 0.258990, 0.273933, 0.856784 ],
                        [ 0.265665, 0.279520, 0.851624 ],
                        [ 0.272339, 0.285107, 0.846463 ],
                        [ 0.279014, 0.290693, 0.841302 ],
                        [ 0.285688, 0.296280, 0.836141 ],
                        [ 0.292363, 0.301867, 0.830980 ],
                        [ 0.299037, 0.307453, 0.825820 ],
                        [ 0.305712, 0.313040, 0.820659 ],
                        [ 0.312386, 0.318627, 0.815498 ],
                        [ 0.319061, 0.324213, 0.810337 ],
                        [ 0.325735, 0.329800, 0.805176 ],
                        [ 0.332410, 0.335387, 0.800016 ],
                        [ 0.339084, 0.340973, 0.794855 ],
                        [ 0.345759, 0.346560, 0.789694 ],
                        [ 0.352433, 0.352147, 0.784533 ],
                        [ 0.359108, 0.357733, 0.779373 ],
                        [ 0.365782, 0.363320, 0.774212 ],
                        [ 0.372457, 0.368907, 0.769051 ],
                        [ 0.379131, 0.374493, 0.763890 ],
                        [ 0.385806, 0.380080, 0.758729 ],
                        [ 0.392480, 0.385667, 0.753569 ],
                        [ 0.399155, 0.391253, 0.748408 ],
                        [ 0.405829, 0.396840, 0.743247 ],
                        [ 0.412504, 0.402427, 0.738086 ],
                        [ 0.419178, 0.408013, 0.732925 ],
                        [ 0.425853, 0.413600, 0.727765 ],
                        [ 0.432527, 0.419187, 0.722604 ],
                        [ 0.439202, 0.424773, 0.717443 ],
                        [ 0.445876, 0.430360, 0.712282 ],
                        [ 0.452551, 0.435947, 0.707122 ],
                        [ 0.459225, 0.441533, 0.701961 ],
                        [ 0.465900, 0.447120, 0.696800 ],
                        [ 0.472575, 0.452707, 0.691639 ],
                        [ 0.479249, 0.458293, 0.686478 ],
                        [ 0.485924, 0.463880, 0.681318 ],
                        [ 0.492598, 0.469467, 0.676157 ],
                        [ 0.499273, 0.475053, 0.670996 ],
                        [ 0.505947, 0.480640, 0.665835 ],
                        [ 0.512622, 0.486227, 0.660675 ],
                        [ 0.519296, 0.491813, 0.655514 ],
                        [ 0.525971, 0.497400, 0.650353 ],
                        [ 0.532645, 0.502987, 0.645192 ],
                        [ 0.539320, 0.508573, 0.640031 ],
                        [ 0.545994, 0.514160, 0.634871 ],
                        [ 0.552669, 0.519747, 0.629710 ],
                        [ 0.559343, 0.525333, 0.624549 ],
                        [ 0.566018, 0.530920, 0.619388 ],
                        [ 0.572692, 0.536507, 0.614227 ],
                        [ 0.579367, 0.542093, 0.609067 ],
                        [ 0.586041, 0.547680, 0.603906 ],
                        [ 0.592716, 0.553267, 0.598745 ],
                        [ 0.599390, 0.558853, 0.593584 ],
                        [ 0.606065, 0.564440, 0.588424 ],
                        [ 0.612739, 0.570027, 0.583263 ],
                        [ 0.619414, 0.575613, 0.578102 ],
                        [ 0.626088, 0.581200, 0.572941 ],
                        [ 0.632763, 0.586787, 0.567780 ],
                        [ 0.639437, 0.592373, 0.562620 ],
                        [ 0.646112, 0.597960, 0.557459 ],
                        [ 0.652786, 0.603547, 0.552298 ],
                        [ 0.659461, 0.609133, 0.547137 ],
                        [ 0.666135, 0.614720, 0.541976 ],
                        [ 0.672810, 0.620307, 0.536816 ],
                        [ 0.679484, 0.625893, 0.531655 ],
                        [ 0.686159, 0.631480, 0.526494 ],
                        [ 0.692833, 0.637067, 0.521333 ],
                        [ 0.699508, 0.642653, 0.516173 ],
                        [ 0.706182, 0.648240, 0.511012 ],
                        [ 0.712857, 0.653827, 0.505851 ],
                        [ 0.719531, 0.659413, 0.500690 ],
                        [ 0.726206, 0.665000, 0.495529 ],
                        [ 0.732880, 0.670587, 0.490369 ],
                        [ 0.739555, 0.676173, 0.485208 ],
                        [ 0.746229, 0.681760, 0.480047 ],
                        [ 0.752904, 0.687347, 0.474886 ],
                        [ 0.759578, 0.692933, 0.469725 ],
                        [ 0.766253, 0.698520, 0.464565 ],
                        [ 0.772927, 0.704107, 0.459404 ],
                        [ 0.779602, 0.709693, 0.454243 ],
                        [ 0.786276, 0.715280, 0.449082 ],
                        [ 0.792951, 0.720867, 0.443922 ],
                        [ 0.799625, 0.726453, 0.438761 ],
                        [ 0.806300, 0.732040, 0.433600 ],
                        [ 0.812975, 0.737627, 0.428439 ],
                        [ 0.819649, 0.743213, 0.423278 ],
                        [ 0.826324, 0.748800, 0.418118 ],
                        [ 0.832998, 0.754387, 0.412957 ],
                        [ 0.839673, 0.759973, 0.407796 ],
                        [ 0.846347, 0.765560, 0.402635 ],
                        [ 0.853022, 0.771147, 0.397475 ],
                        [ 0.859696, 0.776733, 0.392314 ],
                        [ 0.866371, 0.782320, 0.387153 ],
                        [ 0.873045, 0.787907, 0.381992 ],
                        [ 0.879720, 0.793493, 0.376831 ],
                        [ 0.886394, 0.799080, 0.371671 ],
                        [ 0.893069, 0.804667, 0.366510 ],
                        [ 0.899743, 0.810253, 0.361349 ],
                        [ 0.906418, 0.815840, 0.356188 ],
                        [ 0.913092, 0.821427, 0.351027 ],
                        [ 0.919767, 0.827013, 0.345867 ],
                        [ 0.926441, 0.832600, 0.340706 ],
                        [ 0.933116, 0.838187, 0.335545 ],
                        [ 0.939790, 0.843773, 0.330384 ],
                        [ 0.946465, 0.849360, 0.325224 ],
                        [ 0.953139, 0.854947, 0.320063 ],
                        [ 0.959814, 0.860533, 0.314902 ],
                        [ 0.966488, 0.866120, 0.309741 ],
                        [ 0.973163, 0.871707, 0.304580 ],
                        [ 0.976435, 0.871778, 0.301400 ],
                        [ 0.976306, 0.866334, 0.300200 ],
                        [ 0.976176, 0.860890, 0.299000 ],
                        [ 0.976047, 0.855446, 0.297800 ],
                        [ 0.975918, 0.850002, 0.296600 ],
                        [ 0.975788, 0.844558, 0.295400 ],
                        [ 0.975659, 0.839115, 0.294200 ],
                        [ 0.975529, 0.833671, 0.293000 ],
                        [ 0.975400, 0.828227, 0.291800 ],
                        [ 0.975271, 0.822783, 0.290600 ],
                        [ 0.975141, 0.817339, 0.289400 ],
                        [ 0.975012, 0.811895, 0.288200 ],
                        [ 0.974882, 0.806451, 0.287000 ],
                        [ 0.974753, 0.801007, 0.285800 ],
                        [ 0.974624, 0.795563, 0.284600 ],
                        [ 0.974494, 0.790119, 0.283400 ],
                        [ 0.974365, 0.784675, 0.282200 ],
                        [ 0.974235, 0.779231, 0.281000 ],
                        [ 0.974106, 0.773787, 0.279800 ],
                        [ 0.973976, 0.768344, 0.278600 ],
                        [ 0.973847, 0.762900, 0.277400 ],
                        [ 0.973718, 0.757456, 0.276200 ],
                        [ 0.973588, 0.752012, 0.275000 ],
                        [ 0.973459, 0.746568, 0.273800 ],
                        [ 0.973329, 0.741124, 0.272600 ],
                        [ 0.973200, 0.735680, 0.271400 ],
                        [ 0.973071, 0.730236, 0.270200 ],
                        [ 0.972941, 0.724792, 0.269000 ],
                        [ 0.972812, 0.719348, 0.267800 ],
                        [ 0.972682, 0.713904, 0.266600 ],
                        [ 0.972553, 0.708460, 0.265400 ],
                        [ 0.972424, 0.703016, 0.264200 ],
                        [ 0.972294, 0.697573, 0.263000 ],
                        [ 0.972165, 0.692129, 0.261800 ],
                        [ 0.972035, 0.686685, 0.260600 ],
                        [ 0.971906, 0.681241, 0.259400 ],
                        [ 0.971776, 0.675797, 0.258200 ],
                        [ 0.971647, 0.670353, 0.257000 ],
                        [ 0.971518, 0.664909, 0.255800 ],
                        [ 0.971388, 0.659465, 0.254600 ],
                        [ 0.971259, 0.654021, 0.253400 ],
                        [ 0.971129, 0.648577, 0.252200 ],
                        [ 0.971000, 0.643133, 0.251000 ],
                        [ 0.970871, 0.637689, 0.249800 ],
                        [ 0.970741, 0.632245, 0.248600 ],
                        [ 0.970612, 0.626802, 0.247400 ],
                        [ 0.970482, 0.621358, 0.246200 ],
                        [ 0.970353, 0.615914, 0.245000 ],
                        [ 0.970224, 0.610470, 0.243800 ],
                        [ 0.970094, 0.605026, 0.242600 ],
                        [ 0.969965, 0.599582, 0.241400 ],
                        [ 0.969835, 0.594138, 0.240200 ],
                        [ 0.969706, 0.588694, 0.239000 ],
                        [ 0.969576, 0.583250, 0.237800 ],
                        [ 0.969447, 0.577806, 0.236600 ],
                        [ 0.969318, 0.572362, 0.235400 ],
                        [ 0.969188, 0.566918, 0.234200 ],
                        [ 0.969059, 0.561475, 0.233000 ],
                        [ 0.968929, 0.556031, 0.231800 ],
                        [ 0.968800, 0.550587, 0.230600 ],
                        [ 0.968671, 0.545143, 0.229400 ],
                        [ 0.968541, 0.539699, 0.228200 ],
                        [ 0.968412, 0.534255, 0.227000 ],
                        [ 0.968282, 0.528811, 0.225800 ],
                        [ 0.968153, 0.523367, 0.224600 ],
                        [ 0.968024, 0.517923, 0.223400 ],
                        [ 0.967894, 0.512479, 0.222200 ],
                        [ 0.967765, 0.507035, 0.221000 ],
                        [ 0.967635, 0.501591, 0.219800 ],
                        [ 0.967506, 0.496147, 0.218600 ],
                        [ 0.967376, 0.490704, 0.217400 ],
                        [ 0.967247, 0.485260, 0.216200 ],
                        [ 0.967118, 0.479816, 0.215000 ],
                        [ 0.966988, 0.474372, 0.213800 ],
                        [ 0.966859, 0.468928, 0.212600 ],
                        [ 0.966729, 0.463484, 0.211400 ],
                        [ 0.966600, 0.458040, 0.210200 ],
                        [ 0.966471, 0.452596, 0.209000 ],
                        [ 0.966341, 0.447152, 0.207800 ],
                        [ 0.966212, 0.441708, 0.206600 ],
                        [ 0.966082, 0.436264, 0.205400 ],
                        [ 0.965953, 0.430820, 0.204200 ],
                        [ 0.965824, 0.425376, 0.203000 ],
                        [ 0.965694, 0.419933, 0.201800 ],
                        [ 0.965565, 0.414489, 0.200600 ],
                        [ 0.965435, 0.409045, 0.199400 ],
                        [ 0.965306, 0.403601, 0.198200 ],
                        [ 0.965176, 0.398157, 0.197000 ],
                        [ 0.965047, 0.392713, 0.195800 ],
                        [ 0.964918, 0.387269, 0.194600 ],
                        [ 0.964788, 0.381825, 0.193400 ],
                        [ 0.964659, 0.376381, 0.192200 ],
                        [ 0.964529, 0.370937, 0.191000 ],
                        [ 0.964400, 0.365493, 0.189800 ],
                        [ 0.964271, 0.360049, 0.188600 ],
                        [ 0.964141, 0.354605, 0.187400 ],
                        [ 0.964012, 0.349162, 0.186200 ],
                        [ 0.963882, 0.343718, 0.185000 ],
                        [ 0.963753, 0.338274, 0.183800 ],
                        [ 0.963624, 0.332830, 0.182600 ],
                        [ 0.963494, 0.327386, 0.181400 ],
                        [ 0.963365, 0.321942, 0.180200 ],
                        [ 0.963235, 0.316498, 0.179000 ],
                        [ 0.963106, 0.311054, 0.177800 ],
                        [ 0.962976, 0.305610, 0.176600 ],
                        [ 0.962847, 0.300166, 0.175400 ],
                        [ 0.962718, 0.294722, 0.174200 ],
                        [ 0.962588, 0.289278, 0.173000 ],
                        [ 0.962459, 0.283835, 0.171800 ],
                        [ 0.962329, 0.278391, 0.170600 ],
                        [ 0.962200, 0.272947, 0.169400 ],
                        [ 0.962071, 0.267503, 0.168200 ],
                        [ 0.961941, 0.262059, 0.167000 ],
                        [ 0.961812, 0.256615, 0.165800 ],
                        [ 0.961682, 0.251171, 0.164600 ],
                        [ 0.961553, 0.245727, 0.163400 ],
                        [ 0.961424, 0.240283, 0.162200 ],
                        [ 0.961294, 0.234839, 0.161000 ],
                        [ 0.961165, 0.229395, 0.159800 ],
                        [ 0.961035, 0.223951, 0.158600 ],
                        [ 0.960906, 0.218507, 0.157400 ],
                        [ 0.960776, 0.213064, 0.156200 ],
                        [ 0.960647, 0.207620, 0.155000 ],
                        [ 0.960518, 0.202176, 0.153800 ],
                        [ 0.960388, 0.196732, 0.152600 ],
                        [ 0.960259, 0.191288, 0.151400 ],
                        [ 0.960129, 0.185844, 0.150200 ],
                        [ 0.960000, 0.180400, 0.149000 ]]
                         
    # read the source file.
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update() 
    output = reader.GetOutput()
    scalar_range = output.GetScalarRange()
    
    # create own palette
    lut = vtk.vtkLookupTable()
    lut.SetRange(scalar_range)
    for i in range(len(colobar_agros2d)):
        lut.SetTableValue(i, colobar_agros2d[i][0], colobar_agros2d[i][1], colobar_agros2d[i][2])
    lut.Build()
    
    # create the mapper that corresponds the objects of the vtk file into graphics elements
    mapper = vtk.vtkDataSetMapper()
    mapper.SetInput(output)
    mapper.SetScalarRange(scalar_range)
    mapper.SetLookupTable(lut)
     
    # actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    
    return actor
             
setattr(agros2d, "vtk_scalar_actor", vtk_scalar_actor)
                     
def vtk_figure(output_filename, geometry, scalar = None, contours = None, width = 400, height = 230, scalar_colorbar = True):
    import vtk
    import math

    # bounding box
    bounds = geometry.GetBounds()
    cx = (bounds[0] + bounds[1]) / 2.0
    cy = (bounds[2] + bounds[3]) / 2.0
    scale = max(math.fabs(bounds[0] - cx), math.fabs(bounds[1] - cx), math.fabs(bounds[2] - cy), math.fabs(bounds[3] - cy))

    # renderer
    renderer = vtk.vtkRenderer()
    renderer.SetBackground(1, 1, 1)
        
    # scalar
    if (scalar != None):
        renderer.AddActor(scalar)  
            
    # contours
    if (contours != None):
        renderer.AddActor(contours)

    # geometry
    renderer.AddActor(geometry)
        
    # camera
    camera = renderer.GetActiveCamera()
    camera.ParallelProjectionOn()
    camera.SetParallelScale(scale)
    camera.SetPosition(cx, cy, 1);
    camera.SetFocalPoint(cx, cy, 0);
    
    # render window
    render_window = vtk.vtkRenderWindow()
    render_window.SetOffScreenRendering(1)
    render_window.SetSize(width, height)
    render_window.AddRenderer(renderer)
    render_window.Render()
    
    # scalar colorbar
    if (scalar != None):
        if (scalar_colorbar):
            interactor = vtk.vtkRenderWindowInteractor()
            interactor.SetRenderWindow(render_window)
            
            scalar_bar = vtk.vtkScalarBarActor()
            scalar_bar.SetOrientationToHorizontal()
            scalar_bar.SetNumberOfLabels(8)
            scalar_bar.SetLabelFormat("%+#6.2e")
            scalar_bar.SetLookupTable(scalar.GetMapper().GetLookupTable())
            scalar_bar.GetLabelTextProperty().SetFontFamilyToCourier()
            scalar_bar.GetLabelTextProperty().SetJustificationToRight()
            scalar_bar.GetLabelTextProperty().SetVerticalJustificationToCentered()
            scalar_bar.GetLabelTextProperty().BoldOff()
            scalar_bar.GetLabelTextProperty().ItalicOff()
            scalar_bar.GetLabelTextProperty().ShadowOff()        
            scalar_bar.GetLabelTextProperty().SetColor(0, 0, 0)
                
            # create the scalar bar widget
            scalar_bar_widget = vtk.vtkScalarBarWidget()
            scalar_bar_widget.SetInteractor(interactor)
            scalar_bar_widget.SetScalarBarActor(scalar_bar)
            scalar_bar_widget.On()    
       
    window_to_image_filter = vtk.vtkWindowToImageFilter()
    window_to_image_filter.SetInput(render_window)
    window_to_image_filter.Update()
         
    writer = vtk.vtkPNGWriter()
    writer.SetFileName(output_filename)
    writer.SetInputConnection(window_to_image_filter.GetOutputPort())
    writer.Write()

setattr(agros2d, "vtk_figure", vtk_figure)
