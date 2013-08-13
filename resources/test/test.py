import agros2d as a2d
import time

class Test():
    def __init__(self, error_file = None):
        self.error_file = error_file
        
        self.tests = list()
        self.errors = dict()
        self.elapsed_times = dict()

    def add(self, test):
        if test in self.tests:
            return

        self.tests.append(test)

    def run(self):
        start_time = time.time()
        app_time_start = a2d.app_time()

        for test in self.tests:
            self.__run_test__(test)

        self.elapsed_times['total'] = time.time() - start_time
        self.memory_usage = a2d.memory_usage()[1][app_time_start:]

        if (self.errors and self.error_file):
            self.__write_errors__()

        print('Total time: {0} s'.format(self.elapsed_times['total']))
        if len(self.memory_usage):
            print('Used memory: {0} MB'.format(self.memory_usage[-1]-self.memory_usage[0]))

    def __run_test__(self, test):
        locals = dict()
        start_time = start_time = time.time()
        execfile(test, dict(), locals)
        self.elapsed_times[test] = time.time() - start_time

        faults = []
        for key, value in locals.iteritems():
            if ((key.startswith('test')) and (not value)):
                faults.append(key)

        if (faults):
            self.errors[test] = faults

    def __write_errors__(self):
        file = open(self.error_file, 'w')
        for key, value in self.errors.iteritems():
            file.write('{0}; {1}\n'.format(key, "; ".join(value)))
        file.close()