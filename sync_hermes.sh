#!/bin/sh

# backup
cp hermes2d/src/config.h hermes2d/src/config.h.backup
cp hermes_common/common_time_period.h hermes_common/common_time_period.h.backup
cp hermes_common/common_time_period.cpp hermes_common/common_time_period.cpp.backup

cp ../hermes/hermes2d/src/*.cpp hermes2d/src
cp ../hermes/hermes2d/src/*.h hermes2d/src
cp ../hermes/hermes2d/src/adapt/*.cpp hermes2d/src/adapt
cp ../hermes/hermes2d/src/adapt/*.h hermes2d/src/adapt
cp ../hermes/hermes2d/src/boundaryconditions/*.cpp hermes2d/src/boundaryconditions
cp ../hermes/hermes2d/src/boundaryconditions/*.h hermes2d/src/boundaryconditions
cp ../hermes/hermes2d/src/function/*.cpp hermes2d/src/function
cp ../hermes/hermes2d/src/function/*.h hermes2d/src/function
cp ../hermes/hermes2d/src/gen/*.cpp hermes2d/src/gen
cp ../hermes/hermes2d/src/integrals/*.h hermes2d/src/integrals
cp ../hermes/hermes2d/src/linearizer/*.cpp hermes2d/src/linearizer
cp ../hermes/hermes2d/src/linearizer/*.h hermes2d/src/linearizer
cp ../hermes/hermes2d/src/quadrature/*.cpp hermes2d/src/quadrature
cp ../hermes/hermes2d/src/quadrature/*.h hermes2d/src/quadrature
cp ../hermes/hermes2d/src/mesh/*.cpp hermes2d/src/mesh
cp ../hermes/hermes2d/src/mesh/*.h hermes2d/src/mesh
cp ../hermes/hermes2d/src/ref_selectors/*.cpp hermes2d/src/ref_selectors
cp ../hermes/hermes2d/src/ref_selectors/*.h hermes2d/src/ref_selectors
cp ../hermes/hermes2d/src/shapeset/*.cpp hermes2d/src/shapeset
cp ../hermes/hermes2d/src/shapeset/*.h hermes2d/src/shapeset
cp ../hermes/hermes2d/src/space/*.cpp hermes2d/src/space
cp ../hermes/hermes2d/src/space/*.h hermes2d/src/space
cp ../hermes/hermes2d/src/views/*.cpp hermes2d/src/views
cp ../hermes/hermes2d/src/views/*.h hermes2d/src/views
cp ../hermes/hermes2d/src/weakform/*.cpp hermes2d/src/weakform
cp ../hermes/hermes2d/src/weakform/*.h hermes2d/src/weakform

cp ../hermes/hermes_common/*.cpp hermes_common
cp ../hermes/hermes_common/*.h hermes_common
cp ../hermes/hermes_common/solver/*.cpp hermes_common/solver
cp ../hermes/hermes_common/solver/*.h hermes_common/solver
cp ../hermes/hermes_common/compat/*.cpp hermes_common/compat
cp ../hermes/hermes_common/compat/*.cpp hermes_common/compat
cp ../hermes/hermes_common/third_party_codes/trilinos-teuchos/* hermes_common/third_party_codes/trilinos-teuchos
cp ../hermes/hermes_common/python/*.* hermes_common/python

# restore
cp hermes2d/src/config.h.backup hermes2d/src/config.h
cp hermes_common/common_time_period.h.backup hermes_common/common_time_period.h
cp hermes_common/common_time_period.cpp.backup hermes_common/common_time_period.cpp
