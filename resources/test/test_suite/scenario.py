import agros2d
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
        
        print("{0}".format(id.ljust(60, "."))),
        print("{0:08.2f}".format(-self.time * 1000).rjust(15, " ") + " ms " +
              "{0}".format("OK".rjust(10, ".")))

    def addError(self, test, err):
        ut.TestResult.addError(self, test, err)
        id = test.id().split(".")[-2] + "." + test.id().split(".")[-1]
        
        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, 0, "ERROR", err[1]])
        
        print("{0}".format(id.ljust(60, "."))),
        print("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("ERROR".rjust(10, ".")))        
        print(err[1])

    def addFailure(self, test, err):
        ut.TestResult.addFailure(self, test, err)
        id = test.id().split(".")[-2] + "." + test.id().split(".")[-1]

        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, 0, "FAILURE", str(err[1])])

        print("{0}".format(id.ljust(60, "."))),
        print("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("FAILURE".rjust(10, ".")))        
        print(err[1])      
        
    def report(self):
        return self.output

def find_all_scenarios(obj, scerarios):
    for o in dir(obj):
	    if (o.startswith("test_")):
        	scerarios.append(o)
             
def find_all_tests(obj, tests):
    from inspect import getmembers, isclass, ismodule, ismethod

    for o in getmembers(obj, ismodule):
        if (o[1].__name__.startswith("test_suite.")):
            find_all_tests(o[1], tests)

    for o in getmembers(obj, isclass):
        if (issubclass(o[1], Agros2DTestCase) and o[1].__name__ != "Agros2DTestCase"):
            m = []
            for d in getmembers(o[1], ismethod):
                if (d[0].startswith("test_")):
                    m.append(d[0])
                    
            tests.append([o[0], o[1].__module__, m])

def run(tests): 
    suite = ut.TestSuite()
    
    for test in tests:
        suite.addTest(ut.TestLoader().loadTestsFromTestCase(test))
    
    result = Agros2DTestResult()
    suite.run(result)

    if (not result.wasSuccessful()):
        raise Exception('Failure', result.failures)
