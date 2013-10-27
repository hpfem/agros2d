#include "plugin_interface.h"
#include "field.h"
#include "util/global.h"

AgrosSpecialExtFunction::AgrosSpecialExtFunction(FieldInfo* fieldInfo, int offsetI) : AgrosExtFunction(fieldInfo, offsetI)
{
}

void AgrosSpecialExtFunction::init()
{
    assert(m_data.isEmpty());
    for (int labelNum = 0; labelNum < Agros2D::scene()->labels->count(); labelNum++)
    {
        SceneLabel* label = Agros2D::scene()->labels->at(labelNum);
        // Needed because of a bug in VS: http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#1261
#ifdef _MSC_VER
        Hermes::Hermes2D::Mesh::MarkersConversion::IntValid marker = m_fieldInfo->initialMesh()->get_element_markers_conversion().get_internal_marker(std::to_string((long double) labelNum));
#else
        Hermes::Hermes2D::Mesh::MarkersConversion::IntValid marker = m_fieldInfo->initialMesh()->get_element_markers_conversion().get_internal_marker(std::to_string((int) labelNum));
#endif
        if(label->hasMarker(m_fieldInfo) && !label->marker(m_fieldInfo)->isNone())
        {
            assert(marker.valid);
            int hermesMarker = marker.marker;
            assert(!m_data.contains(hermesMarker));
            createOneTable(hermesMarker);
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

