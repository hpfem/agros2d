// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef UTIL_POINT_H
#define UTIL_POINT_H

#include <QtCore>
#include <cmath>
#include <iostream>

struct Point;

// return center
Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle);

// intersection of two lines
bool intersectionLines(const Point &p1s, const Point &p1e, const Point &p2s, const Point &p2e, Point &out);

// intersection of two lines or line and arc
QList<Point> intersection(const Point &p1s, const Point &p1e, const Point &center1, double radius1, double angle1,
                          const Point &p2s, const Point &p2e, const Point &center2, double radius2, double angle2);

struct Point
{
    double x, y;

    Point() { this->x = 0; this->y = 0; }
    Point(double x, double y) { this->x = x; this->y = y; }

    inline Point operator+(const Point &vec) const { return Point(x + vec.x, y + vec.y); }
    inline Point operator-(const Point &vec) const { return Point(x - vec.x, y - vec.y); }
    inline Point operator*(double num) const { return Point(x * num, y * num); }
    inline Point operator/(double num) const { return Point(x / num, y / num); }
    inline double operator&(const Point &vec) const { return x*vec.x + y*vec.y; } // dot product
    inline double operator%(const Point &vec) const { return x*vec.y - y*vec.x; } // cross product
    inline bool operator!=(const Point &vec) const { return ((fabs(vec.x-x) > 1e-12) || (fabs(vec.y-y) > 1e-12)); }
    inline bool operator==(const Point &vec) const { return ((fabs(vec.x-x) < 1e-12) && (fabs(vec.y-y) < 1e-12)); }

    inline double magnitude() const { return sqrt(x * x + y * y); }
    inline double magnitudeSquared() const { return (x * x + y * y); }
    inline double angle() const { return atan2(y, x); }

    Point normalizePoint() const
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;

        return Point(mx, my);
    }

    QString toString() const
    {
        return QString("x = %1, y = %2, magnitude = %3").
                arg(x).
                arg(y).
                arg(magnitude());
    }
};

QDebug& operator<<(QDebug output, const Point& pt);

struct Point3
{
    double x, y, z;

    Point3() { this->x = 0; this->y = 0; this->z = 0; }
    Point3(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }

    inline Point3 operator+(const Point3 &vec) const { return Point3(x + vec.x, y + vec.y, z + vec.z); }
    inline Point3 operator-(const Point3 &vec) const { return Point3(x - vec.x, y - vec.y, z - vec.z); }
    inline Point3 operator*(double num) const { return Point3(x * num, y * num, z * num); }
    inline Point3 operator/(double num) const { return Point3(x / num, y / num, z / num); }
    inline double operator&(const Point3 &vec) const { return x*vec.x + y*vec.y + z*vec.z; } // dot product
    inline Point3 operator%(const Point3 &vec) const { return Point3(- z*vec.y, z*vec.x, x*vec.y - y*vec.x); } // cross product

    inline double magnitude() const { return sqrt(x * x + y * y); }
    inline double anglexy() const { return atan2(y, x); }
    inline double angleyz() const { return atan2(z, y); }
    inline double anglezx() const { return atan2(x, z); }

    Point3 normalizePoint() const
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;
        double mz = z/m;

        return Point3(mx, my, mz);
    }

    QString toString()
    {
        return QString("x = %1, y = %2, z = %3, magnitude = %4").
                arg(x).
                arg(y).
                arg(z).
                arg(magnitude());
    }
};

struct RectPoint
{
    Point start;
    Point end;

    inline RectPoint(const Point &start, const Point &end) { this->start = start; this->end = end; }
    inline RectPoint() { this->start = Point(); this->end = Point(); }

    inline void set(const Point &start, const Point &end) { this->start = start; this->end = end; }
    inline double width() const { return fabs(end.x - start.x); }
    inline double height() const { return fabs(end.y - start.y); }
};

#endif // UTIL_POINT_H

