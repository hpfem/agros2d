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

// #include "hermes_general.h"
#include "hermes_electrostatic.h"
#include "hermes_magnetic.h"
#include "hermes_heat.h"
#include "hermes_current.h"
#include "hermes_elasticity.h"
// #include "hermes_flow.h"
#include "hermes_rf.h"
#include "hermes_acoustic.h"
#include "progressdialog.h"

#include "mesh/h2d_reader.h"

#include <dirent.h>

double actualTime;

Hermes::Module::ModuleAgros *moduleFactory(std::string id, ProblemType problem_type, AnalysisType analysis_type)
{
    Hermes::Module::ModuleAgros *module = NULL;

    if (id == "electrostatic")
        module = new ModuleElectrostatic(problem_type, analysis_type);
    if (id == "current")
        module = new ModuleCurrent(problem_type, analysis_type);
    if (id == "heat")
        module = new ModuleHeat(problem_type, analysis_type);
    if (id == "magnetic")
        module = new ModuleMagnetic(problem_type, analysis_type);
    if (id == "acoustic")
        module = new ModuleAcoustic(problem_type, analysis_type);
    if (id == "elasticity")
        module = new ModuleElasticity(problem_type, analysis_type);
    if (id == "rf")
        module = new ModuleRF(problem_type, analysis_type);

    if (module)
        module->read((datadir() + "/modules/" + QString::fromStdString(id) + ".xml").toStdString());
    else
        std::cout << "Module doesn't exists." << std::endl;

    return module;
}

std::map<std::string, std::string> availableModules()
{
    static std::map<std::string, std::string> modules;

    // read modules
    if (modules.size() == 0)
    {
        DIR *dp;
        if ((dp = opendir((datadir() + "/modules").toStdString().c_str())) == NULL)
            error("Modules dir '%s' doesn't exists", (datadir() + "/modules").toStdString().c_str());

        struct dirent *dirp;
        while ((dirp = readdir(dp)) != NULL)
        {
            std::string filename = dirp->d_name;

            // skip current and parent dir
            if (filename == "." || filename == "..")
                continue;

            if (filename.substr(filename.size() - 4, filename.size() - 1) == ".xml")
            {
                // read name
                rapidxml::file<> file_data((datadir().toStdString() + "/modules/" + filename).c_str());

                // parse xml
                rapidxml::xml_document<> doc;
                doc.parse<0>(file_data.data());

                // module name
                modules[filename.substr(0, filename.size() - 4)] =
                        QObject::tr(doc.first_node("module")->first_node("general")->first_attribute("name")->value()).toStdString();
            }
        }
        closedir(dp);
    }

    return modules;
}

// ***********************************************************************************************

Hermes::Module::LocalVariable::Expression::Expression(rapidxml::xml_node<> *node, ProblemType problemType)
{
    read(node, problemType);
}

void Hermes::Module::LocalVariable::Expression::read(rapidxml::xml_node<> *node, ProblemType problemType)
{
    if (problemType == ProblemType_Planar)
    {
        if (node->first_attribute("planar_scalar"))
            scalar = node->first_attribute("planar_scalar")->value();
        if (node->first_attribute("planar_comp_x"))
            comp_x = node->first_attribute("planar_comp_x")->value();
        if (node->first_attribute("planar_comp_y"))
            comp_y = node->first_attribute("planar_comp_y")->value();
    }
    else
    {
        if (node->first_attribute("planar_scalar"))
            scalar = node->first_attribute("axi_scalar")->value();
        if (node->first_attribute("axi_comp_r"))
            comp_x = node->first_attribute("axi_comp_r")->value();
        if (node->first_attribute("axi_comp_z"))
            comp_y = node->first_attribute("axi_comp_z")->value();
    }
}

// ***********************************************************************************************

Hermes::Module::LocalVariable::LocalVariable(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType)
{
    read(node, problemType, analysisType);
}

void Hermes::Module::LocalVariable::read(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType)
{
    id = node->first_attribute("id")->value();
    name = node->first_attribute("name")->value();
    shortname = node->first_attribute("shortname")->value();
    unit = node->first_attribute("unit")->value();

    if (node->first_attribute("scalar"))
        is_scalar = atoi(node->first_attribute("scalar")->value());
    else
        is_scalar = false;

    expression = Expression(node->first_node(analysis_type_tostring(analysisType).c_str()), problemType);
}

// ***********************************************************************************************

Hermes::Module::Integral::Expression::Expression(rapidxml::xml_node<> *node, ProblemType problemType)
{
    read(node, problemType);
}

void Hermes::Module::Integral::Expression::read(rapidxml::xml_node<> *node, ProblemType problemType)
{
    if (problemType == ProblemType_Planar)
    {
        if (node->first_attribute("planar"))
            scalar = node->first_attribute("planar")->value();
    }
    else
    {
        if (node->first_attribute("axi"))
            scalar = node->first_attribute("axi")->value();
    }
}

// ***********************************************************************************************

Hermes::Module::Integral::Integral(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType)
{
    read(node, problemType, analysisType);
}

void Hermes::Module::Integral::read(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType)
{
    id = node->first_attribute("id")->value();
    name = node->first_attribute("name")->value();
    shortname = node->first_attribute("shortname")->value();
    unit = node->first_attribute("unit")->value();

    expression = Expression(node->first_node(analysis_type_tostring(analysisType).c_str()), problemType);
}

// ***********************************************************************************************

Hermes::Module::Module::Module(ProblemType problemType, AnalysisType analysisType)
{
    m_problemType = problemType;
    m_analysisType = analysisType;

    clear();
}

Hermes::Module::Module::~Module()
{
    clear();
}

void Hermes::Module::Module::read(std::string filename)
{
    clear();

    // read name
    // file exists
    if (ifstream(filename.c_str()))
    {
        // save current locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        rapidxml::file<> file_data(filename.c_str());

        // parse document
        rapidxml::xml_document<> doc;
        doc.parse<0>(file_data.data());

        // problems
        rapidxml::xml_node<> *general = doc.first_node("module")->first_node("general");

        id = general->first_attribute("id")->value();
        name = QObject::tr(general->first_attribute("name")->value()).toStdString();
        description = general->first_node("description")->value();

        rapidxml::xml_node<> *properties = general->first_node("properties");
        has_steady_state = atoi(properties->first_attribute("steadystate")->value());
        has_harmonic = atoi(properties->first_attribute("harmonic")->value());
        has_transient = atoi(properties->first_attribute("transient")->value());

        // constants
        rapidxml::xml_node<> *consts = doc.first_node("module")->first_node("constants");
        for (rapidxml::xml_node<> *node = consts->first_node(); node; node = node->next_sibling())
            constants[node->first_attribute("id")->value()] = atof(node->first_attribute("value")->value());

        rapidxml::xml_node<> *vars = doc.first_node("module")->first_node("localvariable");
        for (rapidxml::xml_node<> *node = vars->first_node(); node; node = node->next_sibling())
            variables.push_back(new LocalVariable(node, m_problemType, m_analysisType));

        // read view
        rapidxml::xml_node<> *view = doc.first_node("module")->first_node("view");

        // scalar variables
        rapidxml::xml_node<> *view_scalar_vars = view->first_node("scalarvariable");
        if (view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str()))
        {
            if (view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default"))
                view_default_scalar_variable = get_variable(view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default")->value());

            rapidxml::xml_node<> *node_scalar = view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str());
            for (rapidxml::xml_node<> *node = node_scalar->first_node(); node; node = node->next_sibling())
            {
                LocalVariable *variable = get_variable(node->first_attribute("id")->value());
                if (variable)
                    view_scalar_variables.push_back(variable);
            }
        }

        // vector variables
        rapidxml::xml_node<> *view_vector_vars = view->first_node("vectorvariable");
        if (view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str()))
        {
            if (view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default"))
                view_default_vector_variable = get_variable(view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default")->value());

            rapidxml::xml_node<> *node_vector = view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str());
            for (rapidxml::xml_node<> *node = node_vector->first_node(); node; node = node->next_sibling())
            {
                LocalVariable *variable = get_variable(node->first_attribute("id")->value());
                if (variable)
                    view_vector_variables.push_back(variable);
            }
        }

        // local variables
        rapidxml::xml_node<> *pointvalues = doc.first_node("module")->first_node("pointvalue");

        rapidxml::xml_node<> *node_pointvalue = pointvalues->first_node(analysis_type_tostring(m_analysisType).c_str());
        if (node_pointvalue)
        {
            for (rapidxml::xml_node<> *node = node_pointvalue->first_node(); node; node = node->next_sibling())
            {
                LocalVariable *variable = get_variable(node->first_attribute("id")->value());
                if (variable)
                    local_point.push_back(variable);
            }
        }

        // surface integral
        rapidxml::xml_node<> *node_surface_integral = doc.first_node("module")->first_node("surfaceintegral");
        for (rapidxml::xml_node<> *node = node_surface_integral->first_node(); node; node = node->next_sibling())
            surface_integral.push_back(new Integral(node, m_problemType, m_analysisType));

        // volume integral
        rapidxml::xml_node<> *node_volume_integral = doc.first_node("module")->first_node("volumeintegral");
        for (rapidxml::xml_node<> *node = node_volume_integral->first_node(); node; node = node->next_sibling())
            volume_integral.push_back(new Integral(node, m_problemType, m_analysisType));

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }
}

void Hermes::Module::Module::clear()
{
    // id
    id = "";
    // name
    name = "";
    // description
    description = "";

    has_steady_state = false;
    has_harmonic = false;
    has_transient = false;

    // constants
    constants.clear();

    // variables
    for (Hermes::vector<LocalVariable *>::iterator it = variables.begin(); it < variables.end(); ++it)
        delete *it;
    variables.clear();

    // scalar and vector variables
    view_scalar_variables.clear();
    view_vector_variables.clear();
    // default variables
    view_default_scalar_variable = NULL;
    view_default_vector_variable = NULL;

    // local variables
    local_point.clear();

    // surface integrals
    surface_integral.clear();

    // volume integrals
    volume_integral.clear();
}

Hermes::Module::LocalVariable* Hermes::Module::Module::get_variable(std::string id)
{
    for (Hermes::vector<LocalVariable *>::iterator it = variables.begin(); it < variables.end(); ++it)
    {
        if (((LocalVariable *) *it)->id == id)
            return *it;
    }
    return NULL;
}

mu::Parser *Hermes::Module::Module::get_parser()
{
    mu::Parser *parser = new mu::Parser();

    // pi
    parser->DefineConst("PI", M_PI);

    // frequency
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        parser->DefineConst("f", Util::scene()->problemInfo()->frequency);

    for (std::map<std::string, double>::iterator it = constants.begin(); it != constants.end(); ++it)
        parser->DefineConst(it->first, it->second);

    return parser;
}

std::string Hermes::Module::Module::get_expression(Hermes::Module::LocalVariable *physicFieldVariable,
                                                   PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    // case PhysicFieldVariableComp_Undefined:
    //    return "";
    case PhysicFieldVariableComp_Scalar:
        return physicFieldVariable->expression.scalar;
    case PhysicFieldVariableComp_X:
        return physicFieldVariable->expression.comp_x;
    case PhysicFieldVariableComp_Y:
        return physicFieldVariable->expression.comp_y;
    case PhysicFieldVariableComp_Magnitude:
        return "sqrt((" + physicFieldVariable->expression.comp_x + ") * (" + physicFieldVariable->expression.comp_x + ") + (" + physicFieldVariable->expression.comp_y + ") * (" + physicFieldVariable->expression.comp_y + "))";
    default:
        error("Unknown type.");
    }
}

// ***********************************************************************************************

void Hermes::Module::ModuleAgros::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, view_scalar_variables);
}

void Hermes::Module::ModuleAgros::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, view_vector_variables);
}

void Hermes::Module::ModuleAgros::fillComboBox(QComboBox *cmbFieldVariable, Hermes::vector<LocalVariable *> list)
{
    for(Hermes::vector<LocalVariable *>::iterator it = list.begin(); it < list.end(); ++it )
    {
        LocalVariable *variable = ((LocalVariable *) *it);
        cmbFieldVariable->addItem(QString::fromStdString(variable->name),
                                  QString::fromStdString(variable->id));
    }
}

// ***********************************************************************************************

void readMeshDirtyFix()
{
    // fix precalulating matrices for mapping of curved elements

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    std::ostringstream os;
    os << "vertices =" << std::endl <<
          "{" << std::endl <<
          "{ 0, 0 }," << std::endl <<
          "{ 1, 0 }," << std::endl <<
          "{ 0, 1 }" << std::endl <<
          "}" << std::endl <<
          "elements =" << std::endl <<
          "{" << std::endl <<
          "{ 0, 1, 2, 0 }" << std::endl <<
          "}" << std::endl <<
          "boundaries =" << std::endl <<
          "{" << std::endl <<
          "{ 0, 1, 1 }," << std::endl <<
          "{ 1, 2, 1 }," << std::endl <<
          "{ 2, 0, 1 }" << std::endl <<
          "}" << std::endl <<
          "curves =" << std::endl <<
          "{" << std::endl <<
          "{ 1, 2, 90 }" << std::endl <<
          "}";

    Mesh mesh;
    H2DReader meshloader;
    meshloader.load_str(os.str().c_str(), &mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

Mesh *readMeshFromFile(const QString &fileName)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh *mesh = new Mesh();
    H2DReader meshloader;
    meshloader.load(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return mesh;
}

void writeMeshFromFile(const QString &fileName, Mesh *mesh)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    H2DReader meshloader;
    meshloader.save(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void refineMesh(Mesh *mesh, bool refineGlobal, bool refineTowardsEdge)
{
    // refine mesh - global
    if (refineGlobal)
        for (int i = 0; i < Util::scene()->problemInfo()->numberOfRefinements; i++)
            mesh->refine_all_elements(0);

    // refine mesh - boundary
    if (refineTowardsEdge)
        for (int i = 0; i < Util::scene()->edges.count(); i++)
            if (Util::scene()->edges[i]->refineTowardsEdge > 0)
                mesh->refine_towards_boundary(QString::number(((Util::scene()->edges[i]->boundary->type != PhysicFieldBC_None) ? i + 1 : -i)).toStdString(),
                                              Util::scene()->edges[i]->refineTowardsEdge);
}

// return geom type
GeomType convertProblemType(ProblemType problemType)
{
    return (problemType == ProblemType_Planar ? HERMES_PLANAR : HERMES_AXISYM_Y);
}

Hermes::vector<SolutionArray *> solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                                  Hermes::vector<EssentialBCs> bcs,
                                                  WeakFormAgros *wf)
{
    SolutionAgros solutionAgros(progressItemSolve, wf);

    Hermes::vector<SolutionArray *> solutionArrayList = solutionAgros.solveSolutioArray(bcs);
    return solutionArrayList;
}

// *********************************************************************************************************************************************

SolutionAgros::SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros *wf)
{
    analysisType = Util::scene()->problemInfo()->analysisType;
    polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;
    adaptivityMaxDOFs = Util::scene()->problemInfo()->adaptivityMaxDOFs;
    numberOfSolution = Util::scene()->problemInfo()->module()->number_of_solution();
    timeTotal = Util::scene()->problemInfo()->timeTotal.number;
    timeStep = Util::scene()->problemInfo()->timeStep.number;
    initialCondition = Util::scene()->problemInfo()->initialCondition.number;

    linearityType = Util::scene()->problemInfo()->linearityType;
    linearityNonlinearTolerance = Util::scene()->problemInfo()->linearityNonlinearTolerance;
    linearityNonlinearSteps = Util::scene()->problemInfo()->linearityNonlinearSteps;

    matrixSolver = Util::scene()->problemInfo()->matrixSolver;

    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
}

Hermes::vector<SolutionArray *> SolutionAgros::solveSolutioArray(Hermes::vector<EssentialBCs> bcs)
{
    QTime time;

    // solution agros array
    Hermes::vector<SolutionArray *> solutionArrayList;

    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    refineMesh(mesh, true, true);

    // create an H1 space
    Hermes::vector<Space *> space;
    // create hermes solution array
    Hermes::vector<Solution *> solution;
    // create reference solution
    Hermes::vector<Solution *> solutionReference;

    // projection norms
    Hermes::vector<ProjNormType> projNormType;

    // prepare selector
    Hermes::vector<RefinementSelectors::Selector *> selector;

    // error marker
    bool isError = false;

    RefinementSelectors::Selector *select = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        select = new RefinementSelectors::HOnlySelector();
        break;
    case AdaptivityType_P:
        select = new RefinementSelectors::H1ProjBasedSelector(RefinementSelectors::H2D_P_ANISO,
                                                              Util::config()->convExp,
                                                              H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new RefinementSelectors::H1ProjBasedSelector(RefinementSelectors::H2D_HP_ANISO,
                                                              Util::config()->convExp,
                                                              H2DRS_DEFAULT_ORDER);
        break;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new H1Space(mesh, &bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());

        // solution agros array
        solution.push_back(new Solution());

        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
            // reference solution
            solutionReference.push_back(new Solution());
        }
    }

    // check for DOFs
    if (Space::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
    }
    else
    {
        for (int i = 0; i < numberOfSolution; i++)
        {
            // transient
            if (analysisType == AnalysisType_Transient)
            {
                // constant initial solution
                solution.at(i)->set_const(mesh, initialCondition);
                solutionArrayList.push_back(solutionArray(solution.at(i)));
            }

            // nonlinear
            if ((linearityType != LinearityType_Linear) && (analysisType != AnalysisType_Transient))
            {
                solution.at(i)->set_const(mesh, 0.0);
            }
        }

        actualTime = 0.0;

        // update time function
        Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

        m_wf->set_current_time(actualTime);
        m_wf->solution = solution;
        m_wf->delete_all();
        m_wf->registerForms();

        // emit message
        if (adaptivityType != AdaptivityType_None)
            m_progressItemSolve->emitMessage(QObject::tr("Adaptivity type: %1").arg(adaptivityTypeString(adaptivityType)), false);

        double error = 0.0;

        // solution
        int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
        int actualAdaptivitySteps = -1;
        for (int i = 0; i<maxAdaptivitySteps; i++)
        {
            // set up the solver, matrix, and rhs according to the solver selection.
            SparseMatrix *matrix = create_matrix(matrixSolver);
            Vector *rhs = create_vector(matrixSolver);
            Solver *solver = create_linear_solver(matrixSolver, matrix, rhs);

            if (adaptivityType == AdaptivityType_None)
            {
                if (analysisType != AnalysisType_Transient)
                    solve(space, solution, solver, matrix, rhs);
            }
            else
            {
                // construct globally refined reference mesh and setup reference space.
                Hermes::vector<Space *> spaceReference = *Space::construct_refined_spaces(space);

                // assemble reference problem.
                solve(spaceReference, solutionReference, solver, matrix, rhs);

                if (!isError)
                {
                    // project the fine mesh solution onto the coarse mesh.
                    OGProjection::project_global(space, solutionReference, solution, matrixSolver);

                    // Calculate element errors and total error estimate.
                    Adapt adaptivity(space, projNormType);

                    // Calculate error estimate for each solution component and the total error estimate.
                    error = adaptivity.calc_err_est(solution,
                                                    solutionReference) * 100;

                    // emit signal
                    m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
                                                     arg(error, 0, 'f', 3).
                                                     arg(i + 1).
                                                     arg(maxAdaptivitySteps).
                                                     arg(Space::get_num_dofs(space)).
                                                     arg(Space::get_num_dofs(spaceReference)), false, 1);
                    // add error to the list
                    m_progressItemSolve->addAdaptivityError(error, Space::get_num_dofs(space));

                    if (error < adaptivityTolerance || Space::get_num_dofs(space) >= adaptivityMaxDOFs)
                    {
                        break;
                    }
                    if (i != maxAdaptivitySteps-1) adaptivity.adapt(selector,
                                                                    Util::config()->threshold,
                                                                    Util::config()->strategy,
                                                                    Util::config()->meshRegularity);
                    actualAdaptivitySteps = i+1;
                }

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

            // clean up.
            delete solver;
            delete matrix;
            delete rhs;
        }

        // delete reference solution
        for (int i = 0; i < solutionReference.size(); i++)
            delete solutionReference.at(i);
        solutionReference.clear();

        // delete selector
        if (select) delete select;
        selector.clear();

        // timesteps
        if (!isError)
        {
            SparseMatrix *matrix = NULL;
            Vector *rhs = NULL;
            Solver *solver = NULL;

            // allocate dp for transient solution
            DiscreteProblem *dpTran = NULL;
            if (analysisType == AnalysisType_Transient)
            {
                // set up the solver, matrix, and rhs according to the solver selection.
                matrix = create_matrix(matrixSolver);
                rhs = create_vector(matrixSolver);
                solver = create_linear_solver(matrixSolver, matrix, rhs);
                // solver->set_factorization_scheme(HERMES_REUSE_FACTORIZATION_COMPLETELY);

                dpTran = new DiscreteProblem(m_wf, space, true);
            }

            int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
            for (int n = 0; n<timesteps; n++)
            {
                // set actual time
                actualTime = (n+1)*timeStep;

                // update essential bc values
                Space::update_essential_bc_values(space, actualTime);
                // update timedep values
                Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

                m_wf->set_current_time(actualTime);
                m_wf->delete_all();
                m_wf->registerForms();

                // transient
                if ((timesteps > 1) && (linearityType == LinearityType_Linear))
                    isError = !solveLinear(dpTran, space, solution,
                                           solver, matrix, rhs);
                if ((timesteps > 1) && (linearityType != LinearityType_Linear))
                    isError = !solve(space, solution,
                                     solver, matrix, rhs);

                // output
                for (int i = 0; i < numberOfSolution; i++)
                {
                    solutionArrayList.push_back(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));
                }

                if (analysisType == AnalysisType_Transient)
                    m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                                     arg(n+1).
                                                     arg(timesteps).
                                                     arg(actualTime, 0, 'e', 2), false, n+2);
                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }
            }

            // clean up
            if (solver) delete solver;
            if (matrix) delete matrix;
            if (rhs) delete rhs;

            if (dpTran) delete dpTran;
        }
    }
    // delete mesh
    delete mesh;

    // delete space
    for (unsigned int i = 0; i < space.size(); i++)
    {
        // delete space.at(i)->get_mesh();
        delete space.at(i);
    }
    space.clear();

    // delete last solution
    for (unsigned int i = 0; i < solution.size(); i++)
        delete solution.at(i);
    solution.clear();

    if (isError)
    {
        for (int i = 0; i < solutionArrayList.size(); i++)
            delete solutionArrayList.at(i);
        solutionArrayList.clear();
    }
    return solutionArrayList;
}

bool SolutionAgros::solveLinear(DiscreteProblem *dp,
                                Hermes::vector<Space *> space,
                                Hermes::vector<Solution *> solution,
                                Solver *solver, SparseMatrix *matrix, Vector *rhs)
{
    // QTime time;
    // time.start();
    dp->assemble(matrix, rhs);
    // qDebug() << "assemble: " << time.elapsed();

    if(solver->solve())
    {
        Solution::vector_to_solutions(solver->get_solution(), space, solution);
        return true;
    }
    else
    {
        m_progressItemSolve->emitMessage(QObject::tr("Matrix solver failed."), true, 1);
        return false;
    }
}

bool SolutionAgros::solve(Hermes::vector<Space *> space,
                          Hermes::vector<Solution *> solution,
                          Solver *solver, SparseMatrix *matrix, Vector *rhs)
{
    bool isError = false;
    if (linearityType == LinearityType_Linear)
    {
        DiscreteProblem dpLin(m_wf, space, true);

        isError = !solveLinear(&dpLin, space, solution,
                               solver, matrix, rhs);

        return !isError;
    }

    if (linearityType == LinearityType_Picard)
    {
        /*
        DiscreteProblem dpNonlinPicard(m_wf, space, true);

        // create Picard solution
        Hermes::vector<Solution *> solutionPicard;
        for (int i = 0; i < numberOfSolution; i++)
            solutionPicard.push_back(new Solution());

        // perform the Picard's iteration
        for (int i = 0; i < linearityNonlinearSteps; i++)
        {
            isError = !solveLinear(&dpNonlinPicard, space, solutionPicard,
                                   solver, matrix, rhs, false);

            ProjNormType projNormTypeTMP = HERMES_H1_NORM;
            // ProjNormType projNormTypeTMP = HERMES_L2_NORM;

            // calc error
            double *val_sol, *val_solpic, *val_diff;
            double error = 0.0;
            for (int i = 0; i < numberOfSolution; i++)
            {
                error += calc_abs_error(solution.at(i), solutionPicard.at(i), projNormTypeTMP) * 100.0
                        / calc_norm(&sln_new, HERMES_H1_NORM) * 100;

                val_sol = solution.at(i)->get_fn_values();
                val_solpic = solutionPicard.at(i)->get_fn_values();

                val_diff = new double[Space::get_num_dofs(space)];
                for (int j = 0; j < Space::get_num_dofs(space); j++)
                    val_diff[j] = val_solpic[j]; //  + 0.5 * (val_sol[j] - val_solpic[j]);
            }

            // emit signal
            m_progressItemSolve->emitMessage(QObject::tr("Picards method rel. error (%2/%3): %1%").
                                             arg(error, 0, 'f', 5).
                                             arg(i + 1).
                                             arg(linearityNonlinearSteps), false, 1);

            // add error to the list
            m_progressItemSolve->addNonlinearityError(error);

            if (error < linearityNonlinearTolerance)
            {
                // FIXME - clean up
                break;
            }

            // copy solution
            for (int i = 0; i < numberOfSolution; i++)
            {
                // if (error > 100.0)
                //     solution.at(i)->multiply(0.5);
                // else
                solution.at(i)->copy(solutionPicard.at(i));
            }
            // Solution::vector_to_solutions(val_solpic, space, solution);
            // Solution::vector_to_solutions(solutionPicard., space, solution);
        }

        for (int i = 0; i < solutionPicard.size(); i++)
            delete solutionPicard.at(i);
        solutionPicard.clear();

        return !isError;
        */
    }

    if (linearityType == LinearityType_Newton)
    {
        /*
        // project the initial condition on the FE space to obtain initial
        // coefficient vector for the Newton's method.
        info("Projecting to obtain initial vector for the Newton's method.");
        double *coeff_vec = new double[Space::get_num_dofs(space)];
        OGProjection::project_global(space, solution->value(), coeff_vec,
                                     Util::scene()->problemInfo()->matrixSolver);

        DiscreteProblem dpNonlinNewton(wf, space, false);

        // The Newton's loop.
        double damping_coeff = 1.0;

        // perform the Picard's iteration
        for (int i = 0; i < linearityNonlinearSteps; i++)
        {
            // assemble the Jacobian matrix and residual vector.
            dpNonlinNewton.assemble(coeff_vec, matrix, rhs, false);

            // Multiply the residual vector with -1 since the matrix
            // equation reads J(Y^n) \deltaY^{n+1} = -F(Y^n).
            rhs->change_sign();

            // Calculate the l2-norm of residual vector
            double error = get_l2_norm(rhs);

            // emit signal
            m_progressItemSolve->emitMessage(QObject::tr("Newton’s method rel. error (%2/%3): %1")
                                             .arg(error, 0, 'f', 5)
                                             .arg(i)
                                             .arg(linearityNonlinearSteps), false, 1);

            // add error to the list
            m_progressItemSolve->addNonlinearityError(error);

            // if residual norm is within tolerance, or the maximum number of iteration has been reached, then quit.
            if (error < linearityNonlinearTolerance)
                break;

            // Solve the linear system.
            isError = solver->solve();
            if (!isError)
                error("Matrix solver failed.\n");

            // add \deltaY^{n+1} to Y^n.
            for (int j = 0; j < Space::get_num_dofs(space); j++)
                coeff_vec[j] += damping_coeff * solver->get_solution()[j];
        }

        Solution::vector_to_solutions(coeff_vec, space, solution);

        delete [] coeff_vec;
        */
    }
}

SolutionArray *SolutionAgros::solutionArray(Solution *sln, Space *space, double adaptiveError, double adaptiveSteps, double time)
{
    SolutionArray *solution = new SolutionArray();
    solution->order = new Orderizer();
    if (space) solution->order->process_space(space);
    solution->sln = new Solution();
    if (sln) solution->sln->copy(sln);
    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    return solution;
}

// *********************************************************************************************************************************************

Parser::~Parser()
{
    // delete parser
    for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
        delete *it;
    parser.clear();
}

// *********************************************************************************************************************************************

ViewScalarFilter::ViewScalarFilter(Hermes::vector<MeshFunction *> sln)
    : Filter(sln)
{
}

ViewScalarFilter::~ViewScalarFilter()
{
    delete parser;

    delete [] pvalue;
    delete [] pdx;
    delete [] pdy;
}

void ViewScalarFilter::initParser(std::string expression)
{
    mu::Parser *pars = Util::scene()->problemInfo()->module()->get_parser();

    pars->SetExpr(expression);

    pars->DefineVar("x", &px);
    pars->DefineVar("y", &py);

    pvalue = new double[num];
    pdx = new double[num];
    pdy = new double[num];

    for (int k = 0; k < num; k++)
    {
        std::stringstream number;
        number << (k+1);

        pars->DefineVar("value" + number.str(), &pvalue[k]);
        pars->DefineVar("dx" + number.str(), &pdx[k]);
        pars->DefineVar("dy" + number.str(), &pdy[k]);
    }

    parser->parser.push_back(pars);
}

double ViewScalarFilter::get_pt_value(double x, double y, int item)
{
    return 0.0;
}

void ViewScalarFilter::precalculate(int order, int mask)
{
    Quad2D* quad = quads[cur_quad];
    int np = quad->get_num_points(order);
    node = new_node(H2D_FN_DEFAULT, np);

    double **value = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudx = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudy = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];

    for (int k = 0; k < num; k++)
    {
        sln[k]->set_quad_order(order, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
        sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
        value[k] = sln[k]->get_fn_values();
    }

    update_refmap();

    double *x = refmap->get_phys_x(order);
    double *y = refmap->get_phys_y(order);
    Element *e = refmap->get_active_element();

    SceneMaterial *material = Util::scene()->labels[atoi(mesh->get_element_markers_conversion().get_user_marker(e->marker).c_str())]->material;
    parser->setParserVariables(material);

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < num; k++)
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }

        // parse expression
        try
        {
            node->values[0][0][i] = parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << e.GetMsg() << endl;
        }
    }

    delete [] value;
    delete [] dudx;
    delete [] dudy;

    if (nodes->present(order))
    {
        assert(nodes->get(order) == cur_node);
        ::free(nodes->get(order));
    }
    nodes->add(node, order);
    cur_node = node;
}
