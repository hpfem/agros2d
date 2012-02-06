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

#include "problem.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenesolution.h"
#include "module.h"
#include "module_agros.h"
#include "coupling.h"
#include "solver.h"
#include "progressdialog.h"


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

Block::Block(QList<FieldInfo *> fieldInfos, QList<Coupling*> couplings, ProgressItemSolve* progressItemSolve) :
    m_progressItemSolve(progressItemSolve), m_couplings(couplings)
{
    foreach(FieldInfo* fi, fieldInfos)
    {
        m_fields.append(new Field(fi));
    }

    m_solutionList = new SolutionArrayList<double>;
}

bool Block::solveInit(Hermes::Hermes2D::Solution<double> *sourceSolution)
{
    foreach(Field* field, m_fields)
    {
        if(! field->solveInitVariables())
            assert(0); //TODO co to znamena?
    }
    assert(m_couplings.size() <= 1);
    if(m_couplings.size())
        m_wf = new WeakFormAgros<double>(this, sourceSolution);
    else
        m_wf = new WeakFormAgros<double>(this);

    m_solutionList->init(m_progressItemSolve, m_wf, this);
    m_solutionList->clear();
}

void Block::solve()
{
    m_solutionList->solve();
    cout << "num elem pri prirazeni do scene solution " <<  Util::problem()->meshInitial()->get_num_active_elements() << endl;

    foreach(Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        Util::scene()->sceneSolution(fieldInfo)->setMeshInitial(Util::problem()->meshInitial());

        QList<SolutionArray<double>* > solutionArrays;
        for(int i = 0; i < fieldInfo->module()->number_of_solution(); i++)
            solutionArrays.push_back(m_solutionList->at(i + offset(field)));
        Util::scene()->sceneSolution(fieldInfo)->setSolutionArray(solutionArrays);
    }
}

int Block::numSolutions() const
{
    int num = 0;

    foreach(Field* field, m_fields)
    {
        num += field->fieldInfo()->module()->number_of_solution();
    }

    return num;
}

int Block::offset(Field *fieldParam) const
{
    int offset = 0;

    foreach(Field* field, m_fields)
    {
        if(field == fieldParam)
            return offset;
        else
            offset += field->fieldInfo()->module()->number_of_solution();
    }

    assert(0);
}

LinearityType Block::linearityType() const
{
    int linear = 0, newton = 0;
    foreach(Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->linearityType == LinearityType_Linear)
            linear++;
        if(fieldInfo->linearityType == LinearityType_Newton)
            newton++;
    }
    assert(linear * newton == 0); // all hard coupled fields has to be solved by the same method

    if(linear)
        return LinearityType_Linear;
    else if(newton)
        return  LinearityType_Newton;
    else
        assert(0);
}

double Block::nonlinearTolerance() const
{
    double tolerance = 10e20;

    foreach(Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->nonlinearTolerance < tolerance)
            tolerance = fieldInfo->nonlinearTolerance;
    }

    return tolerance;
}

int Block::nonlinearSteps() const
{
    int steps = 0;

    foreach(Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->nonlinearSteps > steps)
            steps = fieldInfo->nonlinearSteps;
    }

    return steps;
}

Field* Block::field(FieldInfo *fieldInfo) const
{
    foreach(Field* field, m_fields)
    {
        if(fieldInfo == field->fieldInfo())
            return field;
    }

    return NULL;
}

Problem::Problem()
{
    m_timeStep = 0;
    m_isSolved = false;
    m_isSolving = false;
//    m_progressDialog = new ProgressDialog();
//    m_progressItemMesh = new ProgressItemMesh();
//    m_progressItemSolve = new ProgressItemSolve();
//    m_progressItemSolveAdaptiveStep = new ProgressItemSolveAdaptiveStep();
//    m_progressItemProcessView = new ProgressItemProcessView();

}

void Problem::createStructure()
{
    QMap<QString, FieldInfo *> fieldInfos = Util::scene()->fieldInfos();

    if(hardCoupling) //TODO information about coupling method move to some CouplingInfo ...
    {
        QList<FieldInfo*> fieldInfosParam;
//        foreach(FieldInfo* fi, fieldInfos)
//        {
//            fieldInfosParam.append(fi);
//        }
        //TODO create order in fields and use previous cycle
        fieldInfosParam.append(Util::scene()->fieldInfo("elasticity"));
        fieldInfosParam.append(Util::scene()->fieldInfo("heat"));

        //TODO temporary
        Coupling *heatElastCoup = new Coupling(CoordinateType_Planar);
        heatElastCoup->read("resources/couplings/heat-elasticity-hard.xml");
        QList<Coupling*> couplingsParam;
        couplingsParam.append(heatElastCoup);

        m_blocks.append(new Block(fieldInfosParam, couplingsParam, m_progressItemSolve));
    }
    else
    {
        //TODO temporary
        Coupling *heatElastCoup = new Coupling(CoordinateType_Planar);
        heatElastCoup->read("resources/couplings/heat-elasticity.xml");
        //m_couplings.push_back(heatElastCoup);

        foreach(FieldInfo* fi, fieldInfos)
        {
            QList<FieldInfo*> fieldInfosParam;
            fieldInfosParam.append(fi);

            //TODO temporary
            QList<Coupling*> couplingsParam;
            if(fi == Util::scene()->fieldInfo("elasticity"))
                couplingsParam.append(heatElastCoup);
            m_blocks.append(new Block(fieldInfosParam, couplingsParam, m_progressItemSolve));
        }

    }
}

void Problem::mesh()
{
    ProgressItemMesh* pim = new ProgressItemMesh();
    pim->mesh();
}

void Problem::postprocess()
{
    ProgressItemProcessView* pipv = new ProgressItemProcessView();
    pipv->run();
}

void Problem::solve(SolverMode solverMode)
{
    logMessage("SceneSolution::solve()");

    setVerbose(true);

    if (isSolving()) return;

    m_isSolving = true;

    // clear problem
    //clear(solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve);

    // open indicator progress
    Indicator::openProgress();

    // save problem
    ErrorResult result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

//    if (solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve)
//    {
//        m_progressDialog->clear();
//        m_progressDialog->appendProgressItem(m_progressItemMesh);
//        if (solverMode == SolverMode_MeshAndSolve)
//        {
//            m_progressDialog->appendProgressItem(m_progressItemSolve);
//            m_progressDialog->appendProgressItem(m_progressItemProcessView);
//        }
//    }
//    else if (solverMode == SolverMode_SolveAdaptiveStep)
//    {
//        m_progressDialog->appendProgressItem(m_progressItemSolveAdaptiveStep);
//        m_progressDialog->appendProgressItem(m_progressItemProcessView);
//    }

//    if (m_progressDialog->run())
//    {
//        setTimeStep(timeStepCount() - 1);
//        //emit meshed();
//        //emit solved();
//    }

    createStructure();

    Util::scene()->setActiveViewField(Util::scene()->fieldInfo("elasticity"));

    mesh();
    emit meshed();

    Util::scene()->createSolutions();

    assert(isMeshed());
    if (isMeshed())
    {
        InitialCondition<double> initial(m_meshInitial, 0.0);
        Util::scene()->activeSceneSolution()->linInitialMeshView().process_solution(&initial);
    }


//    foreach(Block* block, m_blocks)
//    {
//        block->solveInit();
//        block->solve();
//    }

    const int elastTODO = 0;  //TODO temp
    const int heatTODO = 1;   //TODO temp

    if(hardCoupling)
    {
        m_blocks[0]->solveInit();
        m_blocks[0]->solve();
    }
    else
    {
        m_blocks[heatTODO]->solveInit();
        m_blocks[heatTODO]->solve();

        m_blocks[elastTODO]->solveInit(m_blocks[heatTODO]->m_solutionList->at(0)->sln.get());
        m_blocks[elastTODO]->solve();
    }
    // delete temp file
    if (Util::scene()->problemInfo()->fileName == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::scene()->problemInfo()->fileName);
        Util::scene()->problemInfo()->fileName = "";
    }


    // close indicator progress
    Indicator::closeProgress();

    m_isSolving = false;
    m_isSolved = true;
    emit solved();
    emit timeStepChanged(false);

    postprocess();

    //    if (!solve_init_variables())
    //        return Hermes::vector<SolutionArray<double> *>(); //TODO PK <double>

    //    WeakFormAgros<double> wf(number_of_solution()); //TODO PK <double>

    //    SolverAgros<double> solutionAgros(progressItemSolve, &wf);

    //    Hermes::vector<SolutionArray<double> *> solutionArrayList = solutionAgros.solve(); //TODO PK <double>
    //    return solutionArrayList;


//    foreach(Block* block, m_blocks)
//    {
//        block->solveInit();
//        block->solve();
//    }

}

ProgressDialog* Problem::progressDialog()
{
    return m_progressDialog;
}


