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

#include "hermes_rf.h"

#include "scene.h"
#include "gui.h"

class WeakFormRFHarmonic : public WeakFormAgros
{
public:
    WeakFormRFHarmonic() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryRF *boundary = dynamic_cast<SceneBoundaryRF *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                // surface current
                if (boundary->type == PhysicFieldBC_RF_SurfaceCurrent)
                {
                    if (fabs(boundary->value_imag.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        - 2 * M_PI * Util::scene()->problemInfo()->frequency * boundary->value_imag.number,
                                                                                                        HERMES_PLANAR));
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(1,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        2 * M_PI * Util::scene()->problemInfo()->frequency * boundary->value_real.number,
                                                                                                        HERMES_PLANAR));
                }

                // port
                if (boundary->type == PhysicFieldBC_RF_Port)
                {
                    if (fabs(boundary->power.number) > EPS_ZERO)
                    {
                        int mode = 0;
                        switch (boundary->mode)
                        {
                        case Mode_0:
                            mode = 1;
                            break;
                        case Mode_01:
                            mode = 2;
                            break;
                        case Mode_02:
                            mode = 3;
                            break;
                        }

                        add_vector_form_surf(new CustomVectorFormSurfPort(0,
                                                                          QString::number(i + 1).toStdString(),
                                                                          boundary->power.number * cos(boundary->phase.number / 180.0 * M_PI),
                                                                          mode));
                        add_vector_form_surf(new CustomVectorFormSurfPort(1,
                                                                          QString::number(i + 1).toStdString(),
                                                                          boundary->power.number * sin(boundary->phase.number / 180.0 * M_PI),
                                                                          mode));
                    }
                }

                // matched boundary
                if ((boundary->type == PhysicFieldBC_RF_MatchedBoundary)
                        || (boundary->type == PhysicFieldBC_RF_Port))
                {
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(0, 1,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 1.0,
                                                                                 2 * M_PI * Util::scene()->problemInfo()->frequency));
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(1, 0,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 - 1.0,
                                                                                 2 * M_PI * Util::scene()->problemInfo()->frequency));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                // real part
                add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(0, 0,
                                                                                                         QString::number(i).toStdString(),
                                                                                                         - 1.0 / (material->permeability.number * MU0),
                                                                                                         HERMES_NONSYM,
                                                                                                         convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                         (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));

                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                          QString::number(i).toStdString(),
                                                                                          sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * (material->permittivity.number * EPS0),
                                                                                          HERMES_NONSYM,
                                                                                          HERMES_PLANAR));

                // imag part
                add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(1, 1,
                                                                                                         QString::number(i).toStdString(),
                                                                                                         - 1.0 / (material->permeability.number * MU0),
                                                                                                         HERMES_NONSYM,
                                                                                                         convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                         (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));


                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 1,
                                                                                          QString::number(i).toStdString(),
                                                                                          sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * (material->permittivity.number * EPS0),
                                                                                          HERMES_NONSYM,
                                                                                          HERMES_PLANAR));

                // lossy environment
                if (fabs(material->conductivity.number) > EPS_ZERO)
                {
                    add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                              QString::number(i).toStdString(),
                                                                                              2 * M_PI * Util::scene()->problemInfo()->frequency * material->conductivity.number,
                                                                                              HERMES_NONSYM,
                                                                                              HERMES_PLANAR));

                    add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 0,
                                                                                              QString::number(i).toStdString(),
                                                                                              - 2 * M_PI * Util::scene()->problemInfo()->frequency * material->conductivity.number,
                                                                                              HERMES_NONSYM,
                                                                                              HERMES_PLANAR));
                }

                // external current density
                if (fabs(material->current_density_imag.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   2 * M_PI * Util::scene()->problemInfo()->frequency * material->current_density_imag.number,
                                                                                                   HERMES_PLANAR));

                if (fabs(material->current_density_real.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   - 2 * M_PI * Util::scene()->problemInfo()->frequency * material->current_density_real.number,
                                                                                                   HERMES_PLANAR));
            }
        }
    }

    class CustomVectorFormSurfPort : public WeakForm::VectorFormSurf
    {
    public:
        CustomVectorFormSurfPort(int i, std::string area, scalar power, int mode)
            : WeakForm::VectorFormSurf(i, area), power(power), mode(mode)
        {
            edge = Util::scene()->edges[QString::fromStdString(area).toInt() - 1];
        }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {            
            SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->elem_marker).c_str())]->material);

            double mu = material->permeability.number * MU0;
            double eps = material->permittivity.number * EPS0;
            double conductivity = material->conductivity.number;

            double length = sqrt(sqr(edge->nodeEnd->point.x - edge->nodeStart->point.x) +
                                 sqr(edge->nodeEnd->point.y - edge->nodeStart->point.y));

            scalar result = 0;
            for (int i = 0; i < n; i++)
            {
                scalar lengthPoint = sqrt(sqr(e->x[i] - edge->nodeStart->point.x) +
                                          sqr(e->y[i] - edge->nodeStart->point.y));

                result += wt[i] * sin((lengthPoint * M_PI) / length) * v->val[i];
            }
            return power * 1e4 * result;
            // sqrt((eps - conductivity / frequency) / mu)
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                        ExtData<Ord> *ext) const {
            return int_v<Ord>(n, wt, v);
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::VectorFormSurf* clone() {
            return new CustomVectorFormSurfPort(*this);
        }

    private:
        int mode;
        scalar power;
        SceneEdge *edge;
    };

    class CustomMatrixFormSurfMatchedBoundary : public WeakForm::MatrixFormSurf
    {
    public:
        CustomMatrixFormSurfMatchedBoundary(int i, int j, std::string area, scalar coeff, scalar frequency)
            : WeakForm::MatrixFormSurf(i, j, area), coeff(coeff), frequency(frequency) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {

            SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->elem_marker).c_str())]->material);

            double mu = material->permeability.number * MU0;
            double eps = material->permittivity.number * EPS0;
            double conductivity = material->conductivity.number;

            return coeff * sqrt((eps - conductivity / frequency) / mu) * int_u_v<double, scalar>(n, wt, u, v);
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u,
                        Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
            return int_u_v<Ord, Ord>(n, wt, u, v);
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::MatrixFormSurf* clone() {
            return new CustomMatrixFormSurfMatchedBoundary(*this);
        }

    private:
        scalar coeff, frequency;
    };
};

/*
template<typename Real, typename Scalar>
Scalar rf_vector_form_surf_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (rfEdge[e->edge_marker].type != PhysicFieldBC_RF_Port)
        return 0.0;

    //   return 0.0;
    // dodelat clen  + 2*j*beta*E0z
    // kde E0z je podle mode
    // pro mode = 1 => pulvlna sinusovky
    // e->x[i]

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    Scalar E0z = 0.0;
    double beta = 0.0;
    int mode = 0;
    Scalar length = sqrt(sqr(rfEdge[e->edge_marker].end.x - rfEdge[e->edge_marker].start.x) +
                         sqr(rfEdge[e->edge_marker].end.y - rfEdge[e->edge_marker].start.y));

    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        Scalar lengthPoint = sqrt(sqr(e->x[i] - rfEdge[e->edge_marker].start.x) +
                                  sqr(e->y[i] - rfEdge[e->edge_marker].start.y));

        switch (rfEdge[e->edge_marker].mode)
        {
        case TEMode_0:
        {
            mode = 1;
            if(!rfEdge[e->edge_marker].height == 0)
                beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            else
                beta = sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps;
            E0z = rfEdge[e->edge_marker].power * sin((lengthPoint * M_PI) / length);
        }

            break;
        case TEMode_1:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  rfEdge[e->edge_marker].power * cos((lengthPoint * M_PI) / (rfEdge[e->edge_marker].height));
        }
            break;
        case TEMode_2:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  0;
        }
            break;
        default:
            break;
        }

        E0z *= sin(rfEdge[e->edge_marker].phase / 180.0 * M_PI);

        if (isPlanar)
            result += wt[i] * E0z * 2 * beta * (v->val[i]);
        else
            result += 2 * M_PI * wt[i] * E0z * 2 * beta * (e->x[i] * v->val[i]);
    }

    return result;
}

template<typename Real, typename Scalar>
Scalar rf_vector_form_surf_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (rfEdge[e->edge_marker].type != PhysicFieldBC_RF_Port)
        return 0.0;

    //   return 0.0;
    // dodelat clen  + 2*j*beta*E0z
    // kde E0z je podle mode
    // pro mode = 1 => pulvlna sinusovky
    // e->x[i]

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    Scalar E0z = 0.0;
    double beta = 0.0;
    int mode = 0;
    Scalar length = sqrt(sqr(rfEdge[e->edge_marker].end.x - rfEdge[e->edge_marker].start.x) +
                         sqr(rfEdge[e->edge_marker].end.y - rfEdge[e->edge_marker].start.y));

    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        Scalar lengthPoint = sqrt(sqr(e->x[i] - rfEdge[e->edge_marker].start.x) +
                                  sqr(e->y[i] - rfEdge[e->edge_marker].start.y));

        switch (rfEdge[e->edge_marker].mode)
        {
        case TEMode_0:
        {
            mode = 1;
            if(!rfEdge[e->edge_marker].height == 0)
                beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            else
                beta = sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps;
            E0z = rfEdge[e->edge_marker].power * sin((lengthPoint * M_PI) / length);
        }

            break;
        case TEMode_1:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  rfEdge[e->edge_marker].power * cos((lengthPoint * M_PI) / (rfEdge[e->edge_marker].height));
        }
            break;
        case TEMode_2:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  0;
        }
            break;
        default:
            break;
        }

        E0z *= - cos(rfEdge[e->edge_marker].phase / 180.0 * M_PI);

        if (isPlanar)
            result += wt[i] * E0z * 2 * beta * (v->val[i]);
        else
            result += 2 * M_PI * wt[i] * E0z * 2 * beta * (e->x[i] * v->val[i]);
    }
    return result;
}

*/

// *******************************************************************************************************

int HermesRF::numberOfSolution() const
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ? 2 : 1;
}

PhysicFieldVariableDeprecated HermesRF::contourPhysicFieldVariable()
{
    return PhysicFieldVariable_RF_ElectricFieldReal;
}

PhysicFieldVariableDeprecated HermesRF::scalarPhysicFieldVariable()
{
    return PhysicFieldVariable_RF_ElectricFieldReal;
}

PhysicFieldVariableComp HermesRF::scalarPhysicFieldVariableComp()
{
    return PhysicFieldVariableComp_Magnitude;
}

PhysicFieldVariableDeprecated HermesRF::vectorPhysicFieldVariable()
{
    return PhysicFieldVariable_RF_ElectricFieldReal;
}


void HermesRF::readBoundaryFromDomElement(QDomElement *element)
{
    Mode mode = modeFromStringKey(element->attribute("mode"));
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
        break;
    case PhysicFieldBC_RF_ElectricField:
    case PhysicFieldBC_RF_SurfaceCurrent:
        Util::scene()->addBoundary(new SceneBoundaryRF(element->attribute("name"),
                                                       type,
                                                       Value(element->attribute("value_real", "0")),
                                                       Value(element->attribute("value_imag", "0"))));
        break;
    case PhysicFieldBC_RF_Port:
        Util::scene()->addBoundary(new SceneBoundaryRF(element->attribute("name"),
                                                       type,
                                                       mode,
                                                       Value(element->attribute("power", "0")),
                                                       Value(element->attribute("phase", "0"))));
        break;
    case PhysicFieldBC_RF_MatchedBoundary:
        Util::scene()->addBoundary(new SceneBoundaryRF(element->attribute("name"),
                                                       type));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesRF::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryRF *edgeRFMarker = dynamic_cast<SceneBoundaryRF *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeRFMarker->type));

    switch (edgeRFMarker->type)
    {
    case PhysicFieldBC_RF_ElectricField:
    case PhysicFieldBC_RF_SurfaceCurrent:
        element->setAttribute("value_real", edgeRFMarker->value_real.text);
        element->setAttribute("value_imag", edgeRFMarker->value_imag.text);
        break;
    case PhysicFieldBC_RF_Port:
        element->setAttribute("power", edgeRFMarker->power.text);
        element->setAttribute("phase", edgeRFMarker->phase.text);
        element->setAttribute("mode", modeToStringKey(edgeRFMarker->mode));
        break;
    case PhysicFieldBC_RF_MatchedBoundary:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesRF::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialRF(element->attribute("name"),
                                                   Value(element->attribute("permittivity", "1")),
                                                   Value(element->attribute("permeability", "1")),
                                                   Value(element->attribute("conductivity", "0")),
                                                   Value(element->attribute("current_density_real", "0")),
                                                   Value(element->attribute("current_density_imag", "0"))));
}

void HermesRF::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialRF *labelRFMarker = dynamic_cast<SceneMaterialRF *>(marker);

    element->setAttribute("permittivity", labelRFMarker->permittivity.text);
    element->setAttribute("permeability", labelRFMarker->permeability.text);
    element->setAttribute("conductivity", labelRFMarker->conductivity.text);
    element->setAttribute("current_density_real", labelRFMarker->current_density_real.text);
    element->setAttribute("current_density_imag", labelRFMarker->current_density_imag.text);
}

LocalPointValue *HermesRF::localPointValue(const Point &point)
{
    return new LocalPointValueRF(point);
}

QStringList HermesRF::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "E_real" << "E_imag" << "E"
            << "B" << "Bx_real" << "By_real" << "B_real" << "Bx_imag" << "By_imag" << "B_imag"
            << "H" << "Hx_real" << "Hy_real" << "H_real" << "Hx_imag" << "Hy_imag" << "H_imag"
            << "Px" << "Py" << "P"
            << "epsr" << "mur" << "gamma"
            << "Je_real" << "Je_imag" << "Je";

    return QStringList(headers);
}

SurfaceIntegralValue *HermesRF::surfaceIntegralValue()
{
    return new SurfaceIntegralValueRF();
}

QStringList HermesRF::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesRF::volumeIntegralValue()
{
    return new VolumeIntegralValueRF();
}

QStringList HermesRF::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneBoundary *HermesRF::newBoundary()
{
    return new SceneBoundaryRF(tr("new boundary"),
                               PhysicFieldBC_RF_ElectricField,
                               Value("0"),
                               Value("0"));
}

SceneBoundary *HermesRF::newBoundary(PyObject *self, PyObject *args)
{
    double value1 = 0.0, value2 = 0.0;
    char *name, *type, *mode = "mode_0";
    if (PyArg_ParseTuple(args, "ss|dds", &name, &type, &value1, &value2, &mode))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_ElectricField ||
                physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_SurfaceCurrent)
            return new SceneBoundaryRF(name,
                                       physicFieldBCFromStringKey(type),
                                       Value(QString::number(value1)),
                                       Value(QString::number(value2)));
        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_Port)
            return new SceneBoundaryRF(name,
                                       physicFieldBCFromStringKey(type),
                                       modeFromStringKey(mode),
                                       Value(QString::number(value1)),
                                       Value(QString::number(value2)));
        if (physicFieldBCFromStringKey(type) == (PhysicFieldBC_RF_MatchedBoundary))
            return new SceneBoundaryRF(name,
                                       physicFieldBCFromStringKey(type));

    }

    return NULL;
}

SceneBoundary *HermesRF::modifyBoundary(PyObject *self, PyObject *args)
{
    // FIXME - parse
    double value1 = 0.0, value2 = 0.0;
    char *name, *type, *mode = "mode_0";
    if (PyArg_ParseTuple(args, "ss|dds", &name, &type, &value1, &value2, &mode))
    {
        if (SceneBoundaryRF *marker = dynamic_cast<SceneBoundaryRF *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
            }

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_ElectricField ||
                    physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_SurfaceCurrent ||
                    physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_Port)
            {
                marker->value_real = Value(QString::number(value1));
                marker->value_imag = Value(QString::number(value2));
                marker->mode = modeFromStringKey(mode);
            }
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
}

SceneMaterial *HermesRF::newMaterial()
{
    return new SceneMaterialRF(tr("new material"),
                               Value("1"),
                               Value("1"),
                               Value("0"),
                               Value("0"),
                               Value("0"));
}

SceneMaterial *HermesRF::newMaterial(PyObject *self, PyObject *args)
{
    double permittivity, permeability, conductivity = 0.0, current_density_real = 0.0, current_density_imag = 0.0;
    char *name;
    if (PyArg_ParseTuple(args, "sdd|ddd", &name, &permittivity, &permeability, &conductivity, &current_density_real, &current_density_imag))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialRF(name,
                                   Value(QString::number(permittivity)),
                                   Value(QString::number(permeability)),
                                   Value(QString::number(conductivity)),
                                   Value(QString::number(current_density_real)),
                                   Value(QString::number(current_density_imag)));
    }

    return NULL;
}

SceneMaterial *HermesRF::modifyMaterial(PyObject *self, PyObject *args)
{
    double permittivity, permeability, conductivity, current_density_real, current_density_imag;
    char *name;
    if (PyArg_ParseTuple(args, "sddddd", &name, &permittivity, &permeability, &conductivity, &current_density_real, &current_density_imag))
    {
        if (SceneMaterialRF *marker = dynamic_cast<SceneMaterialRF *>(Util::scene()->getMaterial(name)))
        {
            marker->permittivity = Value(QString::number(permittivity));
            marker->permeability = Value(QString::number(permeability));
            marker->conductivity = Value(QString::number(conductivity));
            marker->current_density_real = Value(QString::number(current_density_real));
            marker->current_density_imag = Value(QString::number(current_density_imag));
            return marker;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
}

void HermesRF::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricField), PhysicFieldVariable_RF_ElectricField);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricFieldReal), PhysicFieldVariable_RF_ElectricFieldReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricFieldImag), PhysicFieldVariable_RF_ElectricFieldImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticField), PhysicFieldVariable_RF_MagneticField);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldXReal), PhysicFieldVariable_RF_MagneticFieldXReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldXImag), PhysicFieldVariable_RF_MagneticFieldXImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldYReal), PhysicFieldVariable_RF_MagneticFieldYReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldYImag), PhysicFieldVariable_RF_MagneticFieldYImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensity), PhysicFieldVariable_RF_MagneticFluxDensity);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityXReal), PhysicFieldVariable_RF_MagneticFluxDensityXReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityXImag), PhysicFieldVariable_RF_MagneticFluxDensityXImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityYReal), PhysicFieldVariable_RF_MagneticFluxDensityYReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityYImag), PhysicFieldVariable_RF_MagneticFluxDensityYImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PoyntingVector), PhysicFieldVariable_RF_PoyntingVector);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PoyntingVectorX), PhysicFieldVariable_RF_PoyntingVectorX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PoyntingVectorY), PhysicFieldVariable_RF_PoyntingVectorY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PowerLosses), PhysicFieldVariable_RF_PowerLosses);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_Permittivity), PhysicFieldVariable_RF_Permittivity);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_Permeability), PhysicFieldVariable_RF_Permeability);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_Conductivity), PhysicFieldVariable_RF_Conductivity);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_CurrentDensityReal), PhysicFieldVariable_RF_CurrentDensityReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_CurrentDensityImag), PhysicFieldVariable_RF_CurrentDensityImag);
}

void HermesRF::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricFieldReal), PhysicFieldVariable_RF_ElectricFieldReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldXReal), PhysicFieldVariable_RF_MagneticFieldXReal);
}


void HermesRF::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueRF *localPointValueRF = dynamic_cast<LocalPointValueRF *>(localPointValue);

    // rf field
    QTreeWidgetItem *rfNode = new QTreeWidgetItem(trvWidget);
    rfNode->setText(0, tr("RF field"));
    rfNode->setExpanded(true);

    // material
    addTreeWidgetItemValue(rfNode, tr("Permittivity:"), QString("%1").arg(localPointValueRF->permittivity, 0, 'f', 2), "");
    addTreeWidgetItemValue(rfNode, tr("Permeability:"), QString("%1").arg(localPointValueRF->permeability, 0, 'f', 2), "");
    addTreeWidgetItemValue(rfNode, tr("Conductivity:"), QString("%1").arg(localPointValueRF->conductivity, 0, 'e', 3), "");

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(rfNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addTreeWidgetItemValue(itemElectricField, tr("real:"), QString("%1").arg(localPointValueRF->electric_field_real, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, tr("imag:"), QString("%1").arg(localPointValueRF->electric_field_imag, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->electric_field_real) + sqr(localPointValueRF->electric_field_imag)), 0, 'e', 3), "V/m");

    // Flux Density
    addTreeWidgetItemValue(rfNode, "Flux density:", QString("%1").arg(sqrt(sqr(localPointValueRF->flux_density_real.x) +
                                                                           sqr(localPointValueRF->flux_density_real.y) +
                                                                           sqr(localPointValueRF->flux_density_imag.x) +
                                                                           sqr(localPointValueRF->flux_density_imag.y)), 0, 'e', 3), "T");

    // Flux Density - real
    QTreeWidgetItem *itemFluxDensityReal = new QTreeWidgetItem(rfNode);
    itemFluxDensityReal->setText(0, tr("Flux density - real"));
    itemFluxDensityReal->setExpanded(true);

    addTreeWidgetItemValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueRF->flux_density_real.x, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueRF->flux_density_real.y, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityReal, "B:", QString("%1").arg(localPointValueRF->flux_density_real.magnitude(), 0, 'e', 3), "T");

    // Flux Density - imag
    QTreeWidgetItem *itemFluxDensityImag = new QTreeWidgetItem(rfNode);
    itemFluxDensityImag->setText(0, tr("Flux density - imag"));
    itemFluxDensityImag->setExpanded(true);

    addTreeWidgetItemValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueRF->flux_density_imag.x, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueRF->flux_density_imag.y, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityImag, "B:", QString("%1").arg(localPointValueRF->flux_density_imag.magnitude(), 0, 'e', 3), "T");

    // Magnetic Field
    addTreeWidgetItemValue(rfNode, "Magnetic field:", QString("%1").arg(sqrt(sqr(localPointValueRF->magnetic_field_real.x) +
                                                                             sqr(localPointValueRF->magnetic_field_real.y) +
                                                                             sqr(localPointValueRF->magnetic_field_imag.x) +
                                                                             sqr(localPointValueRF->magnetic_field_imag.y)), 0, 'e', 3), "A/m");

    // Magnetic Field - real
    QTreeWidgetItem *itemMagneticFieldReal = new QTreeWidgetItem(rfNode);
    itemMagneticFieldReal->setText(0, tr("Magnetic field - real"));
    itemMagneticFieldReal->setExpanded(true);

    addTreeWidgetItemValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueRF->magnetic_field_real.x, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueRF->magnetic_field_real.y, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldReal, "H", QString("%1").arg(localPointValueRF->magnetic_field_real.magnitude(), 0, 'e', 3), "A/m");

    // Magnetic Field - imag
    QTreeWidgetItem *itemMagneticFieldImag = new QTreeWidgetItem(rfNode);
    itemMagneticFieldImag->setText(0, tr("Magnetic field - imag"));
    itemMagneticFieldImag->setExpanded(true);

    addTreeWidgetItemValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueRF->magnetic_field_imag.x, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueRF->magnetic_field_imag.y, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldImag, "H", QString("%1").arg(localPointValueRF->magnetic_field_imag.magnitude(), 0, 'e', 3), "A/m");

    // Poynting vector
    QTreeWidgetItem *itemPoyntingVector = new QTreeWidgetItem(rfNode);
    itemPoyntingVector->setText(0, tr("Poynting vector"));
    itemPoyntingVector->setExpanded(true);

    addTreeWidgetItemValue(itemPoyntingVector, tr("P") + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueRF->poynting_vector.x, 0, 'e', 3), "W/m2");
    addTreeWidgetItemValue(itemPoyntingVector, tr("P") + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueRF->poynting_vector.x, 0, 'e', 3), "W/m2");
    addTreeWidgetItemValue(itemPoyntingVector, tr("P:"), QString("%1").arg(localPointValueRF->poynting_vector.magnitude(), 0, 'e', 3), "W/m2");

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(rfNode);
    itemCurrentDensity->setText(0, tr("Current density"));
    itemCurrentDensity->setExpanded(true);

    addTreeWidgetItemValue(itemCurrentDensity, tr("real:"), QString("%1").arg(localPointValueRF->current_density_real, 0, 'f', 2), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, tr("imag:"), QString("%1").arg(localPointValueRF->current_density_imag, 0, 'f', 2), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->current_density_real) + sqr(localPointValueRF->current_density_imag)), 0, 'f', 2), "A/m2");
}

void HermesRF::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueRF *surfaceIntegralValueRF = dynamic_cast<SurfaceIntegralValueRF *>(surfaceIntegralValue);

    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("RF field"));
    magneticNode->setExpanded(true);
}

void HermesRF::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueRF *volumeIntegralValueRF = dynamic_cast<VolumeIntegralValueRF *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("RF field"));
    magneticNode->setExpanded(true);
}

ViewScalarFilter *HermesRF::viewScalarFilter(PhysicFieldVariableDeprecated physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        Solution *sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
        return new ViewScalarFilterRF(Hermes::vector<MeshFunction *>(sln1, sln2),
                                      physicFieldVariable,
                                      physicFieldVariableComp);
    }
}

QList<SolutionArray *> HermesRF::solve(ProgressItemSolve *progressItemSolve)
{
    // boundaries
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryRF *boundary = dynamic_cast<SceneBoundaryRF *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value_real.evaluate()) return QList<SolutionArray *>();
        if (!boundary->value_imag.evaluate()) return QList<SolutionArray *>();
        if (!boundary->power.evaluate()) return QList<SolutionArray *>();
        if (!boundary->phase.evaluate()) return QList<SolutionArray *>();        
    }

    // materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->permeability.evaluate()) return QList<SolutionArray *>();
        if (!material->permittivity.evaluate()) return QList<SolutionArray *>();
        if (!material->conductivity.evaluate()) return QList<SolutionArray *>();
        if (!material->current_density_real.evaluate()) return QList<SolutionArray *>();
        if (!material->current_density_imag.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryRF *boundary = dynamic_cast<SceneBoundaryRF *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_RF_ElectricField)
            {
                bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value_real.number));
                bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value_imag.number));
            }
        }
    }
    Hermes::vector<EssentialBCs> bcs(bc1, bc2);

    WeakFormRFHarmonic wf;

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueRF::LocalPointValueRF(const Point &point) : LocalPointValue(point)
{
    permittivity = 0;
    permeability = 0;
    conductivity = 0;
    current_density_real = 0;
    current_density_imag = 0;

    electric_field_real = 0;
    electric_field_imag = 0;
    magnetic_field_real = Point();
    magnetic_field_imag = Point();
    flux_density_real = Point();
    flux_density_imag = Point();
    poynting_vector = Point();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // value real
        PointValue valueReal = PointValue(value, derivative, material);

        SceneMaterialRF *marker = dynamic_cast<SceneMaterialRF *>(valueReal.marker);
        // solution
        if (marker != NULL)
        {
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            {
                Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

                double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;
                double mu = marker->permeability.number * MU0;
                // value imag
                PointValue valueImag = pointValue(sln2, point);
                // derivative
                Point derReal = valueReal.derivative;
                Point derImag = valueImag.derivative;

                // Electric Field
                electric_field_real = valueReal.value;
                electric_field_imag = valueImag.value;

                // Magnetic Field
                magnetic_field_real.x = -(1/(w*mu))*derImag.y;
                magnetic_field_real.y = (1/(w*mu))*derImag.x;
                magnetic_field_imag.x = (1/(w*mu))*derReal.y;
                magnetic_field_imag.y = -(1/(w*mu))*derReal.x;

                // Magnetic Flux Density
                flux_density_real.x = -(1/w)*derImag.y;
                flux_density_real.y = (1/w)*derImag.x;
                flux_density_imag.x = (1/w)*derReal.y;
                flux_density_imag.y = -(1/w)*derReal.x;

                // Poynting vector
                poynting_vector.x = 0.5 * ((valueImag.value * 1/(w*mu) * derReal.x) -
                                           (valueReal.value * 1/(w*mu) * derImag.x));
                poynting_vector.y = 0.5 * ((valueImag.value * 1/(w*mu) * derReal.y) -
                                           (valueReal.value * 1/(w*mu) * derImag.y));

                // material + current density
                permittivity = marker->permittivity.number;
                permeability = marker->permeability.number;
                conductivity = marker->conductivity.number;
                current_density_real = marker->current_density_real.number;
                current_density_imag = marker->current_density_imag.number;
            }
        }
    }
}

double LocalPointValueRF::variableValue(PhysicFieldVariableDeprecated physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_RF_ElectricField:
    {
        return sqrt(sqr(electric_field_real) + sqr(electric_field_imag));
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldReal:
    {
        return electric_field_real;
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldImag:
    {
        return electric_field_imag;
    }
        break;
    case PhysicFieldVariable_RF_MagneticField:
    {
        return sqrt(sqr(magnetic_field_real.x) + sqr(magnetic_field_imag.x) + sqr(magnetic_field_real.y) + sqr(magnetic_field_imag.y));
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldXReal:
    {
        return magnetic_field_real.x;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldXImag:
    {
        return magnetic_field_imag.x;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldYReal:
    {
        return magnetic_field_real.y;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldYImag:
    {
        return magnetic_field_imag.y;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensity:
    {
        return sqrt(sqr(flux_density_real.x) + sqr(flux_density_imag.x) + sqr(flux_density_real.y) + sqr(flux_density_imag.y));
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityXReal:
    {
        return flux_density_real.x;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityXImag:
    {
        return flux_density_imag.x;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityYReal:
    {
        return flux_density_real.y;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityYImag:
    {
        return flux_density_imag.y;
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVector:
    {
        return poynting_vector.magnitude();
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorX:
    {
        return poynting_vector.x;
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorY:
    {
        return poynting_vector.y;
    }
        break;
    case PhysicFieldVariable_RF_Permittivity:
    {
        return permittivity;
    }
        break;
    case PhysicFieldVariable_RF_Permeability:
    {
        return permeability;
    }
        break;
    case PhysicFieldVariable_RF_Conductivity:
    {
        return conductivity;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityReal:
    {
        return current_density_real;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityImag:
    {
        return current_density_imag;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueRF::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueRF::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(electric_field_real, 0, 'e', 5) <<
           QString("%1").arg(electric_field_imag, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(electric_field_real) + sqr(electric_field_imag)), 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(flux_density_real.x) + sqr(flux_density_imag.x) + sqr(flux_density_real.y) + sqr(flux_density_imag.y)), 0, 'e', 5) <<
           QString("%1").arg(flux_density_real.x, 0, 'e', 5) <<
           QString("%1").arg(flux_density_real.y, 0, 'e', 5) <<
           QString("%1").arg(flux_density_real.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(flux_density_imag.x, 0, 'e', 5) <<
           QString("%1").arg(flux_density_imag.y, 0, 'e', 5) <<
           QString("%1").arg(flux_density_imag.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(magnetic_field_real.x) + sqr(magnetic_field_imag.x) + sqr(magnetic_field_real.y) + sqr(magnetic_field_imag.y)), 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_real.x, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_real.y, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_real.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_imag.x, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_imag.y, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_imag.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(poynting_vector.x, 0, 'e', 5) <<
           QString("%1").arg(poynting_vector.y, 0, 'e', 5) <<
           QString("%1").arg(poynting_vector.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(permittivity, 0, 'e', 5) <<
           QString("%1").arg(permeability, 0, 'f', 3) <<
           QString("%1").arg(conductivity, 0, 'e', 5) <<
           QString("%1").arg(current_density_real, 0, 'e', 5) <<
           QString("%1").arg(current_density_imag, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(current_density_real) + sqr(current_density_imag)), 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueRF::SurfaceIntegralValueRF() : SurfaceIntegralValue()
{
    calculate();
}

void SurfaceIntegralValueRF::calculateVariables(int i)
{
}

QStringList SurfaceIntegralValueRF::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueRF::VolumeIntegralValueRF() : VolumeIntegralValue()
{
    calculate();
}

void VolumeIntegralValueRF::calculateVariables(int i)
{
}

void VolumeIntegralValueRF::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList VolumeIntegralValueRF::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterRF::calculateVariable(int i)
{
    SceneMaterialRF *marker = dynamic_cast<SceneMaterialRF *>(material);

    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_RF_ElectricField:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
        }
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldReal:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value1[i];
        }
        else
        {
            node->values[0][0][i] = value1[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldImag:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value2[i];
        }
        else
        {
            node->values[0][0][i] = - value2[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticField:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudy1[i]) + sqr((1/(w*mu)) * dudx2[i]) + sqr(-(1/(w*mu)) * dudx1[i]));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudy1[i]) + sqr((1/(w*mu)) * dudx2[i]) + sqr(-(1/(w*mu)) * dudx1[i])) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldXReal:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = -(1/(w*mu)) * dudy2[i];
        }
        else
        {
            node->values[0][0][i] = -(1/(w*mu)) * dudy2[i] * x[i];
        }
    }

        break;
    case PhysicFieldVariable_RF_MagneticFieldXImag:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = (1/(w*mu)) * dudy1[i];
        }
        else
        {
            node->values[0][0][i] = (1/(w*mu)) * dudy1[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldYReal:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = (1/(w*mu)) * dudx2[i];
        }
        else
        {
            node->values[0][0][i] = (1/(w*mu)) * dudx2[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldYImag:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = -(1/(w*mu)) * dudx1[i];
        }
        else
        {
            node->values[0][0][i] = -(1/(w*mu)) * dudx1[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensity:
    {
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(-(1/(w)) * dudy2[i]) + sqr((1/(w)) * dudy1[i]) + sqr((1/(w)) * dudx2[i]) + sqr(-(1/(w)) * dudx1[i]));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(-(1/(w)) * dudy2[i]) + sqr((1/(w)) * dudy1[i]) + sqr((1/(w)) * dudx2[i]) + sqr(-(1/(w)) * dudx1[i]));
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityXReal:
    {
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = -(1/(w)) * dudy2[i];
        }
        else
        {
            node->values[0][0][i] = -(1/(w)) * dudy2[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityXImag:
    {
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = (1/(w)) * dudy1[i];
        }
        else
        {
            node->values[0][0][i] = (1/(w)) * dudy1[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityYReal:
    {
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = (1/(w)) * dudx2[i];
        }
        else
        {
            node->values[0][0][i] = (1/(w)) * dudx2[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityYImag:
    {
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = -(1/(w)) * dudx1[i];
        }
        else
        {
            node->values[0][0][i] = -(1/(w)) * dudx1[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVector:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = 0.5 * sqrt(sqr(((value2[i] * 1/(w*mu) * dudx1[i]) - (value1[i] * 1/(w*mu) * dudx2[i]))) +
                                               sqr(((value2[i] * 1/(w*mu) * dudy1[i]) - (value1[i] * 1/(w*mu) * dudy2[i]))));
        }
        else
        {
            node->values[0][0][i] = 0.5 * sqrt(sqr(((value2[i] * 1/(w*mu) * dudx1[i]) - (value1[i] * 1/(w*mu) * dudx2[i]))) +
                                               sqr(((value2[i] * 1/(w*mu) * dudy1[i]) - (value1[i] * 1/(w*mu) * dudy2[i]))));
        }
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorX:
    {
        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = - 0.5 * ((value2[i] * 1/(w*mu) * dudx1[i]) - (value1[i] * 1/(w*mu) * dudx2[i]));
        }
        else
        {
            node->values[0][0][i] = - 0.5 * ((value2[i] * 1/(w*mu) * dudx1[i]) - (value1[i] * 1/(w*mu) * dudx2[i]));
        }
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorY:
    {
        SceneMaterialRF *marker = dynamic_cast<SceneMaterialRF *>(material);

        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * Util::scene()->problemInfo()->frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = 0.5 * ((value2[i] * 1/(w*mu) * dudy1[i]) - (value1[i] * 1/(w*mu) * dudy2[i]));
        }
        else
        {
            //node->values[0][0][i] = 0.5 * ((value2[i] * 1/(w*mu) * dudx1[i]) - (value1[i] * 1/(w*mu) * dudx2[i]));
            node->values[0][0][i] = 0.5 * ((value2[i] * 1/(w*mu) * dudy1[i]) - (value1[i] * 1/(w*mu) * dudy2[i]));
        }
    }
        break;
    case PhysicFieldVariable_RF_PowerLosses:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = 0.5 * (sqr(value1[i]) + sqr(value2[i])) * (marker->conductivity.number);
        }
        else
        {
            node->values[0][0][i] = 0.5 * (sqr(value1[i]) + sqr(value2[i])) * (marker->conductivity.number) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_Permeability:
    {
        node->values[0][0][i] = marker->permeability.number;
    }
        break;
    case PhysicFieldVariable_RF_Permittivity:
    {
        node->values[0][0][i] = marker->permittivity.number;
    }
        break;
    case PhysicFieldVariable_RF_Conductivity:
    {
        node->values[0][0][i] = marker->conductivity.number;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityReal:
    {
        node->values[0][0][i] = marker->current_density_real.number;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityImag:
    {
        node->values[0][0][i] = marker->current_density_imag.number;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterRF::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneBoundaryRF::SceneBoundaryRF(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag)
    : SceneBoundary(name, type)
{
    this->value_real = value_real;
    this->value_imag = value_imag;
}

SceneBoundaryRF::SceneBoundaryRF(const QString &name, PhysicFieldBC type, Mode mode, Value power, Value phase)
    : SceneBoundary(name, type)
{
    this->power = power;
    this->phase = phase;
    this->mode = mode;
}

SceneBoundaryRF::SceneBoundaryRF(const QString &name, PhysicFieldBC type) : SceneBoundary(name, type) { }

QString SceneBoundaryRF::script()
{
    if (type == PhysicFieldBC_RF_ElectricField ||
            type == PhysicFieldBC_RF_SurfaceCurrent)
        return QString("addboundary(\"%1\", \"%2\", %3, %4)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(value_real.text).
                arg(value_imag.text);

    if (type == (PhysicFieldBC_RF_Port))
        return QString("addboundary(\"%1\", \"%2\", %3, %4, \"%5\")").
                arg(name).
                arg(physicFieldBCToStringKey(type)).                
                arg(power.text).
                arg(phase.text).
                arg(modeToStringKey(mode));

    if (type == PhysicFieldBC_RF_MatchedBoundary)
    {
        return QString("addboundary(\"%1\", \"%2\")").
                arg(name).
                arg(physicFieldBCToStringKey(type));
    }

}

QMap<QString, QString> SceneBoundaryRF::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_RF_ElectricField:
        out["Electric field - real (V/m)"] = value_real.text;
        out["Electric field - imag (V/m)"] = value_imag.text;
        break;
    case PhysicFieldBC_RF_SurfaceCurrent:
        out["Surface current - real (A/m)"] = value_real.text;
        out["Surface current - imag (A/m)"] = value_imag.text;
        break;
    case PhysicFieldBC_RF_Port:
        out["Port - power (W)"] = power.text;
        out["Port - phase (deg.)"] = phase.text;
        break;
    case PhysicFieldBC_RF_MatchedBoundary:
        break;

    }
    return QMap<QString, QString>(out);
}

int SceneBoundaryRF::showDialog(QWidget *parent)
{
    SceneBoundaryRFDialog *dialog = new SceneBoundaryRFDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialRF::SceneMaterialRF(const QString &name, Value permittivity,Value permeability, Value conductivity, Value current_density_real, Value current_density_imag)
    : SceneMaterial(name)
{
    this->permittivity = permittivity;
    this->permeability = permeability;
    this->conductivity = conductivity;
    this->current_density_real = current_density_real;
    this->current_density_imag = current_density_imag;
}

QString SceneMaterialRF::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5)").
            arg(name).
            arg(permittivity.text).
            arg(permeability.text).
            arg(conductivity.text).
            arg(current_density_real.text).
            arg(current_density_imag.text);
}

QMap<QString, QString> SceneMaterialRF::data()
{
    QMap<QString, QString> out;
    out["Permittivity (-)"] = permittivity.text;
    out["Permeability (-)"] = permeability.text;
    out["Conductivity (S/m)"] = conductivity.text;
    out["Current density - real (A/m2)"] = current_density_real.text;
    out["Current density - imag (A/m2)"] = current_density_imag.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialRF::showDialog(QWidget *parent)
{
    SceneMaterialRFDialog *dialog = new SceneMaterialRFDialog(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryRFDialog::SceneBoundaryRFDialog(SceneBoundaryRF *edgeRFMarker, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = edgeRFMarker;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryRFDialog::createContent()
{
    lblValueUnitReal = new QLabel("");
    lblValueUnitImag = new QLabel("");

    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_ElectricField), PhysicFieldBC_RF_ElectricField);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_SurfaceCurrent), PhysicFieldBC_RF_SurfaceCurrent);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_MatchedBoundary), PhysicFieldBC_RF_MatchedBoundary);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_Port), PhysicFieldBC_RF_Port);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValueReal = new ValueLineEdit(this, true);
    txtValueImag = new ValueLineEdit(this, true);

    cmbMode = new QComboBox(this);
    cmbMode->addItem(teModeString(Mode_0), Mode_0);
    cmbMode->addItem(teModeString(Mode_01), Mode_01);
    cmbMode->addItem(teModeString(Mode_02), Mode_02);

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    connect(txtValueReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtValueImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutPhysicField = new QHBoxLayout();
    layoutPhysicField->addWidget(txtValueReal);
    layoutPhysicField->addWidget(lblValueUnitImag, 10, 0);
    layoutPhysicField->addWidget(txtValueImag);

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnitReal, 10, 0);
    layout->addLayout(layoutPhysicField, 10, 2);
    layout->addWidget(new QLabel(tr("Mode:")), 12, 0);
    layout->addWidget(cmbMode, 12, 2);
}

void SceneBoundaryRFDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryRF *edgeRFMarker = dynamic_cast<SceneBoundaryRF *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(edgeRFMarker->type));

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port)
        cmbMode->setCurrentIndex(cmbMode->findData(edgeRFMarker->mode));

    if (edgeRFMarker->type == PhysicFieldBC_RF_ElectricField ||
            edgeRFMarker->type == PhysicFieldBC_RF_SurfaceCurrent)
    {
        txtValueReal->setValue(edgeRFMarker->value_real);
        txtValueImag->setValue(edgeRFMarker->value_imag);
    }

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port)
    {
        txtValueReal->setValue(edgeRFMarker->power);
        txtValueImag->setValue(edgeRFMarker->phase);
    }
}

bool SceneBoundaryRFDialog::save()
{
    if (!SceneBoundaryDialog::save()) return false;;

    SceneBoundaryRF *edgeRFMarker = dynamic_cast<SceneBoundaryRF *>(m_boundary);

    edgeRFMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();
    edgeRFMarker->mode = (Mode) cmbMode->itemData(cmbMode->currentIndex()).toInt();

    if (edgeRFMarker->type == PhysicFieldBC_RF_ElectricField ||
            edgeRFMarker->type == PhysicFieldBC_RF_SurfaceCurrent)
    {
        if (txtValueReal->evaluate())
            edgeRFMarker->value_real  = txtValueReal->value();
        else
            return false;

        if (txtValueImag->evaluate())
            edgeRFMarker->value_imag  = txtValueImag->value();
        else
            return false;
    }
    else
    {
        edgeRFMarker->value_real = Value();
        edgeRFMarker->value_imag = Value();
    }

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port)
    {
        if (txtValueReal->evaluate())
            edgeRFMarker->power  = txtValueReal->value();
        else
            return false;

        if (txtValueImag->evaluate())
            edgeRFMarker->phase = txtValueImag->value();
        else
            return false;
    }
    else
    {
        edgeRFMarker->power = Value();
        edgeRFMarker->phase = Value();
    }

    return true;
}

void SceneBoundaryRFDialog::doTypeChanged(int index)
{
    txtValueReal->setEnabled(false);
    txtValueImag->setEnabled(false);
    cmbMode->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_RF_ElectricField:
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>E</i><sub>%1</sub>").arg(Util::scene()->problemInfo()->labelZ().toLower()));
        lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr(" + j "));
        lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
        break;

    case PhysicFieldBC_RF_SurfaceCurrent:
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>J</i><sub>0</sub>"));
        lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr(" + j "));
        lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
        break;

    case PhysicFieldBC_RF_MatchedBoundary:
    {
        lblValueUnitReal->setText("-");
        lblValueUnitImag->setText("");
    }
        break;

    case PhysicFieldBC_RF_Port:
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>P</i> (W):"));
        lblValueUnitReal->setToolTip(tr("Power"));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr("<i>%1</i> (°):").arg(QString::fromUtf8("φ")));
        lblValueUnitImag->setToolTip(tr("Phase"));
        cmbMode->setEnabled(true);
    }
        break;
    }
}


// *************************************************************************************************************************************

SceneMaterialRFDialog::SceneMaterialRFDialog(QWidget *parent, SceneMaterialRF *labelRFMarker) : SceneMaterialDialog(parent)
{
    m_material = labelRFMarker;

    createDialog();

    load();
    setSize();
}

void SceneMaterialRFDialog::createContent()
{
    txtPermittivity = new ValueLineEdit(this);
    txtPermeability = new ValueLineEdit(this);
    txtConductivity = new ValueLineEdit(this);
    txtCurrentDensityReal = new ValueLineEdit(this);
    txtCurrentDensityImag = new ValueLineEdit(this);

    connect(txtPermeability, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPermittivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("ε")),
                                  tr("Permittivity")), 10, 0);
    layout->addWidget(txtPermittivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("μ")),
                                  tr("Permeability")), 11, 0);
    layout->addWidget(txtPermeability, 11, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 12, 0);
    layout->addWidget(txtConductivity, 12, 2);
    layout->addWidget(new QLabel(), 13, 0);
    layout->addWidget(createLabel(tr("<i>J</i><sub>ext</sub> (A/m<sup>2</sup>)"),
                                  tr("Current density")), 13, 0);
    layout->addLayout(layoutCurrentDensity, 13, 2);
}

void SceneMaterialRFDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialRF *labelRFMarker = dynamic_cast<SceneMaterialRF *>(m_material);

    txtPermittivity->setValue(labelRFMarker->permittivity);
    txtPermeability->setValue(labelRFMarker->permeability);
    txtConductivity->setValue(labelRFMarker->conductivity);
    txtCurrentDensityReal->setValue(labelRFMarker->current_density_real);
    txtCurrentDensityImag->setValue(labelRFMarker->current_density_imag);
}

bool SceneMaterialRFDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialRF *labelRFMarker = dynamic_cast<SceneMaterialRF *>(m_material);

    if (txtPermittivity->evaluate())
        labelRFMarker->permittivity  = txtPermittivity->value();
    else
        return false;

    if (txtPermeability->evaluate())
        labelRFMarker->permeability  = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelRFMarker->conductivity  = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        labelRFMarker->current_density_real  = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        labelRFMarker->current_density_imag  = txtCurrentDensityImag->value();
    else
        return false;

    return true;
}
