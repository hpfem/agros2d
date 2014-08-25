import unittest as ut

class Agros2DTestCase(ut.TestCase):
    def __init__(self, methodName='runTest'):
        ut.TestCase.__init__(self, methodName)

    def value_test(self, text, value, normal, error = 0.03):
        if (abs(value) < 1e-50):
            if(abs(normal) < 1e-30):
                self.assertTrue(True)
            else:
                str = "{0}: Agros2D = {1}, correct = {2}".format(text, value, normal)
                self.assertTrue(False, str)
                
            return
            
        test = abs((value - normal)/normal) < error
        str = "{0}: Agros2D = {1}, correct = {2}, error = {3:.4f} %".format(text, value, normal, abs(value - normal)/value*100)
        self.assertTrue(test, str)
        
    def interval_test(self, text, value, min, max):
        test = abs((value - normal)/value) < error
        str = "{0}: Agros2D = {1}, correct = {2}, error = {3:.4f} %".format(text, value, normal, abs(value - normal)/value*100)
        self.assertTrue(test, str)

class Agros2DTestResult(ut.TestResult):
    def __init__(self):
        ut.TestResult.__init__(self)
        self.output = []

    def startTest(self, test):
        from time import time
        
        ut.TestResult.startTest(self, test)
        self.time = time()
        # print("{0}".format(test.id().ljust(60, "."))),

    def addSuccess(self, test):
        from time import time
        
        ut.TestResult.addSuccess(self, test)
        self.time -= time()

        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, -self.time * 1000, "OK", ""])
        
        print(("{0}".format(id.ljust(60, "."))), end=' ')
        print(("{0:08.2f}".format(-self.time * 1000).rjust(15, " ") + " ms " +
              "{0}".format("OK".rjust(10, "."))))

    def addError(self, test, err):
        ut.TestResult.addError(self, test, err)
        id = test.id().split(".")[-2] + "." + test.id().split(".")[-1]
        
        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, 0, "ERROR", err[1]])
        
        print(("{0}".format(id.ljust(60, "."))), end=' ')
        print(("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("ERROR".rjust(10, "."))))        

        import traceback
        #print(traceback.print_tb(err[2]))
        print(err[1])

    def addFailure(self, test, err):
        ut.TestResult.addFailure(self, test, err)
        id = test.id().split(".")[-2] + "." + test.id().split(".")[-1]

        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, 0, "FAILURE", str(err[1])])

        print(("{0}".format(id.ljust(60, "."))), end=' ')
        print(("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("FAILURE".rjust(10, "."))))        
        print((err[1]))      
        
    def report(self):
        return self.output

__scerarios__ = []
def find_all_scenarios():
    global __scerarios__
    if (len(__scerarios__) == 0):
        from test_suite.tests import all_tests

        for key in all_tests():
            __scerarios__.append(key)    
    
        __scerarios__.sort()
    
    return __scerarios__
           
__tests__ = []
def find_all_tests():
    global __tests__
    if (len(__tests__) == 0):
        from test_suite.tests import all_tests
        
        alltests = all_tests()
        
        # remove multiple items
        alltestsdist = []
        for key in alltests:
            for obj in alltests[key]:
                if (not obj.__name__.endswith("GeneralTestCase")):
                    if (not obj in alltestsdist):
                        alltestsdist.append(obj) 
                    
        for obj in alltestsdist:
            m = []
            for d in dir(obj):
                if (d.startswith("test_")):
                    m.append(d)
            
            m.sort()
            __tests__.append([obj.__name__, obj.__module__, m])
                    
        tmp = sorted(__tests__, key=lambda x: x[1]+"."+x[0])
        __tests__ = tmp
            
    return __tests__

def run_test(test): 
    agros2d_suite = ut.TestSuite(); 
    agros2d_suite.addTest(ut.TestLoader().loadTestsFromTestCase(test)); 
    agros2d_result = Agros2DTestResult(); 
    agros2d_suite.run(agros2d_result); 

    return agros2d_result.report()

def run_suite(tests): 
    suite = ut.TestSuite()
    
    for test in tests:
        suite.addTest(ut.TestLoader().loadTestsFromTestCase(test))
    
    result = Agros2DTestResult()
    suite.run(result)

    if (not result.wasSuccessful()):
        raise Exception('Failure', result.failures)
