#ifndef BLOCK_H
#define BLOCK_H

#include "util.h"

class FieldInfo;
class CouplingInfo;
class Field;
class Problem;

template <typename Scalar>
class Solver;

template <typename Scalar>
class WeakFormAgros;

/// represents one or more fields, that are hard-coupled -> produce only 1 weak form
class Block
{
public:
    Block(QList<FieldInfo*> fieldInfos, QList<CouplingInfo*> couplings);

    Solver<double>* prepareSolver();

    QSharedPointer<WeakFormAgros<double> > weakForm() { return m_wf;}
    void setWeakForm(QSharedPointer<WeakFormAgros<double> > wf) {m_wf = wf;}

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

    // minimal nonlinear tolerance of individual fields
    double nonlinearTolerance() const;

    //maximal nonlinear steps of individual fields
    int nonlinearSteps() const;

//    Field* couplingSourceField(Coupling* coupling) const;
//    Field* couplingTargetField(Coupling* coupling) const;

    bool contains(FieldInfo* fieldInfo) const;
    Field* field(FieldInfo* fieldInfo) const;

    inline QList<Field*> fields() const { return m_fields; }
    inline QList<CouplingInfo*> couplings() const { return m_couplings; }

    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormTypeVector() const;

private:
    QSharedPointer<WeakFormAgros<double> > m_wf;

    QList<Field*> m_fields;
    QList<CouplingInfo*> m_couplings;
};

ostream& operator<<(ostream& output, const Block& id);

#endif // BLOCK_H
