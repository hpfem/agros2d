#ifndef FIRESAFETY_H
#define FIRESAFETY_H
#include <QList>

struct EnvelopePoint
{
    double distance;
    double position;
};

class FireSafety
{
public:    
    enum FireCurve
    {
        FireCurve_ISO
    };

    FireSafety(double width, double height, double pv, FireSafety::FireCurve fireCurve, double increase);

    double critical_intensity(double position, double d);
    double newton(double position, double estimate);
    double fireCurve(double T);
    QList<EnvelopePoint> calculateArea();
    void sortEnvelope();
    void setWidth (double width) {m_width = width;}
    void setHight (double height) {m_height = height;}
    void setPv (double pv) {m_pv = pv;}

    static bool compare(const EnvelopePoint &s1, const EnvelopePoint &s2);

private:
    static const double I0_LIMIT;
    static const double SIGMA;

    double m_width;
    double m_height;
    double m_pv;
    double m_increase;
    FireSafety::FireCurve m_fireCurve;

    QList<EnvelopePoint> m_envelope;
};

#endif // FIRESAFETY_H
