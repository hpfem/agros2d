// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef BLOCK_H
#define BLOCK_H

#include "util.h"
#include "util/enums.h"

class FieldInfo;
class CouplingInfo;
class Field;
class Problem;
class SceneBoundary;

template <typename Scalar>
class ExactSolutionScalarAgros;

template <typename Scalar>
class ProblemSolver;

template <typename Scalar>
class WeakFormAgros;


/// represents one or more fields, that are hard-coupled -> produce only 1 weak form
class Block
{
public:
    Block(QList<FieldInfo*> fieldInfos, QList<CouplingInfo*> couplings);
    ~Block();

    ProblemSolver<double> *prepareSolver();
    void createBoundaryConditions();

    inline WeakFormAgros<double> *weakForm() { return m_wf;}
    void setWeakForm(WeakFormAgros<double> *wf);

    int numSolutions() const;
    int offset(Field* field) const;

    LinearityType linearityType() const;
    bool isTransient() const;

    // returns minimal time skip of individual fields
    double timeSkip() const;
    //bool skipThisTimeStep() const;

    AdaptivityType adaptivityType() const;
    int adaptivitySteps() const;
    double adaptivityTolerance() const;
    int adaptivityBackSteps() const;
    int adaptivityRedoneEach() const;

    // minimal nonlinear tolerance of individual fields
    double nonlinearTolerance() const;

    // maximal nonlinear steps of individual fields
    int nonlinearSteps() const;

    // use only if true for all fields
    bool newtonAutomaticDamping() const;

    // minimal damping coefficient
    double newtonDampingCoeff() const;

    // minimal automatic damping coefficient
    double newtonAutomaticDampingCoeff() const;

    // maximal damping number to increase in one step
    int newtonDampingNumberToIncrease() const;

    // use Anderson acceleration
    bool picardAndersonAcceleration() const;

    // Anderson beta
    double picardAndersonBeta() const;

    // number of last vectors used for Anderson acceleration
    int picardAndersonNumberOfLastVectors() const;

    bool contains(FieldInfo* fieldInfo) const;
    Field* field(FieldInfo* fieldInfo) const;

    inline QList<Field*> fields() const { return m_fields; }
    inline QList<CouplingInfo*> couplings() const { return m_couplings; }

    inline Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> *> bcs() const { return m_bcs; }

    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormTypeVector() const;

    void updateExactSolutionFunctions();

private:
    WeakFormAgros<double> *m_wf;
    Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> *> m_bcs;

    QMap<ExactSolutionScalarAgros<double>*, SceneBoundary *> m_exactSolutionFunctions;

    QList<Field*> m_fields;
    QList<CouplingInfo*> m_couplings;
};

ostream& operator<<(ostream& output, const Block& id);

#endif // BLOCK_H
