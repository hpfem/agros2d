#!/bin/sh

# backup
# cp hermes2d/src/config.h hermes2d/src/config.h.backup
# cp hermes_common/common_time_period.h hermes_common/common_time_period.h.backup
# cp hermes_common/common_time_period.cpp hermes_common/common_time_period.cpp.backup

mkdir hermes2d

mkdir hermes2d/src
mkdir hermes2d/src/adapt
mkdir hermes2d/src/boundary_conditions
mkdir hermes2d/src/function
mkdir hermes2d/src/mesh
mkdir hermes2d/src/quadrature
mkdir hermes2d/src/refinement_selectors
mkdir hermes2d/src/shapeset
mkdir hermes2d/src/space
mkdir hermes2d/src/views
mkdir hermes2d/src/weakform
mkdir hermes2d/src/weakform_library

mkdir hermes2d/include
mkdir hermes2d/include/adapt
mkdir hermes2d/include/boundary_conditions
mkdir hermes2d/include/function
mkdir hermes2d/include/integrals
mkdir hermes2d/include/mesh
mkdir hermes2d/include/quadrature
mkdir hermes2d/include/refinement_selectors
mkdir hermes2d/include/shapeset
mkdir hermes2d/include/space
mkdir hermes2d/include/views
mkdir hermes2d/include/weakform
mkdir hermes2d/include/weakform_library

mkdir hermes_common

mkdir hermes_common/src
mkdir hermes_common/src/solvers
mkdir hermes_common/src/third_party_codes
mkdir hermes_common/src/third_party_codes/trilinos-teuchos
mkdir hermes_common/src/python_API

mkdir hermes_common/include
mkdir hermes_common/include/solvers
mkdir hermes_common/include/third_party_codes
mkdir hermes_common/include/third_party_codes/trilinos-teuchos
mkdir hermes_common/include/python_API


cp ../hermes/hermes2d/src/*.cpp hermes2d/src
cp ../hermes/hermes2d/src/adapt/*.cpp hermes2d/src/adapt
cp ../hermes/hermes2d/src/boundary_conditions/*.cpp hermes2d/src/boundary_conditions
cp ../hermes/hermes2d/src/function/*.cpp hermes2d/src/function
cp ../hermes/hermes2d/src/mesh/*.cpp hermes2d/src/mesh
cp ../hermes/hermes2d/src/quadrature/*.cpp hermes2d/src/quadrature
cp ../hermes/hermes2d/src/refinement_selectors/*.cpp hermes2d/src/refinement_selectors
cp ../hermes/hermes2d/src/shapeset/*.cpp hermes2d/src/shapeset
cp ../hermes/hermes2d/src/space/*.cpp hermes2d/src/space
cp ../hermes/hermes2d/src/views/*.cpp hermes2d/src/views
cp ../hermes/hermes2d/src/weakform/*.cpp hermes2d/src/weakform
cp ../hermes/hermes2d/src/weakform_library/*.cpp hermes2d/src/weakform_library

cp ../hermes/hermes2d/include/*.h hermes2d/include
cp ../hermes/hermes2d/include/adapt/*.h hermes2d/include/adapt
cp ../hermes/hermes2d/include/boundary_conditions/*.h hermes2d/include/boundary_conditions
cp ../hermes/hermes2d/include/function/*.h hermes2d/include/function
cp ../hermes/hermes2d/include/integrals/*.h hermes2d/include/integrals
cp ../hermes/hermes2d/include/mesh/*.h hermes2d/include/mesh
cp ../hermes/hermes2d/include/quadrature/*.h hermes2d/include/quadrature
cp ../hermes/hermes2d/include/refinement_selectors/*.h hermes2d/include/refinement_selectors
cp ../hermes/hermes2d/include/shapeset/*.h hermes2d/include/shapeset
cp ../hermes/hermes2d/include/space/*.h hermes2d/include/space
cp ../hermes/hermes2d/include/views/*.h hermes2d/include/views
cp ../hermes/hermes2d/include/weakform/*.h hermes2d/include/weakform
cp ../hermes/hermes2d/include/weakform_library/*.h hermes2d/include/weakform_library


cp ../hermes/hermes_common/src/*.cpp hermes_common/src/
cp ../hermes/hermes_common/src/solvers/*.cpp hermes_common/src/solvers/
cp ../hermes/hermes_common/src/third_party_codes/trilinos-teuchos/* hermes_common/src/third_party_codes/trilinos-teuchos/
cp ../hermes/hermes_common/src/python_API/*.* hermes_common/src/python_API/

cp ../hermes/hermes_common/include/*.h hermes_common/include/
cp ../hermes/hermes_common/include/solvers/*.h hermes_common/include/solvers/
cp ../hermes/hermes_common/include/third_party_codes/trilinos-teuchos/* hermes_common/include/third_party_codes/trilinos-teuchos/
cp ../hermes/hermes_common/include/python_API/*.* hermes_common/include/python_API/

# restore
# cp hermes2d/src/config.h.backup hermes2d/src/config.h
# cp hermes_common/common_time_period.h.backup hermes_common/common_time_period.h
# cp hermes_common/common_time_period.cpp.backup hermes_common/common_time_period.cpp
# cp hermes_common/compat.h.backup hermes_common/compat.h
# cp hermes_common/config.h.backup hermes_common/config.h
# cp hermes_common/third_party_codes/trilinos-teuchos/Teuchos_DLLExportMacro.h.backup hermes_common/third_party_codes/trilinos-teuchos/Teuchos_DLLExportMacro.h
 
