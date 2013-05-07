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

#include "field.h"
#include "util/global.h"

#include "problem.h"
#include "hermes2d/problem_config.h"
#include "scene.h"
#include "scenemarker.h"
#include "module.h"
#include "plugin_interface.h"
#include "logview.h"

#include "../../resources_source/classes/module_xml.h"
#include "../../resources_source/classes/problem_a2d_30_xml.h"

Field::Field(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

bool Field::solveInitVariables()
{
    if (!Agros2D::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if (!Agros2D::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

FieldInfo::FieldInfo(QString fieldId, const AnalysisType analysisType)
    : m_plugin(NULL), m_numberOfSolutions(0)
{
    if (fieldId.isEmpty())
        m_fieldId = "electrostatic";
    else
        m_fieldId = fieldId;

    // read plugin
    try{
        m_plugin = Agros2D::loadPlugin(m_fieldId);
    }
    catch (AgrosPluginException &e)
    {
        Agros2D::log()->printError("Solver", "Cannot load plugin");
        throw;
    }

    assert(m_plugin);

    setStringKeys();
    setDefaultValues();

    clear();

    // default analysis
    setAnalysisType(analyses().begin().key());

}

FieldInfo::~FieldInfo()
{
    delete m_plugin;
}

void FieldInfo::setInitialMesh(MeshSharedPtr mesh)
{
    clearInitialMesh();
    m_initialMesh = mesh;
}

void FieldInfo::setAnalysisType(AnalysisType at)
{
    m_analysisType = at;

    foreach (XMLModule::analysis an, m_plugin->module()->general().analyses().analysis())
    {
        if (an.type() == analysisTypeToStringKey(at).toStdString())
        {
            m_numberOfSolutions = an.solutions();

            if (an.field_config().present())
            {
                for (int i = 0; i < an.field_config().get().field_item().size(); i ++)
                {
                    Type key = stringKeyToType(QString::fromStdString(an.field_config().get().field_item().at(i).field_key()));

                    if (m_settingDefault.keys().contains(key))
                    {
                        if (m_settingDefault[key].type() == QVariant::Double)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toDouble();
                        else if (m_settingDefault[key].type() == QVariant::Int)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toInt();
                        else if (m_settingDefault[key].type() == QVariant::Bool)
                            m_settingDefault[key] = (QString::fromStdString(an.field_config().get().field_item().at(i).field_value()) == "1");
                        else if (m_settingDefault[key].type() == QVariant::String)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                        else if (m_settingDefault[key].type() == QVariant::StringList)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).split("|");
                        else
                            qDebug() << "Key not found" << QString::fromStdString(an.field_config().get().field_item().at(i).field_key()) << QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                    }
                }
            }

            m_availableLinearityTypes = availableLinearityTypes(at);
        }
    }
}

QList<LinearityType> FieldInfo::availableLinearityTypes(AnalysisType at) const
{
    QList<LinearityType> availableLinearityTypes;
    foreach (XMLModule::analysis an, m_plugin->module()->general().analyses().analysis())
    {
        if (an.type() == analysisTypeToStringKey(at).toStdString())
        {
            if (an.linearity_config().present())
            {
                for (int i = 0; i < an.linearity_config().get().linearity_item().size(); i ++)
                {
                    QString linearityTypeStr = QString::fromStdString(an.linearity_config().get().linearity_item().at(i).linearity_option());
                    LinearityType linearityType = linearityTypeFromStringKey(linearityTypeStr);
                    availableLinearityTypes.push_back(linearityType);
                }
            }
            else
            {
                availableLinearityTypes.push_back(LinearityType_Linear);
                availableLinearityTypes.push_back(LinearityType_Newton);
            }
        }
    }
    return availableLinearityTypes;
}


int FieldInfo::edgeRefinement(SceneEdge *edge)
{
    QMapIterator<SceneEdge *, int> i(m_edgesRefinement);
    while (i.hasNext()) {
        i.next();
        if (i.key() == edge)
            return i.value();
    }

    return 0;
}

int FieldInfo::labelRefinement(SceneLabel *label)
{
    QMapIterator<SceneLabel *, int> i(m_labelsRefinement);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return 0;
}


int FieldInfo::labelPolynomialOrder(SceneLabel *label)
{
    QMapIterator<SceneLabel *, int> i(m_labelsPolynomialOrder);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return value(FieldInfo::SpacePolynomialOrder).toInt();
}

void FieldInfo::clear()
{
    // mesh
    clearInitialMesh();

    // set default values and types
    m_setting.clear();
    setDefaultValues();

    m_setting = m_settingDefault;

    m_edgesRefinement.clear();
    m_labelsRefinement.clear();
    m_labelsPolynomialOrder.clear();

    m_analysisType = AnalysisType_Undefined;
    m_linearityType = LinearityType_Linear;
    m_adaptivityType = AdaptivityType_None;
}

void FieldInfo::refineMesh(MeshSharedPtr mesh, bool refineGlobal, bool refineTowardsEdge, bool refineArea)
{
    // refine mesh - global
    if (refineGlobal)
        for (int i = 0; i < value(FieldInfo::SpaceNumberOfRefinements).toInt(); i++)
            mesh->refine_all_elements(0);

    // refine mesh - boundary
    if (refineTowardsEdge)
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            if (edgeRefinement(edge) > 0)
            {
                mesh->refine_towards_boundary(QString::number(Agros2D::scene()->edges->items().indexOf(edge)).toStdString(),
                                              edgeRefinement(edge));
            }
        }

    // refine mesh - elements
    if (refineArea)
    {
        foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        {
            if (labelRefinement(label) > 0)
                mesh->refine_in_area(QString::number(Agros2D::scene()->labels->items().indexOf(label)).toStdString(),
                                     labelRefinement(label));
        }
    }
}

// xml module
// name
QString FieldInfo::name() const
{
    return m_plugin->localeName(QString::fromStdString(m_plugin->module()->general().name()));
}

// description
QString FieldInfo::description() const
{
    return m_plugin->localeName(QString::fromStdString(m_plugin->module()->general().description()));
}

// deformable shape
bool FieldInfo::hasDeformableShape() const
{
    if (m_plugin->module()->general().deformed_shape().present())
        return m_plugin->module()->general().deformed_shape().get();

    return false;
}

// latex equation
QString FieldInfo::equation() const
{
    // volumetric weakforms
    for (unsigned int i = 0; i < m_plugin->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = m_plugin->module()->volume().weakforms_volume().weakform_volume().at(i);
        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            return QString::fromStdString(wf.equation());
    }

    assert(0);
}

// constants
QMap<QString, double> FieldInfo::constants() const
{
    QMap<QString, double> constants;
    // constants
    foreach (XMLModule::constant cnst, m_plugin->module()->constants().constant())
        constants[QString::fromStdString(cnst.id())] = cnst.value();

    return constants;
}

// macros
QMap<QString, QString> FieldInfo::macros() const
{
    // QMap<QString, QString> macros;
    // // constants
    // foreach (XMLModule::macro mcro, m_plugin->module()->macros().macro())
    //     macros[QString::fromStdString(mcro.id())] = mcro.value();

    return QMap<QString, QString>();
}

QMap<AnalysisType, QString> FieldInfo::analyses() const
{
    QMap<AnalysisType, QString> analyses;

    for (unsigned int i = 0; i < m_plugin->module()->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = m_plugin->module()->general().analyses().analysis().at(i);

        analyses[analysisTypeFromStringKey(QString::fromStdString(an.id()))] = m_plugin->localeName(QString::fromStdString(an.name()));
    }

    return analyses;
}

// spaces
QMap<int, Module::Space> FieldInfo::spaces() const
{
    // spaces
    QMap<int, Module::Space> spaces;

    foreach (XMLModule::space spc, m_plugin->module()->spaces().space())
        if (spc.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            foreach (XMLModule::space_config config, spc.space_config())
                spaces[config.i()] = Module::Space(config.i(),
                                                   spaceTypeFromStringKey(QString::fromStdString(config.type())),
                                                   config.orderadjust());
    return spaces;
}

// material type
QList<Module::MaterialTypeVariable> FieldInfo::materialTypeVariables() const
{
    // all materials variables
    QList<Module::MaterialTypeVariable> materialTypeVariablesAll;
    for (int i = 0; i < m_plugin->module()->volume().quantity().size(); i++)
    {
        XMLModule::quantity quant = m_plugin->module()->volume().quantity().at(i);

        // gui default
        for (unsigned int i = 0; i < m_plugin->module()->preprocessor().gui().size(); i++)
        {
            XMLModule::gui ui = m_plugin->module()->preprocessor().gui().at(i);
            if (ui.type() == "volume")
            {
                for (unsigned int i = 0; i < ui.group().size(); i++)
                {
                    XMLModule::group grp = ui.group().at(i);
                    for (unsigned int i = 0; i < grp.quantity().size(); i++)
                    {
                        XMLModule::quantity quant_ui = grp.quantity().at(i);
                        if ((quant_ui.id() == quant.id()) && quant_ui.default_().present())
                            quant.default_().set(quant_ui.default_().get());
                    }
                }
            }
        }

        // add to the list
        materialTypeVariablesAll.append(Module::MaterialTypeVariable(quant));
    }

    QList<Module::MaterialTypeVariable> materialTypeVariables;
    for (unsigned int i = 0; i < m_plugin->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = m_plugin->module()->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
        {
            for (unsigned int i = 0; i < wf.quantity().size(); i++)
            {
                XMLModule::quantity qty = wf.quantity().at(i);

                foreach (Module::MaterialTypeVariable variable, materialTypeVariablesAll)
                {
                    if (variable.id().toStdString() == qty.id())
                    {
                        QString nonlinearExpression;
                        if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar && qty.nonlinearity_planar().present())
                            nonlinearExpression = QString::fromStdString(qty.nonlinearity_planar().get());
                        else
                            if (qty.nonlinearity_axi().present())
                                nonlinearExpression = QString::fromStdString(qty.nonlinearity_axi().get());

                        bool isTimeDep = false;
                        if (qty.dependence().present())
                            isTimeDep = (QString::fromStdString(qty.dependence().get()) == "time");

                        materialTypeVariables.append(Module::MaterialTypeVariable(variable.id(), variable.shortname(),
                                                                                  nonlinearExpression, isTimeDep));
                    }
                }
            }
        }
    }

    materialTypeVariablesAll.clear();

    return materialTypeVariables;
}

// variable by name
bool FieldInfo::materialTypeVariableContains(const QString &id) const
{
    foreach (Module::MaterialTypeVariable var, materialTypeVariables())
        if (var.id() == id)
            return true;

    return false;
}

Module::MaterialTypeVariable FieldInfo::materialTypeVariable(const QString &id) const
{
    foreach (Module::MaterialTypeVariable var, materialTypeVariables())
        if (var.id() == id)
            return var;

    assert(0);
}

QList<Module::BoundaryType> FieldInfo::boundaryTypes() const
{
    QList<Module::BoundaryTypeVariable> boundaryTypeVariablesAll;
    for (int i = 0; i < m_plugin->module()->surface().quantity().size(); i++)
    {
        XMLModule::quantity quant = m_plugin->module()->surface().quantity().at(i);

        // add to list
        boundaryTypeVariablesAll.append(Module::BoundaryTypeVariable(quant));
    }

    QList<Module::BoundaryType> boundaryTypes;
    for (int i = 0; i < m_plugin->module()->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface wf = m_plugin->module()->surface().weakforms_surface().weakform_surface().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
        {
            for (int i = 0; i < wf.boundary().size(); i++)
            {
                XMLModule::boundary bdy = wf.boundary().at(i);
                boundaryTypes.append(Module::BoundaryType(this, boundaryTypeVariablesAll, bdy));
            }
        }
    }

    boundaryTypeVariablesAll.clear();

    return boundaryTypes;
}

// default boundary condition
Module::BoundaryType FieldInfo::boundaryTypeDefault() const
{
    for (int i = 0; i < m_plugin->module()->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface wf = m_plugin->module()->surface().weakforms_surface().weakform_surface().at(i);

        // default
        return boundaryType(QString::fromStdString(wf.default_().get()));
    }

    assert(0);
}

// variable by name
bool FieldInfo::boundaryTypeContains(const QString &id) const
{
    foreach (Module::BoundaryType var, boundaryTypes())
        if (var.id() == id)
            return true;

    return false;
}

Module::BoundaryType FieldInfo::boundaryType(const QString &id) const
{
    foreach (Module::BoundaryType var, boundaryTypes())
        if (var.id() == id)
            return var;

    throw AgrosModuleException(QString("Boundary type %1 not found. Probably using corrupted a2d file or wrong version.").arg(id));
}

// force
Module::Force FieldInfo::force() const
{
    // force
    XMLModule::force force = m_plugin->module()->postprocessor().force();
    for (unsigned int i = 0; i < force.expression().size(); i++)
    {
        XMLModule::expression exp = force.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            return Module::Force((Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? QString::fromStdString(exp.planar_x().get()) : QString::fromStdString(exp.axi_r().get()),
                                 (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? QString::fromStdString(exp.planar_y().get()) : QString::fromStdString(exp.axi_z().get()),
                                 (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? QString::fromStdString(exp.planar_z().get()) : QString::fromStdString(exp.axi_phi().get()));
    }
}

// material and boundary user interface
Module::DialogUI FieldInfo::materialUI() const
{
    // preprocessor
    for (unsigned int i = 0; i < m_plugin->module()->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = m_plugin->module()->preprocessor().gui().at(i);
        if (ui.type() == "volume")
            return Module::DialogUI(this, ui);
    }

    assert(0);
}

Module::DialogUI FieldInfo::boundaryUI() const
{
    // preprocessor
    for (unsigned int i = 0; i < m_plugin->module()->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = m_plugin->module()->preprocessor().gui().at(i);
        if (ui.type() == "surface")
            return Module::DialogUI(this, ui);
    }

    assert(0);
}

// local point variables
QList<Module::LocalVariable> FieldInfo::localPointVariables() const
{
    // local variables
    QList<Module::LocalVariable> variables;
    for (unsigned int i = 0; i < m_plugin->module()->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = m_plugin->module()->postprocessor().localvariables().localvariable().at(i);

        for (unsigned int i = 0; i < lv.expression().size(); i++)
        {
            XMLModule::expression expr = lv.expression().at(i);
            if (expr.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            {
                variables.append(Module::LocalVariable(this,
                                                       lv,
                                                       Agros2D::problem()->config()->coordinateType(),
                                                       analysisType()));
            }
        }
    }

    return variables;
}

// view scalar variables
QList<Module::LocalVariable> FieldInfo::viewScalarVariables() const
{
    // scalar variables = local variables
    return localPointVariables();
}

// view vector variables
QList<Module::LocalVariable> FieldInfo::viewVectorVariables() const
{
    // vector variables
    QList<Module::LocalVariable> variables;
    foreach (Module::LocalVariable var, localPointVariables())
        if (!var.isScalar())
            variables.append(var);

    return variables;
}

// surface integrals
QList<Module::Integral> FieldInfo::surfaceIntegrals() const
{
    // surface integrals
    QList<Module::Integral> surfaceIntegrals;
    for (unsigned int i = 0; i < m_plugin->module()->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral sur = m_plugin->module()->postprocessor().surfaceintegrals().surfaceintegral().at(i);

        QString expr;
        for (unsigned int i = 0; i < sur.expression().size(); i++)
        {
            XMLModule::expression exp = sur.expression().at(i);
            if (exp.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            {
                if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                    expr = QString::fromStdString(exp.planar().get()).trimmed();
                else
                    expr = QString::fromStdString(exp.axi().get()).trimmed();
            }
        }

        // new integral
        if (!expr.isEmpty())
        {
            Module::Integral surint = Module::Integral(
                        QString::fromStdString(sur.id()),
                        m_plugin->localeName(QString::fromStdString(sur.name())),
                        QString::fromStdString(sur.shortname()),
                        (sur.shortname_html().present()) ? QString::fromStdString(sur.shortname_html().get()) : QString::fromStdString(sur.shortname()),
                        QString::fromStdString(sur.unit()),
                        (sur.unit_html().present()) ? QString::fromStdString(sur.unit_html().get()) : QString::fromStdString(sur.unit()),
                        expr);

            surfaceIntegrals.append(surint);
        }
    }

    return surfaceIntegrals;
}

// volume integrals
QList<Module::Integral> FieldInfo::volumeIntegrals() const
{
    // volume integrals
    QList<Module::Integral> volumeIntegrals;
    foreach (XMLModule::volumeintegral vol, m_plugin->module()->postprocessor().volumeintegrals().volumeintegral())
    {
        QString expr;
        for (unsigned int i = 0; i < vol.expression().size(); i++)
        {
            XMLModule::expression exp = vol.expression().at(i);
            if (exp.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            {
                if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                    expr = QString::fromStdString(exp.planar().get()).trimmed();
                else
                    expr = QString::fromStdString(exp.axi().get()).trimmed();
            }
        }

        // new integral
        if (!expr.isEmpty())
        {
            Module::Integral volint = Module::Integral(
                        QString::fromStdString(vol.id()),
                        m_plugin->localeName(QString::fromStdString(vol.name())),
                        QString::fromStdString(vol.shortname()),
                        (vol.shortname_html().present()) ? QString::fromStdString(vol.shortname_html().get()) : QString::fromStdString(vol.shortname()),
                        QString::fromStdString(vol.unit()),
                        (vol.unit_html().present()) ? QString::fromStdString(vol.unit_html().get()) : QString::fromStdString(vol.unit()),
                        expr);

            volumeIntegrals.append(volint);
        }
    }

    return volumeIntegrals;
}

// variable by name
Module::LocalVariable FieldInfo::localVariable(const QString &id) const
{
    foreach (Module::LocalVariable var, localPointVariables())
        if (var.id() == id)
            return var;

    qDebug() << "Warning: unable to return local variable: " << id;
    return Module::LocalVariable();
}

Module::Integral FieldInfo::surfaceIntegral(const QString &id) const
{
    foreach (Module::Integral var, surfaceIntegrals())
        if (var.id() == id)
            return var;

    qDebug() << "surfaceIntegral: " << id;
    assert(0);
}

Module::Integral FieldInfo::volumeIntegral(const QString &id) const
{
    foreach (Module::Integral var, volumeIntegrals())
        if (var.id() == id)
            return var;

    qDebug() << "volumeIntegral: " << id;
    assert(0);
}

// default variables
Module::LocalVariable FieldInfo::defaultViewScalarVariable() const
{
    // scalar variables default
    foreach (XMLModule::default_ def, m_plugin->module()->postprocessor().view().scalar_view().default_())
        if (def.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            return localVariable(QString::fromStdString(def.id()));

    assert(0);
}

Module::LocalVariable FieldInfo::defaultViewVectorVariable() const
{
    // vector variables default
    foreach (XMLModule::default_ def, m_plugin->module()->postprocessor().view().vector_view().default_())
        if (def.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            return(localVariable(QString::fromStdString(def.id())));

    assert(0);
}

// weak forms
QList<FormInfo> FieldInfo::wfMatrixVolume() const
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    for (unsigned int i = 0; i < m_plugin->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = m_plugin->module()->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
        {
            // weakform
            for (unsigned int i = 0; i < wf.matrix_form().size(); i++)
            {
                XMLModule::matrix_form form = wf.matrix_form().at(i);
                weakForms.append(FormInfo(QString::fromStdString(form.id()),
                                          form.i(),
                                          form.j(),
                                          form.symmetric() ? Hermes::Hermes2D::HERMES_SYM : Hermes::Hermes2D::HERMES_NONSYM));
            }
        }
    }

    return weakForms;
}

QList<FormInfo> FieldInfo::wfVectorVolume() const
{
    // vector weakforms
    QList<FormInfo> weakForms;
    for (unsigned int i = 0; i < m_plugin->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = m_plugin->module()->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
        {
            for (unsigned int i = 0; i < wf.vector_form().size(); i++)
            {
                XMLModule::vector_form form = wf.vector_form().at(i);
                weakForms.append(FormInfo(QString::fromStdString(form.id()),
                                          form.i(),
                                          form.j()));
            }
        }
    }

    return weakForms;
}

void FieldInfo::load(XMLProblem::field_config *configxsd)
{
    // default
    m_setting = m_settingDefault;

    for (int i = 0; i < configxsd->field_item().size(); i ++)
    {
        Type key = stringKeyToType(QString::fromStdString(configxsd->field_item().at(i).field_key()));

        if (m_settingDefault.keys().contains(key))
        {
            if (m_settingDefault[key].type() == QVariant::Double)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).toDouble();
            else if (m_settingDefault[key].type() == QVariant::Int)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).toInt();
            else if (m_settingDefault[key].type() == QVariant::Bool)
                m_setting[key] = (QString::fromStdString(configxsd->field_item().at(i).field_value()) == "1");
            else if (m_settingDefault[key].type() == QVariant::String)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value());
            else if (m_settingDefault[key].type() == QVariant::StringList)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).split("|");
            else
                qDebug() << "Key not found" << QString::fromStdString(configxsd->field_item().at(i).field_key()) << QString::fromStdString(configxsd->field_item().at(i).field_value());
        }
    }
}

void FieldInfo::save(XMLProblem::field_config *configxsd)
{
    foreach (Type key, m_setting.keys())
    {
        if (m_settingDefault[key].type() == QVariant::StringList)
            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), m_setting[key].toStringList().join("|").toStdString()));
        else if (m_settingDefault[key].type() == QVariant::Bool)
            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), QString::number(m_setting[key].toInt()).toStdString()));
        else
            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), m_setting[key].toString().toStdString()));
    }
}

void FieldInfo::setStringKeys()
{
    m_settingKey[NonlinearTolerance] = "NonlinearTolerance";
    m_settingKey[NonlinearSteps] = "NonlinearSteps";
    m_settingKey[NonlinearConvergenceMeasurement] = "NonlinearConvergenceMeasurement";
    m_settingKey[NewtonDampingType] = "NewtonDampingType";
    m_settingKey[NewtonDampingCoeff] = "NewtonDampingCoeff";
    m_settingKey[NewtonReuseJacobian] = "NewtonReuseJacobian";
    m_settingKey[NewtonSufImprovJacobian] = "NewtonSufImprovJacobian";
    m_settingKey[NewtonMaxStepsReuseJacobian] = "NewtonMaxStepsReuseJacobian";
    m_settingKey[NewtonStepsToIncreaseDF] = "NewtonStepsToIncreaseDF";
    m_settingKey[PicardAndersonAcceleration] = "PicardAndersonAcceleration";
    m_settingKey[PicardAndersonBeta] = "PicardAndersonBeta";
    m_settingKey[PicardAndersonNumberOfLastVectors] = "PicardAndersonNumberOfLastVectors";
    m_settingKey[SpaceNumberOfRefinements] = "SpaceNumberOfRefinements";
    m_settingKey[SpacePolynomialOrder] = "SpacePolynomialOrder";
    m_settingKey[AdaptivitySteps] = "AdaptivitySteps";
    m_settingKey[AdaptivityTolerance] = "AdaptivityTolerance";
    m_settingKey[AdaptivityTransientBackSteps] = "AdaptivityTransientBackSteps";
    m_settingKey[AdaptivityTransientRedoneEach] = "AdaptivityTransientRedoneEach";
    m_settingKey[TransientTimeSkip] = "TransientTimeSkip";
    m_settingKey[TransientInitialCondition] = "TransientInitialCondition";
}

void FieldInfo::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[NonlinearTolerance] = 0.1;
    m_settingDefault[NonlinearSteps] = 10;
    m_settingDefault[NonlinearConvergenceMeasurement] = Hermes::Hermes2D::ResidualNormAbsolute;
    m_settingDefault[NewtonDampingType] = DampingType_Automatic;
    m_settingDefault[NewtonDampingCoeff] = 0.8;
    m_settingDefault[NewtonReuseJacobian] = true;
    m_settingDefault[NewtonSufImprovJacobian] = 0.8;
    m_settingDefault[NewtonMaxStepsReuseJacobian] = 20;
    m_settingDefault[NewtonStepsToIncreaseDF] = 1;
    m_settingDefault[PicardAndersonAcceleration] = true;
    m_settingDefault[PicardAndersonBeta] = 0.2;
    m_settingDefault[PicardAndersonNumberOfLastVectors] = 3;
    m_settingDefault[SpaceNumberOfRefinements] = 1;
    m_settingDefault[SpacePolynomialOrder] = 2;
    m_settingDefault[AdaptivitySteps] = 0;
    m_settingDefault[AdaptivityTolerance] = 1.0;
    m_settingDefault[AdaptivityTransientBackSteps] = 3;
    m_settingDefault[AdaptivityTransientRedoneEach] = 5;
    m_settingDefault[TransientTimeSkip] = 0.0;
    m_settingDefault[TransientInitialCondition] = 0.0;
}
