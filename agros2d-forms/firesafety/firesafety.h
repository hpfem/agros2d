#ifndef FIRESAFETY_H
#define FIRESAFETY_H
#include <QList>

class FireSafety;

struct EnvelopePoint
{
    double distance;
    double position;
};

enum FireCurve
{
    FireCurve_ISO
};

class FireProperty
{
public:
    FireProperty(double width = 3.0, double height = 3.0, double pv = 90.0, FireCurve fireCurve = FireCurve_ISO, double i0 = 18500, double eps = 1.0, double increase = 0.0);
    virtual ~FireProperty();

    inline void setWidth(double width) { m_width = width; }
    inline void setHeight(double height) { m_height = height; }
    inline void setPv(double pv) { m_pv = pv; }
    inline void setIncrease(double increase) { m_increase = increase; }
    inline void setI0(double i0) { m_i0 = i0; }
    inline void setEmisivity(double eps) { m_eps = eps; }
    inline void setFireCurve(FireCurve curve) { m_fireCurve = curve; }

    inline double width() { return m_width; }
    inline double height() { return m_height; }
    inline double pv() { return m_pv; }
    inline double increase() { return m_increase; }
    inline double i0() { return m_i0; }
    inline double emisivity() { return m_eps; }
    inline FireCurve fireCurve() { return m_fireCurve; }

    double fireCurveValue();

    inline FireSafety *fs() { return m_fs; }

private:
    double m_width;
    double m_height;
    double m_pv;
    double m_increase;
    double m_i0;
    double m_eps;
    FireCurve m_fireCurve;

    FireSafety *m_fs;
};

class FireSafety
{
public:
    FireSafety(FireProperty fp);

    double critical_intensity(double position, double d);
    double newton(double position, double estimate);
    QList<EnvelopePoint> calculateArea();
    void sortEnvelope();

    static bool compare(const EnvelopePoint &s1, const EnvelopePoint &s2);

private:
    static const double I0_LIMIT;
    static const double SIGMA;

    FireProperty m_fp;

    QList<EnvelopePoint> m_envelope;
};

#endif // FIRESAFETY_H
