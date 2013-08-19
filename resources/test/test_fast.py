from test import Test

test_fast = Test(error_file='test_fast.err')

# electromagnetic wave - TE
test_fast.add("fields/test_rf_te_planar.py")
test_fast.add("fields/test_rf_te_axisymmetric.py")

# electromagnetic wave - TM
test_fast.add("fields/test_rf_tm_planar.py")
test_fast.add("fields/test_rf_tm_axisymmetric.py")

# internal
#test_fast.add("internal/test_matrix_solvers.py")

test_fast.run()
