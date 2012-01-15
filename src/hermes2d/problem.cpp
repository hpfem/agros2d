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

#include "scene.h"
#include "scenemarker.h"
#include "module.h"
#include "solver.h"
#include "problem.h"


Field::Field(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

bool Field::solveInitVariables()
{
    //TODO transient
    //
    //
    //    // transient
    //    if (Util::scene()->problemInfo()->analysisType() == AnalysisType_Transient)
    //    {
    //        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return false;
    //        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return false;
    //        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return false;
    //    }

    if(!Util::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if(!Util::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

Block::Block(QList<FieldInfo *> fieldInfos, ProgressItemSolve* progressItemSolve) : m_progressItemSolve(progressItemSolve)
{
    foreach(FieldInfo* fi, fieldInfos)
    {
        m_fields.append(new Field(fi));
    }

    m_solutionList = new SolutionArrayList<double>;
}

bool Block::solveInit()
{
    foreach(Field* field, m_fields)
    {
        if(! field->solveInitVariables())
            assert(0); //TODO co to znamena?
    }

    m_wf = new WeakFormAgros<double>(1);

    m_solutionList->init(m_progressItemSolve, m_wf, m_fields.at(0)->m_fieldInfo);
    m_solutionList->clear();
}

void Block::solve()
{
    m_solutionList->solve();
}

Problem::Problem(ProgressItemSolve* progressItemSolve) : m_progressItemSolve(progressItemSolve)
{

}

void Problem::createStructure()
{
    QMap<QString, FieldInfo *> fieldInfos = Util::scene()->fieldInfos();
    assert(fieldInfos.size() == 1);
    QList<FieldInfo*> tmp;
    tmp.append(fieldInfos.begin().value());
    m_blocks.append(new Block(tmp, m_progressItemSolve));
}

void Problem::solve()
{
    //    if (!solve_init_variables())
    //        return Hermes::vector<SolutionArray<double> *>(); //TODO PK <double>

    //    WeakFormAgros<double> wf(number_of_solution()); //TODO PK <double>

    //    SolverAgros<double> solutionAgros(progressItemSolve, &wf);

    //    Hermes::vector<SolutionArray<double> *> solutionArrayList = solutionAgros.solve(); //TODO PK <double>
    //    return solutionArrayList;


    foreach(Block* block, m_blocks)
    {
        block->solveInit();
        block->solve();
    }


}
