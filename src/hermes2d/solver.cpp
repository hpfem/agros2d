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

#include "solver.h"

#include "module.h"
#include "module_agros.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenesolution.h"
#include "progressdialog.h"
#include "weakform_parser.h"
#include "../weakform/src/weakform_factory.h"

using namespace Hermes::Hermes2D;

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray()
{
    logMessage("SolutionArray::SolutionArray()");

    sln.reset();
    space.reset();

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

template <typename Scalar>
SolutionArray<Scalar>::~SolutionArray()
{
    logMessage("SolutionArray::~SolutionArray()");
}

template <typename Scalar>
void SolutionArray<Scalar>::load(QDomElement element)
{
    logMessage("SolutionArray::load()");

    QString fileNameSolution = tempProblemFileName() + ".sln";
    QString fileNameSpace = tempProblemFileName() + ".spc";

    // write content (saved solution)
    QByteArray contentSolution;
    contentSolution.append(element.elementsByTagName("sln").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSolution, QByteArray::fromBase64(contentSolution));

    // write content (saved space)
    QByteArray contentSpace;
    contentSpace.append(element.elementsByTagName("space").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSpace, QByteArray::fromBase64(contentSpace));

    sln = shared_ptr<Solution<Scalar> >(new Hermes::Hermes2D::Solution<Scalar>());
    // sln->load(fileNameSolution.toStdString().c_str());
    //space = new Hermes::Hermes2D::Space<Scalar>();
    //space->load(fileNameSpace.toStdString().c_str());
    adaptiveError = element.attribute("adaptiveerror").toDouble();
    adaptiveSteps = element.attribute("adaptivesteps").toInt();
    time = element.attribute("time").toDouble();

    // delete solution
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

template <typename Scalar>
void SolutionArray<Scalar>::save(QDomDocument *doc, QDomElement element)
{
    logMessage("SolutionArray::save()");

    // solution
    QString fileNameSolution = tempProblemFileName() + ".sln";
    // sln->save(fileNameSolution.toStdString().c_str(), false);
    QDomText textSolution = doc->createTextNode(readFileContentByteArray(fileNameSolution).toBase64());

    // space
    QString fileNameSpace = tempProblemFileName() + ".spc";
    //space->save_data(fileNameSpace.toStdString().c_str());
    QDomNode textSpace = doc->createTextNode(readFileContentByteArray(fileNameSpace).toBase64());

    QDomNode eleSolution = doc->createElement("sln");
    QDomNode eleSpace = doc->createElement("space");

    eleSolution.appendChild(textSolution);
    eleSpace.appendChild(textSpace);

    element.setAttribute("adaptiveerror", adaptiveError);
    element.setAttribute("adaptivesteps", adaptiveSteps);
    element.setAttribute("time", time);
    element.appendChild(eleSolution);
    element.appendChild(eleSpace);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

// *********************************************************************************************


template <typename Scalar>
void SolutionArrayList<Scalar>::init(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf, FieldInfo *fieldInfo)
{
    m_fieldInfo = fieldInfo;
    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
    isError = false;
}

template <typename Scalar>
SolutionArray<Scalar>* SolutionArrayList<Scalar>::at(int i)
{
    if(i >= listOfSolutionArrays.size()){
        printf("i %d, size %d\n", i, listOfSolutionArrays.size());
        assert(0);
    }
    return listOfSolutionArrays.at(i);
}

template <typename Scalar>
void SolutionArrayList<Scalar>::clear()
{
    printf("clear solution list, size %d\n", listOfSolutionArrays.size());
    //TODO memory leak!
    //TODO ale pada to na delete space ...
    for(int i=0; i < listOfSolutionArrays.size(); i++){
        assert(listOfSolutionArrays.at(i) != NULL);
        delete listOfSolutionArrays.at(i);
    }
    listOfSolutionArrays.clear();
    printf("cleared, size %d\n", listOfSolutionArrays.size());
}

template <typename Scalar>
void SolutionArrayList<Scalar>::readMesh()
{
    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".xml");
    refineMesh(m_fieldInfo, mesh, true, true);
}


template <typename Scalar>
void SolutionArrayList<Scalar>::createSpace()
{
    printf("---- createSpace()\n");
    // essential boundary conditions
    Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> *> bcs; //TODO PK <double>
    for (int i = 0; i < m_fieldInfo->module()->number_of_solution(); i++)
        bcs.push_back(new Hermes::Hermes2D::EssentialBCs<double>());  //TODO PK <double>

    int i = 0;
    foreach(SceneEdge* edge, Util::scene()->edges->items())
    {
        SceneBoundary *boundary = edge->getMarker(m_fieldInfo);

        if (boundary && (!boundary->isNone()))
        {
           // printf(" ---- chci typ %s\n", boundary->type.data());
            Hermes::Module::BoundaryType *boundary_type = m_fieldInfo->module()->get_boundary_type(boundary->getType());

            printf(" ---- bdr type %s\n", boundary_type->id.data(), boundary_type->name.data());
            for (Hermes::vector<ParserFormEssential *>::iterator it = boundary_type->essential.begin();
                 it < boundary_type->essential.end(); ++it)
            {
                ParserFormEssential *form = ((ParserFormEssential *) *it);

                Hermes::Hermes2D::EssentialBoundaryCondition<Scalar> *custom_form = NULL;

                // compiled form
                if (m_fieldInfo->weakFormsType == WeakFormsType_Compiled)
                {
                    assert(0);
//                    string problemId = m_fieldInfo->module()->fieldid + "_" +
//                            analysisTypeToStringKey(m_fieldInfo->module()->get_analysis_type()).toStdString()  + "_" +
//                            coordinateTypeToStringKey(m_fieldInfo->module()->get_problem_type()).toStdString();

//                    Hermes::Hermes2D::ExactSolutionScalar<double> * function = factoryExactSolution<double>(problemId, form->i-1, mesh, boundary);
//                    custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(i + 1).toStdString(),
//                                                                                           function);
                }

                if (!custom_form && m_fieldInfo->weakFormsType == WeakFormsType_Compiled)
                    qDebug() << "Cannot find compiled VectorFormEssential().";

                // interpreted form
                if (!custom_form || m_fieldInfo->weakFormsType == WeakFormsType_Interpreted)
                {
                    {
                        CustomExactSolution<double> *function = new CustomExactSolution<double>(mesh,
                                                                                                form->expression,
                                                                                                boundary);
                        custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(i + 1).toStdString(),
                                                                                               function);
                    }
                }

                if (custom_form)
                {
                    bcs[form->i - 1]->add_boundary_condition(custom_form);
                }
            }
        }
        i++;
    }

    // create space
    for (int i = 0; i < m_fieldInfo->module()->number_of_solution(); i++)
    {
        space.push_back(shared_ptr<Space<Scalar> >(new Hermes::Hermes2D::H1Space<Scalar>(mesh, bcs[i], m_fieldInfo->polynomialOrder)));

        int j = 0;
        // set order by element
        foreach(SceneLabel* label, Util::scene()->labels->items()){
            if (!label->getMarker(m_fieldInfo)->isNone())
                space.at(i)->set_uniform_order(label->polynomialOrder > 0 ? label->polynomialOrder : m_fieldInfo->polynomialOrder,
                                               QString::number(j).toStdString());
            j++;
        }
    }

}

template <typename Scalar>
void SolutionArrayList<Scalar>::createSolutions(bool copyPrevious)
{
    for (int i = 0; i < m_fieldInfo->module()->number_of_solution(); i++)
    {
        // solution agros array
        Solution<double> *sln = new Solution<double>();
        solution.push_back(shared_ptr<Solution<double> >(sln));

        // single adaptive step
        if (copyPrevious)
            sln->copy((listOfSolutionArrays.at(listOfSolutionArrays.size() - m_fieldInfo->module()->number_of_solution() + i)->sln).get());

        if (m_fieldInfo->adaptivityType != AdaptivityType_None)
        {
            // reference solution
            solutionReference.push_back(shared_ptr<Solution<Scalar> >(new Solution<Scalar>()));
        }
    }

}

template <typename Scalar>
void SolutionArrayList<Scalar>::initSelectors()
{
    // set adaptivity selector
    select = NULL;
    switch (m_fieldInfo->adaptivityType)
    {
    case AdaptivityType_H:
        select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
        break;
    case AdaptivityType_P:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    }

    // create types of projection and selectors
    for (int i = 0; i < m_fieldInfo->module()->number_of_solution(); i++)
    {
        if (m_fieldInfo->adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
        }
    }
}

template <typename Scalar>
void SolutionArrayList<Scalar>::cleanup()
{
    // delete mesh
    //delete mesh;

    // clear space vector (space is deleted in SolutionArray)
    space.clear();

    // delete last solution
    solution.clear();

    // delete reference solution
    solutionReference.clear();

    // delete selector
    if (select)
        delete select;
    selector.clear();
}

template <typename Scalar>
bool SolutionArrayList<Scalar>::solveOneProblem(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
                                          Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam)
{
    // Initialize the FE problem.
    Hermes::Hermes2D::DiscreteProblem<Scalar> dp(m_wf, castConst(desmartize(spaceParam)));

    // Linear solver
    if (m_fieldInfo->linearityType == LinearityType_Linear)
    {
        // set up the solver, matrix, and rhs according to the solver selection.
        Hermes::Algebra::SparseMatrix<Scalar> *matrix = create_matrix<Scalar>(Hermes::SOLVER_UMFPACK);
        Hermes::Algebra::Vector<Scalar> *rhs = create_vector<Scalar>(Hermes::SOLVER_UMFPACK);
        Hermes::Algebra::LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(Hermes::SOLVER_UMFPACK, matrix, rhs);

        // assemble the linear problem.
        dp.assemble(matrix, rhs);

        if (solver->solve())
        {
            Hermes::Hermes2D::Solution<Scalar>::vector_to_solutions(solver->get_sln_vector(), castConst(desmartize(spaceParam)), desmartize(solutionParam));

            Hermes::Hermes2D::Views::Linearizer lin;
            bool mode_3D = true;
            lin.save_solution_vtk(solutionParam[0].get(), "sln.vtk", "SLN", mode_3D);
        }
        else
        {
            m_progressItemSolve->emitMessage(QObject::tr("Linear solver failed."), true);
            return false;
        }

        delete matrix;
        delete rhs;
        delete solver;
    }

    // Nonlinear solver
    if (m_fieldInfo->linearityType == LinearityType_Newton)
    {
        // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
        Hermes::Hermes2D::NewtonSolver<Scalar> newton(&dp, Hermes::SOLVER_UMFPACK);
        try
        {
            int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(spaceParam)));

            // Initial coefficient vector for the Newton's method.
            Scalar* coeff_vec = new Scalar[ndof];
            memset(coeff_vec, 0, ndof*sizeof(Scalar));

            newton.solve(coeff_vec, m_fieldInfo->nonlinearTolerance, m_fieldInfo->nonlinearSteps);

            Hermes::Hermes2D::Solution<Scalar>::vector_to_solutions(newton.get_sln_vector(), castConst(desmartize(spaceParam)), desmartize(solutionParam));

            m_progressItemSolve->emitMessage(QObject::tr("Newton's solver - assemble: %1 s").
                                             arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")), false);
            m_progressItemSolve->emitMessage(QObject::tr("Newton's solver - solve: %1 s").
                                             arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")), false);

            //delete coeff_vec; //TODO nebo se to dela v resici???
        }
        catch(Hermes::Exceptions::Exception e)
        {
            QString error = QString(e.getMsg());
            m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed: ") + error, true);
            return false;
        }
    }

    if (m_fieldInfo->linearityType == LinearityType_Picard)
    {
    }

    return true;

}


template <typename Scalar>
bool SolutionArrayList<Scalar>::performAdaptivityStep(double &error, int stepI, int &actualAdaptivitySteps, int maxAdaptivitySteps)
{
    assert(0);
//    // construct refined spaces
//    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaceReference
//            = smartize(*Hermes::Hermes2D::Space<Scalar>::construct_refined_spaces(desmartize(space)));

//    // solve reference problem
//    if (!solveOneProblem(spaceReference, solutionReference))
//    {
//        isError = true;
//        return false;
//    }

//    // project the fine mesh solution onto the coarse mesh.
//    Hermes::Hermes2D::OGProjection<Scalar>::project_global(desmartize(space), desmartize(solutionReference), desmartize(solution), matrixSolver);

//    // calculate element errors and total error estimate.
//    Hermes::Hermes2D::Adapt<Scalar> adaptivity(desmartize(space), projNormType);

//    // calculate error estimate for each solution component and the total error estimate.
//    error = adaptivity.calc_err_est(desmartize(solution), desmartize(solutionReference)) * 100;

//    // emit signal
//    m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
//                                     arg(error, 0, 'f', 3).
//                                     arg(stepI + 1).
//                                     arg(maxAdaptivitySteps).
//                                     arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(desmartize(space))).
//                                     arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(desmartize(spaceReference))), false, 1);
//    // add error to the list
//    m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(desmartize(space)));

//    if (error < adaptivityTolerance || Hermes::Hermes2D::Space<Scalar>::get_num_dofs(desmartize(space)) >= adaptivityMaxDOFs)
//    {
//        return false;
//    }
//    adaptivity.adapt(selector,
//                     Util::config()->threshold,
//                     Util::config()->strategy,
//                     Util::config()->meshRegularity);

//    actualAdaptivitySteps = stepI + 1;

//    if (m_progressItemSolve->isCanceled())
//    {
//        isError = true;
//        return false;
//    }

//    // delete reference space
//    for (int i = 0; i < spaceReference.size(); i++)
//    {
//        //TODO taky nejak obalit...
//        delete spaceReference.at(i)->get_mesh();
//        //delete spaceReference.at(i);
//    }
//    spaceReference.clear();

//    return true;
}

template <typename Scalar>
void SolutionArrayList<Scalar>::doAdaptivityStep()
{
    assert(0);
//    adaptivitySteps = 1;
//    adaptivityTolerance = 0.0;

//    // forced hp-adaptivity
//    if (adaptivityType == AdaptivityType_None)
//        adaptivityType = AdaptivityType_HP;

//    mesh = new Hermes::Hermes2D::Mesh();
//    mesh->copy(listOfSolutionArrays.at(listOfSolutionArrays.size() - numberOfSolution)->space->get_mesh());
//    for (int i = 0; i < numberOfSolution; i++)
//        space.push_back(shared_ptr<Hermes::Hermes2D::Space<Scalar> >(listOfSolutionArrays.at(listOfSolutionArrays.size() - numberOfSolution + i)->space->dup(mesh)));

//    // create essential boundary conditions and space
//    createSpace();

//    qDebug() << "nodes: " << mesh->get_num_nodes();
//    qDebug() << "elements: " << mesh->get_num_elements();
//    qDebug() << "ndof: " << Hermes::Hermes2D::Space<double>::get_num_dofs(desmartize(space));

//    // create solutions
//    createSolutions(true);

//    // init selectors
//    initSelectors();

//    // check for DOFs
//    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(desmartize(space)) == 0)
//    {
//        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
//        cleanup();
//        return;
//    }

//    double error;
//    int actualAdaptivitySteps;
//    performAdaptivityStep(error, 0, actualAdaptivitySteps, adaptivitySteps);

//    //TODO Jak se ma presne chovat tato funkce?
//    //TODO ma smysl pro transienty?
//    //TODO ma mit clovek moznost navazat na jeden krok adaptivity spustenim celeho procesu?
//    for (int i = 0; i < numberOfSolution; i++)
//        recordSolution(solution.at(i), space.at(i), error, 1, 0);
}


template <typename Scalar>
void SolutionArrayList<Scalar>::solve()
{
    QTime time;

    double error = 0.0;

    // read mesh from file
    readMesh();

    // create essential boundary conditions and space
    createSpace();

    qDebug() << "nodes: " << mesh->get_num_nodes();
    qDebug() << "elements: " << mesh->get_num_elements();
    qDebug() << "ndof: " << Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(desmartize(space)));

    // create solutions
    createSolutions();

    // init selectors
    initSelectors();

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(space))) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
        cleanup();
        return;
    }


    assert(m_fieldInfo->analysisType() == AnalysisType_SteadyState); //transient se dodela
//    for (int i = 0; i <m_fieldInfo->module()->number_of_solution(); i++)
//    {
//        // nonlinear - initial solution
//        // solution.at(i)->set_const(mesh, 0.0);

//        // transient
//        if (m_fieldInfo->analysisType() == AnalysisType_Transient)
//        {
//            // constant initial solution
//            shared_ptr<InitialCondition> initial(new InitialCondition(mesh, initialCondition));
//            recordSolution(initial, space.at(i));
//        }
//    }

    actualTime = 0.0;
    int timesteps = (m_fieldInfo->analysisType() == AnalysisType_Transient) ? floor(m_fieldInfo->timeTotal().number() / m_fieldInfo->timeStep().number()) : 1;
    for (int n = 0; n<timesteps; n++)
    {
        // set actual time
        actualTime = (n + 1) * m_fieldInfo->timeStep().number();

        // update essential bc values
        Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(space), actualTime);
        // update timedep values
        m_fieldInfo->module()->update_time_functions(actualTime);

        m_wf->set_current_time(actualTime);
        //TODO tady predavam reseni v casovych vrstvach... asi by to slo delat jinde/bez toho..
        if (m_fieldInfo->analysisType() == AnalysisType_Transient)
            for (int i = 0; i < solution.size(); i++)
                m_wf->solution.push_back(listOfSolutionArrays.at(listOfSolutionArrays.size() - solution.size() + i)->sln.get() );
        m_wf->delete_all();
        m_wf->registerForms();

        int maxAdaptivitySteps = (m_fieldInfo->adaptivityType == AdaptivityType_None) ? 1 : m_fieldInfo->adaptivitySteps;
        int actualAdaptivitySteps = -1;
        int i = 0;
        do
        {
            if (m_fieldInfo->adaptivityType == AdaptivityType_None)
            {
                if (!solveOneProblem(space, solution))
                    isError = true;
            }
            else
            {
                if(! performAdaptivityStep(error, i, actualAdaptivitySteps, maxAdaptivitySteps))
                    break;
            }

            i++;
        }
        while (i < maxAdaptivitySteps);

        // output
        if (!isError)
        {
            for (int i = 0; i < m_fieldInfo->module()->number_of_solution(); i++)
                recordSolution(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*m_fieldInfo->timeStep().number());

            if (m_fieldInfo->analysisType() == AnalysisType_Transient)
                m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                                 arg(n+1).
                                                 arg(timesteps).
                                                 arg(actualTime, 0, 'e', 2), false, n+2);
        }
        else
            break;
    }

    cleanup();

    if (isError)
    {
        clear();
//        for (int i = 0; i < listOfSolutionArrays.size(); i++)
//            delete listOfSolutionArrays.at(i);
//        listOfSolutionArrays.clear();
    }

}


template <typename Scalar>
void SolutionArrayList<Scalar>::recordSolution(shared_ptr<Hermes::Hermes2D::Solution<Scalar> > sln, shared_ptr<Hermes::Hermes2D::Space<Scalar> > space, double adaptiveError, double adaptiveSteps, double time)
{
    SolutionArray<Scalar> *solution = new SolutionArray<Scalar>();

    assert(sln);
    if (sln->get_type() == Hermes::Hermes2D::HERMES_EXACT)
    {
        solution->sln = sln;
    }
    else
    {
        solution->sln = shared_ptr<Solution<Scalar> >(new Solution<Scalar>());
        solution->sln->copy(sln.get());
    }

    assert(space);
    //solution->space = new Hermes::Hermes2D::Space<Scalar>();
    //solution->space = space->dup(space->get_mesh());
    solution->space = space;


    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    listOfSolutionArrays.push_back(solution);
    printf("record solution, %d\n", size());
}

template class SolutionArrayList<double>;

template class SolutionArray<double>;

