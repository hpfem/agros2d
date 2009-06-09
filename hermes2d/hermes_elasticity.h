#ifndef HERMES_ELASTICITY_H
#define HERMES_ELASTICITY_H

#include "util.h"
#include "solverdialog.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

using namespace std;

struct SolutionArray;
class SolverDialog;

struct ElasticityEdge
{
public:
    PhysicFieldBC typeX;
    PhysicFieldBC typeY;
    double forceX;
    double forceY;
};

struct ElasticityLabel
{
    double young_modulus;
    double poisson_ratio;

    // Lame constant
    inline double lambda() { return (young_modulus * poisson_ratio) / ((1 + poisson_ratio) * (1 - 2*poisson_ratio)); }
    inline double mu() { return young_modulus / (2*(1 + poisson_ratio)); }
};

static ElasticityEdge *elasticityEdge;
static ElasticityLabel *elasticityLabel;
static bool elasticityIsPlanar;

SolutionArray *elasticity_main(const char *fileName, ElasticityEdge *edge, ElasticityLabel *label, int numberOfRefinements, int polynomialOrder, bool isPlanar);

#endif // HERMES_ELASTICITY_H
