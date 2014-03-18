#include "plugin_interface.h"
#include "field.h"
#include "util/global.h"

template<typename Scalar>
FormAgrosInterface<Scalar>::FormAgrosInterface(const WeakFormAgros<Scalar>* weakFormAgros) : m_markerSource(NULL), m_markerTarget(NULL), m_table(NULL), m_wfAgros(weakFormAgros), m_markerVolume(0.0)
{
}

template<typename Scalar>
void FormAgrosInterface<Scalar>::setMarkerSource(const Marker *marker)
{
    m_markerSource = marker;
}

template<typename Scalar>
void FormAgrosInterface<Scalar>::setMarkerTarget(const Marker *marker)
{
    assert(marker != nullptr);
    m_markerTarget = marker;
}

AgrosExtFunction::AgrosExtFunction(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros) : UExtFunction(), m_fieldInfo(fieldInfo), m_wfAgros(wfAgros)
{
//    const int fieldID = this->m_fieldInfo->numberId();
//    m_formsOffset = this->m_wfAgros->positionInfo(fieldID)->formsOffset;
}

AgrosExtFunction::~AgrosExtFunction()
{
}

AgrosSpecialExtFunction::AgrosSpecialExtFunction(const FieldInfo *fieldInfo,  const WeakFormAgros<double>* wfAgros, SpecialFunctionType type, int count) : AgrosExtFunction(fieldInfo, wfAgros), m_type(type), m_count(count)
{
    if((type == SpecialFunctionType_Constant) || (count > 0))
        m_useTable = true;
    else
        m_useTable = false;
}

void AgrosSpecialExtFunction::init()
{
    if(m_useTable)
    {
        for (int labelNum = 0; labelNum < Agros2D::scene()->labels->count(); labelNum++)
        {
            SceneLabel* label = Agros2D::scene()->labels->at(labelNum);
            Hermes::Hermes2D::Mesh::MarkersConversion::IntValid marker = m_fieldInfo->initialMesh()->get_element_markers_conversion().get_internal_marker(QString::number(labelNum).toStdString());
            if(label->hasMarker(m_fieldInfo) && !label->marker(m_fieldInfo)->isNone())
            {
                assert(marker.valid);
                int hermesMarker = marker.marker;
                assert(!m_data.contains(hermesMarker));
                createOneTable(hermesMarker);
            }
        }
    }

}

void AgrosSpecialExtFunction::createOneTable(int hermesMarker)
{
    double constantValue = -123456;
    double extrapolationLow = -123456;
    double extrapolationHi = -123456;
    Hermes::vector<double> points;
    Hermes::vector<double> values;

    if(m_type == SpecialFunctionType_Constant)
        constantValue = calculateValue(hermesMarker, 0);
    else
    {
        double step = (m_boundHi - m_boundLow) / (m_count - 1);
        for (int i = 0; i < m_count; i++)
        {
            double h = m_boundLow + i * step;
            points.push_back(h);
            values.push_back(calculateValue(hermesMarker, h));
        }
        extrapolationLow = calculateValue(hermesMarker, m_boundLow - 1);
        extrapolationHi = calculateValue(hermesMarker, m_boundHi + 1);
    }
    QSharedPointer<DataTable> table(new DataTable(points, values));
    AgrosSpecialExtFunctionOneMaterial materialData(table, constantValue, extrapolationLow, extrapolationHi);
    m_data[hermesMarker] = materialData;
}

double AgrosSpecialExtFunction::valueFromTable(int hermesMarker, double h) const
{
    AgrosSpecialExtFunctionOneMaterial data = m_data[hermesMarker];

    assert(data.m_isValid);

    if(m_type == SpecialFunctionType_Constant)
        return data.m_constantValue;
    else
    {
        if(h < m_boundLow)
            return data.m_extrapolationLow;
        else if(h > m_boundHi)
            return data.m_extrapolationHi;
        else
            return data.m_dataTable->value(h);
    }
}

double AgrosSpecialExtFunction::getValue(int hermesMarker, double h) const
{
    if(m_useTable)
        return valueFromTable(hermesMarker, h);
    else
        return calculateValue(hermesMarker, h);
}

template class AGROS_LIBRARY_API FormAgrosInterface<double>;
template class AGROS_LIBRARY_API MatrixFormVolAgros<double>;
template class AGROS_LIBRARY_API VectorFormVolAgros<double>;
template class AGROS_LIBRARY_API MatrixFormSurfAgros<double>;
template class AGROS_LIBRARY_API VectorFormSurfAgros<double>;
template class AGROS_LIBRARY_API ExactSolutionScalarAgros<double>;
