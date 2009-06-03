#ifndef ELECTROSTATIC_H
#define ELECTROSTATIC_H

#include "util.h"
#include "scenehermes.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

using namespace std;

struct SolutionArray;

struct ElectrostaticEdge
{
public:
    PhysicFieldBC type;
    double value;        
};

struct ElectrostaticLabel
{
    double charge_density;
    double permittivity;
};

static double ERR_STOP = 2;
static double THRESHOLD = 0.3;
static int STRATEGY = 0;
static int H_ONLY = 0;
static int NDOF_STOP = 40000;

static ElectrostaticEdge *electrostaticEdge;
static ElectrostaticLabel *electrostaticLabel;
static bool electrostaticIsPlanar;

double electrostatic_integrate(MeshFunction* sln, int marker);

SolutionArray electrostatic_main(const char *fileName, ElectrostaticEdge *edge, ElectrostaticLabel *label, int numberOfRefinements, int polynomialOrder, int adaptivitySteps, bool isPlanar);

#endif // ELECTROSTATIC_H
