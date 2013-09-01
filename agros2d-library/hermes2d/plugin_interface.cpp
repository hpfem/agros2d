#include "plugin_interface.h"

template <typename Scalar>
Scalar SpecialFunction<Scalar>::operator ()(double h) const
{
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
    if(m_interpolation)
        delete m_interpolation;

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
    m_interpolation = new PiecewiseLinear(points, values);
}

template class SpecialFunction<double>;
