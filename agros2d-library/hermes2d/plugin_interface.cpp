#include "plugin_interface.h"
#include "field.h"

template <typename Scalar>
SpecialFunction<Scalar>::~SpecialFunction()
{
//    QMutex mutex;
//    mutex.lock();
//    {
//        if(m_interpolation){
//            delete m_interpolation;
//            m_interpolation = 0;
//        }
//    }
//    mutex.unlock();
}
template <typename Scalar>
Scalar SpecialFunction<Scalar>::operator ()(double h) const
{
//    if((h > m_bound_hi) || (h < m_bound_low))
//        std::cout << "mimo " << h << " -> " << m_interpolation->value(h) << std::endl;

//    double value = m_interpolation->value(h);
//    if((value > 1e5) || (value < -1e5))
//        std::cout << "velke " << h << " -> " << value  << std::endl;
//    if(h > m_bound_hi)
//        std::cout << "mimo " << h << " -> " << value << std::endl;


    double value;
    if((h > m_bound_hi) && m_extrapolation_hi_present)
        value = m_extrapolation_hi;
    else if((h < m_bound_low) && m_extrapolation_low_present)
        value = m_extrapolation_low;
    else
        value = m_interpolation->value(h);

        if(!((value < 1e5) && (value > -1e5)))
            std::cout << "velke " << h << " -> " << value  << std::endl;
//        if(h > m_bound_hi)
//            std::cout << "mimo " << h << " -> " << value << std::endl;


    return value;
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
        assert(m_interpolation.isNull());
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
    mutex.unlock();
}

double FormAgrosInterface::markerVolume() const
{
    if(! m_markerVolumeCalculated)
        calculateMarkerVolume();

    return m_markerVolume;
}

template class SpecialFunction<double>;
template class MatrixFormVolAgros<double>;
template class VectorFormVolAgros<double>;
