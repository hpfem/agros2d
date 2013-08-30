import agros2d as a2d

class TestField(a2d.Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')

    """ field_id """
    def test_field_id(self):
        self.assertEqual(self.field.field_id, 'magnetic')

    """ analysis_type """
    def test_analysis_type(self):
        for type in ['steadystate', 'transient', 'harmonic']:
            self.field.analysis_type = type
            self.assertEqual(self.field.analysis_type, type)

    def test_set_wrong_analysis_type(self):
        with self.assertRaises(ValueError):
            self.field.analysis_type = 'wrong_analysis_type'

    """ solver """
    def test_solver(self):
        for solver in ['linear', 'newton']:
            self.field.solver = solver
            self.assertEqual(self.field.solver, solver)

    def test_set_wrong_solver(self):
        with self.assertRaises(ValueError):
            self.field.solver = 'wrong_solver'

    """ matrix_solver """
    def test_matrix_solver(self):
        for solver in ['umfpack', 'paralution_iterative', 'paralution_amg',
                     'mumps', 'external', 'empty']:
            self.field.matrix_solver = solver
            self.assertEqual(self.field.matrix_solver, solver)

    def test_set_wrong_matrix_solver(self):
        with self.assertRaises(ValueError):
            self.field.matrix_solver = 'wrong_solver'

    """ number_of_refinements """
    def test_number_of_refinements(self):
        self.field.number_of_refinements = 2
        self.assertEqual(self.field.number_of_refinements, 2)

    def test_set_wrong_number_of_refinements(self):
        with self.assertRaises(IndexError):
            self.field.number_of_refinements = -1

        with self.assertRaises(IndexError):
            self.field.number_of_refinements = 6

    """ polynomial_order """
    def test_polynomial_order(self):
        self.field.polynomial_order = 5
        self.assertEqual(self.field.polynomial_order, 5)

    def test_set_wrong_polynomial_order(self):
        with self.assertRaises(IndexError):
            self.field.polynomial_order = 0

        with self.assertRaises(IndexError):
            self.field.polynomial_order = 11

    """ adaptivity_type """
    def test_adaptivity_typer(self):
        for type in ['hp-adaptivity', 'h-adaptivity', 'p-adaptivity', 'disabled']:
            self.field.adaptivity_type = type
            self.assertEqual(self.field.adaptivity_type, type)

    def test_set_wrong_adaptivity_type(self):
        with self.assertRaises(ValueError):
            self.field.adaptivity_type = 'wrong_type'

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = a2d.Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestField))
    suite.run(result)