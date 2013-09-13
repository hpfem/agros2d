import test_suite

# fast tests
test_suite.scenario.run(test_suite.test_fast)
# nonlin tests
test_suite.scenario.run(test_suite.test_nonlin)
# script
test_suite.scenario.run(test_suite.test_script)