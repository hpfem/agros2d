#include "util.h"
#include <cmath>
#include <iostream>

#include "firesafety.h"

using namespace std;

const double FireSafety::I0_LIMIT = 18500;
const double FireSafety::SIGMA = 5.67e-8;


FireSafety::FireSafety(double width, double height, double pv)
{
    m_width = width;
    m_height = height;
    m_pv = pv;
}

double FireSafety::fireCurve(double pv)
{
    double T = 20 + 345 * log10(8 * m_pv + 1) + 273;
    return T;
}


double FireSafety::critical_intensity(double position, double d)
{
    // krivka pozaru
    double epsilon = 1; // emisivita
    double T = fireCurve(m_pv); // Teplota dle krivky pozaru
    double I = epsilon * SIGMA * pow(T, 4); // Salava slozka

    double l = m_width / 2   +  position;    // pozice horizontalni
    double h = m_height / 2;     // pozice vertikalni ve 2D vzdy polovina vysky
    double X = h / d;
    double Y = l / d;


    // if (d == 0) return 0;
    double phi_1 = 1/(2 * M_PI) * (X / sqrt(1 + X * X) * atan( Y / sqrt(1 + X * X)) +
                                   ( Y /  sqrt(1 + Y * Y) * atan( X / sqrt(1 + Y * Y)))) * 2;

    l =  m_width / 2   -  position;    // pozice horizontalni

    X = h / d;
    Y = l / d;


    double phi_2 = 1/(2 * M_PI) * (X / sqrt(1 + X * X) * atan( Y / sqrt(1 + X * X)) +
                                   ( Y /  sqrt(1 + Y * Y) * atan( X / sqrt(1 + Y * Y)))) * 2;

    double I0 = I * (phi_2 + phi_1);
    return  (I0 - I0_LIMIT);
}


double FireSafety::newton(double position, double estimate = 10)
{
    double x = 1e-6;
    double dx = 1e-5;
    double distance = estimate;
    bool found = false ;

    {
        double x = estimate;
        double dx = 1e-5;
        double distance = estimate;

        double I0 = 10;
        int j = 0;
        while ((abs(I0) > 1e-4) && (j < 1000))
        {
            j++;
            I0 = critical_intensity(position, x);
            double dI0 = (critical_intensity(position, x + dx) - I0) / dx;
            x = x - I0 / dI0;

        }

        if (j < 1000)
        {
            if (abs(distance - x) > 1e-10)
            {
                return x;
            }
        }
    }
    return -1;
}

QList<EnvelopePoint> FireSafety::calculateArea()
{
    int N = 150;

    QList<double> positions;


    //    QList<double> positions;
    //    for (int i = 0; i < N; i++)
    //    {
    //        double xl = (double) i / (N-1) * 10.0;
    //        double yl = xl * xl / 10 * 10;
    //        double pos = m_width - m_width * yl;
    //        // qInfo() << xl << yl << pos;
    //        positions.append(pos);
    //    }

    EnvelopePoint point;

    double step = m_width / (N-1);
    point.position = 0;

    double estimate = m_width / 4;
    for(int i = 0; i < N; i++)
    {
        if(m_envelope.length() > 0)
            estimate = m_envelope.last().distance;

        if (step < 1e-3)
        {
            step = - m_width / (N-1);
            point.position += step;
            estimate = m_envelope.last().distance / 2;
        }


        point.distance = newton(point.position, estimate);

        if( point.distance != -1)
        {
            m_envelope.append(point);
            point.position += step;
        }
        else
        {
            point.position -= step;
            step = step / 2;
        }
        qInfo() << step;
        qInfo() << i;
    }

    // add end point
    point.position = m_width / 2;
    point.distance = 0;
    m_envelope.append(point);

    sortEnvelope();
    return m_envelope;
}

bool FireSafety::compare(const EnvelopePoint &s1, const EnvelopePoint &s2)
{
    return s1.distance < s2.distance;
}


void FireSafety::sortEnvelope()
{
    qSort(m_envelope.begin(), m_envelope.end(), FireSafety::compare);
}
