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

#include "point.h"

#include "util.h"

Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle)
{
    double distance = (pointEnd - pointStart).magnitude();
    Point t = (pointEnd - pointStart) / distance;
    double R = distance / (2.0*sin(angle/180.0*M_PI / 2.0));

    Point p = Point(distance/2.0, sqrt(R*R - distance*distance/4.0 > 0.0 ? R*R - distance*distance/4.0 : 0.0));
    Point center = pointStart + Point(p.x*t.x - p.y*t.y, p.x*t.y + p.y*t.x);

    return center;
}

//   Function tests if angle specified by the third parameter is between angles
//   angle_1 and angle_2
bool isBetween(double angle_1, double angle_2, double angle)
{
    // tolerance 1e-3 degree
    double tol = 1e-3 * M_PI / 180;
#ifndef M_PI_2
#define M_PI_2 M_PI / 2
#endif

    // angle_1 has to be lower then angle_2
    if (angle_2 < angle_1)
    {
        double temp = angle_1;
        angle_1 = angle_2;
        angle_2 = temp;
    }

    // angle_1 is in the third quadrant, angle_2 is in the second quadrant
    if ((angle_1 <= - (M_PI_2 - tol) ) && (angle_2 >= (M_PI_2 - tol)))
    {
        if((angle <= angle_1) || (angle >= angle_2))
        {
           return true;
        }

    } else
    {
        if((angle >= angle_1) && (angle <= angle_2))
        {
           return true;
        }
    }
    return false;
}

bool intersection(Point p, Point p1s, Point p1e)
{
    Point dv_line = p1e - p1s;
    Point dv_point = p1e - p;
    if ((dv_line.angle() - dv_point.angle()) < EPS_ZERO)
        return true;
    else
        return false;
}

Point *intersection(Point p1s, Point p1e, Point p2s, Point p2e)
{
    double denom = (p2e.y-p2s.y)*(p1e.x-p1s.x) - (p2e.x-p2s.x)*(p1e.y-p1s.y);

    double nume_a = ((p2e.x-p2s.x)*(p1s.y-p2s.y) - (p2e.y-p2s.y)*(p1s.x-p2s.x));
    double nume_b = ((p1e.x-p1s.x)*(p1s.y-p2s.y) - (p1e.y-p1s.y)*(p1s.x-p2s.x));

    double ua = nume_a / denom;
    double ub = nume_b / denom;

    if ((p2e != p1s) && (p1e != p2s) && (p1e != p2e) && (p1s != p2s))
    {
        if ((abs(denom) > EPS_ZERO) && (ua >= 0.0) && (ua <= 1.0) && (ub >= 0.0) && (ub <= 1.0))
        {
            double xi = p1s.x + ua*(p1e.x - p1s.x);
            double yi = p1s.y + ua*(p1e.y - p1s.y);

            return new Point(xi, yi);
        }
    }

    return NULL;
}

QList<Point> intersection(Point p1s, Point p1e, Point center1, double radius1, double angle1,
                          Point p2s, Point p2e, Point center2, double radius2, double angle2)
{
    QList<Point> out;

    double dx = p1e.x - p1s.x;      // component of direction vector of the line
    double dy = p1e.y - p1s.y;      // component of direction vector of the line
    double a = dx * dx + dy * dy;   // square of the length of direction vector
    double tol = sqrt(a)/1e4;

    // Crossing of two arcs
    if ((angle1 > 0.0) && (angle2 > 0.0))
    {
        if (((p1s == p2e) && (p1e == p2s)) || ((p1e == p2e) && (p1s == p2s)))
        {
            // Crossing of arcs is not possible
        }
        else
        {
            // Calculate distance between centres of circle
            float distance = (center1 - center2).magnitude();
            float dx = center2.x - center1.x;
            float dy = center2.y - center1.y;

            if ((distance < (radius1 + radius2)))
            {
                // Determine the distance from point 0 to point 2.
                double a = ((radius1*radius1) - (radius2*radius2) + (distance*distance)) / (2.0 * distance);

                // Determine the coordinates of point 2.
                Point middle;
                middle.x = center1.x + (dx * a/distance);
                middle.y = center1.y + (dy * a/distance);

                // Determine the distance from point 2 to either of the
                // intersection points.
                double h = std::sqrt((radius1 * radius1) - (a*a));

                // Now determine the offsets of the intersection points from
                // point 2.
                double rx = -dy * (h/distance);
                double ry =  dx * (h/distance);

                // Determine the absolute intersection points.
                Point p1(middle.x + rx, middle.y + ry);
                Point p2(middle.x - rx, middle.y - ry);

                // Angles of starting and end points due to center of the arc 1
                double angle1_1 = (p1e - center1).angle();
                double angle2_1 = (p1s - center1).angle();

                // Angles of starting and end points due to center of the arc 2
                double angle1_2 = (p2e - center2).angle();
                double angle2_2 = (p2s - center2).angle();

                // Angles of intersection points due to center of the arc 1
                double iangle1_1 = (p1 - center1).angle();
                double iangle2_1 = (p2 - center1).angle();

                // Angles of intersection points due to center of the arc 2
                double iangle1_2 = (p1 - center2).angle();
                double iangle2_2 = (p2 - center2).angle();

                // Test if intersection 1 lies on arc
                if ((isBetween(angle1_1, angle2_1, iangle1_1) && isBetween(angle1_2 , angle2_2, iangle1_2)))
                {
                    if (((p1 - p1s).magnitude() > tol) &&
                            ((p1 - p1e).magnitude() > tol) &&
                            ((p1 - p2e).magnitude() > tol) &&
                            ((p1 - p2s).magnitude() > tol))
                        out.append(p1);
                }

                // Test if intersection 1 lies on arc
                if (isBetween(angle1_1, angle2_1, iangle2_1) && isBetween(angle1_2, angle2_2, iangle2_2))
                {
                    if(((p2 - p1s).magnitude() > tol) &&
                            ((p2 - p1e).magnitude() > tol) &&
                            ((p2 - p2e).magnitude() > tol) &&
                            ((p2 - p2s).magnitude() > tol))
                        out.append(p2);
                }
            }
        }
    }
    else
    {
        if (angle2 > 0.0)
            // crossing of arc and line
        {
            double b = 2 * (dx * (p1s.x - center2.x) + dy * (p1s.y - center2.y));
            double c = p1s.x * p1s.x + p1s.y * p1s.y + center2.x * center2.x + center2.y * center2.y - 2 * (center2.x * p1s.x + center2.y * p1s.y)-(radius2 * radius2);

            double bb4ac = b * b - 4 * a * c;

            double mu1 = (-b + sqrt(bb4ac)) / (2*a);
            double mu2 = (-b - sqrt(bb4ac)) / (2*a);

            double i1x = p1s.x + mu1*(dx);
            double i1y = p1s.y + mu1*(dy);

            double i2x = p1s.x + mu2*(dx);
            double i2y = p1s.y + mu2*(dy);


            Point p1(i1x, i1y);     // possible intersection point
            Point p2(i2x, i2y);     // possible intersection point

            double t1;
            double t2;

            if (std::abs(dx - dy) > tol)
            {
                t1 = (p1.x - p1s.x - p1.y + p1s.y) / (dx - dy); // tangent
                t2 = (p2.x - p1s.x - p2.y + p1s.y) / (dx - dy); // tangent
            }
            else
            {
                t1 = (p1.x - p1s.x) / dx; // tangent
                t2 = (p2.x - p1s.x) / dx; // tangent
            }

            double angle_1 = (p2e - center2).angle();
            double angle_2 = (p2s - center2).angle();
            double iangle1 = (p1 - center2).angle();
            double iangle2 = (p2 - center2).angle();


            if ((t1 >= 0) && (t1 <= 1))
            {
                // 1 solution: One Point in the circle
                if (isBetween(angle_1, angle_2, iangle1) && ((p1 - p2s).magnitude() > tol) && ((p1 - p2e).magnitude() > tol))
                    out.append(p1);
            }

            if ((t2 >= 0) && (t2 <= 1))
            {
                // 1 solution: One Point in the circle
                if (isBetween(angle_1, angle_2, iangle2) && ((p2 -  p2s).magnitude() > tol) && ((p2 - p2e).magnitude() > tol))
                    out.append(p2);
            }
        }
        else
        {
            // straight line
            Point *point = intersection(p1s, p1e, p2s, p2e);
            if (point)
                out.append(Point(point->x, point->y));
            delete point;
        }
    }
    return out;
}
