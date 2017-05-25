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
    FireSafety(double width, double height, double pv);
    double critical_intensity(double position, double d);
    double newton(double position, double estimate);
    double fireCurve(double T);
    QList<EnvelopePoint> calculateArea();
    static bool compare(const EnvelopePoint &s1, const EnvelopePoint &s2);
    void sortEnvelope();
    void setWidth (double width) {m_width = width;}
    void setHight (double height) {m_height = height;}
    void setPv (double pv) {m_pv = pv;}


private:
    static const double I0_LIMIT;
    static const double SIGMA;
    double m_width;
    double m_height;
    double m_pv;
    QList<EnvelopePoint> m_envelope;
};

#endif // FIRESAFETY_H
