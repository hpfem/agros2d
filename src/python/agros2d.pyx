cdef extern from "../scripteditorcommandpython.h":
    void pythonMessage(char *message)
    void pythonAddNode(double x, double y)

def message(char *message):
    pythonMessage(message)

def addnode(double x, double y):
    pythonAddNode(x, y)
