#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"

template <typename Scalar>
class WeakFormAgros;

template <typename Scalar>
class SolutionArrayList;

class FieldInfo;
class ProgressItemSolve;

class Field
{
public:
    Field(FieldInfo* fieldInfo);
    bool solveInitVariables();

public:
//private:
    FieldInfo* m_fieldInfo;
};

/// represents one or more fields, that are hard-coupled -> produce only 1 weak form
class Block
{
public:
    Block(QList<FieldInfo*> fieldInfos, ProgressItemSolve* progressItemSolve);

    bool solveInit();
    void solve();

public:
//private:
    WeakFormAgros<double> *m_wf;
    SolutionArrayList<double> *m_solutionList;

    QList<Field*> m_fields;
    ProgressItemSolve* m_progressItemSolve;
};

/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class Problem
{
public:
    Problem(ProgressItemSolve* progressItemSolve);
    void createStructure();
    void solve();

public:
//private:
    QList<Block*> m_blocks;
    ProgressItemSolve* m_progressItemSolve;
};

#endif // PROBLEM_H
