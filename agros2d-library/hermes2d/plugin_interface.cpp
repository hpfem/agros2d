#include "plugin_interface.h"
#include "field.h"
#include "util/global.h"

template <typename Scalar>
Scalar SpecialFunction<Scalar>::operator ()(double h) const
{
    assert(m_type == SpecialFunctionType_Function1D);

    double value;

    if((h > m_bound_hi) && m_extrapolation_hi_present)
        value = m_extrapolation_hi;
    else if((h < m_bound_low) && m_extrapolation_low_present)
        value = m_extrapolation_low;
    else
    {
        if(m_useInterpolation)
        {
            assert(m_interpolationCreated);
            value = m_interpolation->value(h);
        }
        else
            value = this->value(h);
    }
    return value;
}

template <typename Scalar>
SpecialFunction<Scalar>::operator Scalar() const
{
    assert(m_type == SpecialFunctionType_Constant);

    if(m_useInterpolation)
        return m_constantValue;
    else
        return value(0);

}


template <typename Scalar>
void SpecialFunction<Scalar>::setBounds(double bound_low, double bound_hi, bool extrapolate_low, bool extrapolate_hi)
{
    m_bound_low = bound_low;
    m_bound_hi = bound_hi;
    m_extrapolation_low_present = extrapolate_low;
    m_extrapolation_hi_present = extrapolate_hi;
    if(extrapolate_low)
        m_extrapolation_low = extrapolation_low();
    if(extrapolate_hi)
        m_extrapolation_hi = extrapolation_hi();
}

template <typename Scalar>
void SpecialFunction<Scalar>::createInterpolation()
{
    QMutex mutex;
    mutex.lock();
    {
        // when more variants will be implemented, this will not be true
        assert(m_interpolation.isNull());        
        assert(!m_interpolationCreated);

        if(m_type == SpecialFunctionType_Constant)
            m_constantValue = value(0);
        else
        {
            Hermes::vector<double> points;
            Hermes::vector<double> values;

            double step = (m_bound_hi - m_bound_low) / (m_count - 1);
            for (int i = 0; i < m_count; i++)
            {
                double h = m_bound_low + i * step;
                points.push_back(h);
                values.push_back(value(h));
            }

            // piece = new Hermes::Hermes2D::CubicSpline(points, values, 0.0, 0.0, true, true, false, false);
            m_interpolation = QSharedPointer<PiecewiseLinear>(new PiecewiseLinear(points, values));
        }
        m_interpolationCreated = true;
    }
    mutex.unlock();
}

//***********************************************************************************

AgrosSpecialExtFunction::AgrosSpecialExtFunction(FieldInfo* fieldInfo, int offsetI) : AgrosExtFunction(fieldInfo, offsetI)
{
}

void AgrosSpecialExtFunction::init()
{
    assert(m_data.isEmpty());
    for (int labelNum = 0; labelNum < Agros2D::scene()->labels->count(); labelNum++)
    {
        Hermes::Hermes2D::Mesh::MarkersConversion::IntValid marker = m_fieldInfo->initialMesh()->get_element_markers_conversion().get_internal_marker(std::to_string(labelNum));
        assert(marker.valid);
        int hermesMarker = marker.marker;
        assert(!m_data.contains(hermesMarker));

        SceneLabel* label = Agros2D::scene()->labels->at(labelNum);
        if(label->hasMarker(m_fieldInfo))
            createOneTable(hermesMarker);
        else
            m_data[hermesMarker] = AgrosSpecialExtFunctionOneMaterial();
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

template AGROS_LIBRARY_API class SpecialFunction<double>;
//template AGROS_LIBRARY_API class AgrosExtFunction<double>;
