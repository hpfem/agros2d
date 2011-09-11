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

#include "module.h"
#include "solver.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenesolution.h"
#include "progressdialog.h"
#include "weakform_parser.h"

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray()
{
    logMessage("SolutionArray::SolutionArray()");

    sln = NULL;
    space = NULL;

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

template <typename Scalar>
SolutionArray<Scalar>::~SolutionArray()
{
    logMessage("SolutionArray::~SolutionArray()");

    if (space)
    {
        delete space;
        space = NULL;
    }

    if (sln)
    {
        // delete sln;
        sln = NULL;
    }
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

    sln = new Hermes::Hermes2D::Solution<Scalar>();
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
SolverAgros<Scalar>::SolverAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf)
{
    analysisType = Util::scene()->problemInfo()->analysisType;
    polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;
    adaptivityMaxDOFs = Util::config()->maxDofs;
    numberOfSolution = Util::scene()->problemInfo()->module()->number_of_solution();
    timeTotal = Util::scene()->problemInfo()->timeTotal.number();
    timeStep = Util::scene()->problemInfo()->timeStep.number();
    initialCondition = Util::scene()->problemInfo()->initialCondition.number();

    nonlinearTolerance = Util::scene()->problemInfo()->nonlinearTolerance;
    nonlinearSteps = Util::scene()->problemInfo()->nonlinearSteps;

    matrixSolver = Util::scene()->problemInfo()->matrixSolver;

    weakFormsType = Util::scene()->problemInfo()->weakFormsType;

    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
    isError = false;
}

template <typename Scalar>
void SolverAgros<Scalar>::readMesh()
{
    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".xml");
    refineMesh(mesh, true, true);
}


template <typename Scalar>
void SolverAgros<Scalar>::createSpace()
{
    // essential boundary conditions
    Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> *> bcs; //TODO PK <double>
    for (int i = 0; i < numberOfSolution; i++)
        bcs.push_back(new Hermes::Hermes2D::EssentialBCs<double>());  //TODO PK <double>

    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary && boundary != Util::scene()->boundaries[0])
        {
            Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(boundary->type);

            for (Hermes::vector<ParserFormEssential *>::iterator it = boundary_type->essential.begin();
                 it < boundary_type->essential.end(); ++it)
            {
                ParserFormEssential *form = ((ParserFormEssential *) *it);

                Hermes::Hermes2D::EssentialBoundaryCondition<Scalar> *custom_form = NULL;

                // compiled form
                if (weakFormsType == WeakFormsType_Compiled)
                {
                    /*
                custom_form = factoryMatrixFormEssential<Scalar>(problemId,
                                                            form->i - 1, form->j - 1,
                                                            QString::number(i + 1).toStdString(),
                                                            boundary);
                */
                }

                if (!custom_form && weakFormsType == WeakFormsType_Compiled)
                    qDebug() << "Cannot find compiled VectorFormEssential().";

                // interpreted form
                if (!custom_form || weakFormsType == WeakFormsType_Interpreted)
                {
                    /*
                if (form->expression == "")
                {
                    custom_form = new Hermes::Hermes2D::DefaultEssentialBCConst<double>(form->i - 1,
                                                                                        boundary->values[it->second->id].number());
                }
                else
                */
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
    }

    // create space
    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new Hermes::Hermes2D::H1Space<Scalar>(mesh, bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());
    }

}

template <typename Scalar>
void SolverAgros<Scalar>::initSelectors()
{
    // set adaptivity selector
    select = NULL;
    switch (adaptivityType)
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
    for (int i = 0; i < numberOfSolution; i++)
    {
        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
        }
    }
}

template <typename Scalar>
void SolverAgros<Scalar>::cleanup()
{
    // delete mesh
    //delete mesh;

    // clear space vector (space is deleted in SolutionArray)
    space.clear();

    // delete last solution
    for (unsigned int i = 0; i < solution.size(); i++)
        delete solution.at(i);
    solution.clear();

    // delete reference solution
    for (int i = 0; i < solutionReference.size(); i++)
        delete solutionReference.at(i);
    solutionReference.clear();

    // delete selector
    if (select)
        delete select;
    selector.clear();
}

template <typename Scalar>
bool SolverAgros<Scalar>::solveOneProblem(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> &spaceParam,
                                          Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> &solutionParam)
{
    // Initialize the FE problem.
    Hermes::Hermes2D::DiscreteProblem<double> dp(m_wf, spaceParam);

    int ndof = Hermes::Hermes2D::Space<double>::get_num_dofs(spaceParam);

    // Initial coefficient vector for the Newton's method.
    double* coeff_vec = new double[ndof];
    memset(coeff_vec, 0, ndof*sizeof(double));

    // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
    Hermes::Hermes2D::NewtonSolver<double> newton(&dp, Hermes::SOLVER_UMFPACK);
    if (!newton.solve(coeff_vec, nonlinearTolerance, nonlinearSteps))
    {
        m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed"), true);
        return false;
    }
    else
    {
        Hermes::Hermes2D::Solution<double>::vector_to_solutions(newton.get_sln_vector(), spaceParam, solutionParam);

        m_progressItemSolve->emitMessage(QObject::tr("Newton's solver - assemble: %1 s").
                                         arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")), false);
        m_progressItemSolve->emitMessage(QObject::tr("Newton's solver - solve: %1 s").
                                         arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")), false);
    }

    //delete coeff_vec; //TODO nebo se to dela v resici???

    return true;
}

template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> SolverAgros<Scalar>::solve(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceParam,
                                                                   Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionParam)
{
    QTime time;

    // solution agros array
    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    double error = 0.0;

    // new solution
    if (spaceParam.empty())
    {
        // read mesh from file
        readMesh();

        // create essential boundary conditions and space
        createSpace();
    }
    else
    {
        // forced one step
        adaptivitySteps = 1;
        adaptivityTolerance = 0.0;
        // forced hp-adaptivity
        if (adaptivityType == AdaptivityType_None)
            adaptivityType = AdaptivityType_HP;

        space = spaceParam;
        mesh = space[0]->get_mesh();
    }

    qDebug() << "nodes: " << mesh->get_num_nodes();
    qDebug() << "elements: " << mesh->get_num_elements();
    qDebug() << "ndof: " << Hermes::Hermes2D::Space<double>::get_num_dofs(space);

    // create solutions
    for (int i = 0; i < numberOfSolution; i++)
    {
        // solution agros array
        if (solutionParam.empty())
            solution.push_back(new Hermes::Hermes2D::Solution<double>);
        else
            solution.push_back(solutionParam.at(i));

        if (adaptivityType != AdaptivityType_None)
        {
            // reference solution
            solutionReference.push_back(new Hermes::Hermes2D::Solution<Scalar>());
        }
    }

    // init selectors
    initSelectors();

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
        cleanup();
        return solutionArrayList;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        // nonlinear - initial solution
        // solution.at(i)->set_const(mesh, 0.0);

        // transient
        if (analysisType == AnalysisType_Transient)
        {
            // constant initial solution
            InitialCondition *initial = new InitialCondition(mesh, initialCondition);
            solutionArrayList.push_back(solutionArray(initial, space.at(i)));
        }
    }

    actualTime = 0.0;
    int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
    for (int n = 0; n<timesteps; n++)
    {
        // set actual time
        actualTime = (n + 1) * timeStep;

        // update essential bc values
        Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(space, actualTime);
        // update timedep values
        Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

        m_wf->set_current_time(actualTime);
        if (analysisType == AnalysisType_Transient)
            for (int i = 0; i < solution.size(); i++)
                m_wf->solution.push_back(solutionArrayList.at(solutionArrayList.size() - solution.size() + i)->sln );
        m_wf->delete_all();
        m_wf->registerForms();

        int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
        int actualAdaptivitySteps = -1;
        for (int i = 0; i <= maxAdaptivitySteps; i++)
        {
            if (adaptivityType == AdaptivityType_None)
            {
                if (!solveOneProblem(space, solution))
                    isError = true;
            }
            else
            {
                Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceReference
                        = *Hermes::Hermes2D::Space<Scalar>::construct_refined_spaces(space);

                if (!solveOneProblem(spaceReference, solutionReference))
                {
                    isError = true;
                    break;
                }

                // project the fine mesh solution onto the coarse mesh.
                Hermes::Hermes2D::OGProjection<Scalar>::project_global(space, solutionReference, solution, matrixSolver);

                // Calculate element errors and total error estimate.
                Hermes::Hermes2D::Adapt<Scalar> adaptivity(space, projNormType);

                // Calculate error estimate for each solution component and the total error estimate.
                error = adaptivity.calc_err_est(solution, solutionReference) * 100;

                // emit signal
                m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
                                                 arg(error, 0, 'f', 3).
                                                 arg(i).
                                                 arg(maxAdaptivitySteps).
                                                 arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space)).
                                                 arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceReference)), false, 1);
                // add error to the list
                m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space));

                if (error < adaptivityTolerance || Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) >= adaptivityMaxDOFs)
                {
                    break;
                }
                if (i != maxAdaptivitySteps-1) adaptivity.adapt(selector,
                                                                Util::config()->threshold,
                                                                Util::config()->strategy,
                                                                Util::config()->meshRegularity);
                actualAdaptivitySteps = i;

                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }

                // delete reference space
                for (int i = 0; i < spaceReference.size(); i++)
                {
                    delete spaceReference.at(i)->get_mesh();
                    delete spaceReference.at(i);
                }
                spaceReference.clear();
            }
        }

        // output
        if (!isError)
        {
            for (int i = 0; i < numberOfSolution; i++)
                solutionArrayList.push_back(solutionArray(solution.at(i), space.at(i),
                                                          error, actualAdaptivitySteps, (n+1)*timeStep));

            if (analysisType == AnalysisType_Transient)
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
        for (int i = 0; i < solutionArrayList.size(); i++)
            delete solutionArrayList.at(i);
        solutionArrayList.clear();
    }

    return solutionArrayList;
}


template <typename Scalar>
SolutionArray<Scalar> *SolverAgros<Scalar>::solutionArray(Hermes::Hermes2D::Solution<Scalar> *sln, Hermes::Hermes2D::Space<Scalar> *space, double adaptiveError, double adaptiveSteps, double time)
{
    SolutionArray<Scalar> *solution = new SolutionArray<Scalar>();

    assert(sln);
    if (sln->get_type() == Hermes::Hermes2D::HERMES_EXACT)
    {
        solution->sln = sln;
    }
    else
    {
        solution->sln = new Hermes::Hermes2D::Solution<Scalar>();
        solution->sln->copy(sln);
    }

    assert(space);
    //solution->space = new Hermes::Hermes2D::Space<Scalar>();
    solution->space = space->dup(space->get_mesh());


    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    return solution;
}

template class SolverAgros<double>;

template class SolutionArray<double>;

