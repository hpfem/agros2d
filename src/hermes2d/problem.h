#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"

template <typename Scalar>
class WeakFormAgros;

template <typename Scalar>
class SolutionArrayList;

class Field
{
public:
    bool solveInitVariables();

private:
    FieldInfo* m_fieldInfo;
};

/// represents one or more fields, that are hard-coupled -> produce only 1 weak form
class Block
{
public:

private:
    WeakFormAgros<double> *m_wf;
    SolutionArrayList<double> *m_solutionList;
};

/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class Problem
{
public:

    void solve();

    Hermes::vector<Block> blocks;
};

#endif // PROBLEM_H
