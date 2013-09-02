#include "plugin_interface.h"

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
    return m_interpolation->value(h);
}

template <typename Scalar>
void SpecialFunction<Scalar>::setBounds(double bound_low, double bound_hi)
{
    m_bound_low = bound_low;
    m_bound_hi = bound_hi;
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

template class SpecialFunction<double>;
