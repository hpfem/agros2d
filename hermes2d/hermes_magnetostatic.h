#ifndef MAGNETOSTATIC_H
#define MAGNETOSTATIC_H

#include "util.h"
#include "scenehermes.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

#include <iostream>

using namespace std;

struct SolutionArray;

struct MagnetostaticEdge
{
    PhysicFieldBC type;
    double value;        
};

struct MagnetostaticLabel
{
    double current_density;
    double permeability;
};

static MagnetostaticEdge *magnetostaticEdge;
static MagnetostaticLabel *magnetostaticLabel;
static bool magnetostaticIsPlanar;

SolutionArray magnetostatic_main(const char *fileName, MagnetostaticEdge *edge, MagnetostaticLabel *label, int numberOfRefinements, int polynomialOrder, int adaptivitySteps, bool isPlanar);

#endif // MAGNETOSTATIC_H
