import numpy as np

A = np.array([[3, 2, -1], [2, -2, 4], [-1, 1/2.0, -1]])
b = np.array([1, -2, 0])

if (np.linalg.det(A) != 0):
    Ai = np.linalg.inv(A)
    x = np.dot(Ai, b)

    print(x)