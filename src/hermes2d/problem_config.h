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

#ifndef PROBLEM_CONFIG_H
#define PROBLEM_CONFIG_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"

class FieldInfo;
class CouplingInfo;

class Field;

class Problem;

//template <typename Scalar>
//class Solver;

class ProblemConfig : public QObject
{
    Q_OBJECT
public:
    ProblemConfig(QWidget *parent = 0);

    inline QString labelX() { return ((m_coordinateType == CoordinateType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((m_coordinateType == CoordinateType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((m_coordinateType == CoordinateType_Planar) ? "Z" : "a");  }

    void clear();

    inline QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    inline QString fileName() const { return m_fileName; }
    void setFileName(const QString &fileName) { m_fileName = fileName; }

    // coordinates
    inline CoordinateType coordinateType() const { return m_coordinateType; }
    void setCoordinateType(const CoordinateType coordinateType) { m_coordinateType = coordinateType; emit changed(); }

    // harmonic problem
    inline double frequency() const { return m_frequency; }
    void setFrequency(const double frequency) { m_frequency = frequency; emit changed(); }

    // transient problem
    inline int numConstantTimeSteps() const { return m_numConstantTimeSteps; }
    void setNumConstantTimeSteps(const int numConstantTimeSteps) { m_numConstantTimeSteps = numConstantTimeSteps; emit changed(); }

    inline Value timeTotal() const { return m_timeTotal; }
    void setTimeTotal(const Value &timeTotal) { m_timeTotal = timeTotal; emit changed(); }

    inline double constantTimeStepLength() { return m_timeTotal.number() / m_numConstantTimeSteps; }
    double initialTimeStepLength();

    inline TimeStepMethod timeStepMethod() const {return m_timeStepMethod; }
    void setTimeStepMethod(TimeStepMethod timeStepMethod) { m_timeStepMethod = timeStepMethod; }
    bool isTransientAdaptive() const;

    int timeOrder() const { return m_timeOrder; }
    void setTimeOrder(int timeOrder) {m_timeOrder = timeOrder; }

    inline Value timeMethodTolerance() const { return m_timeMethodTolerance; }
    void setTimeMethodTolerance(Value timeMethodTolerance) {m_timeMethodTolerance = timeMethodTolerance; }

    // matrix
    inline Hermes::MatrixSolverType matrixSolver() const { return m_matrixSolver; }
    void setMatrixSolver(const Hermes::MatrixSolverType matrixSolver) { m_matrixSolver = matrixSolver; emit changed(); }

    // mesh
    inline MeshType meshType() const { return m_meshType; }
    void setMeshType(const MeshType meshType) { m_meshType = meshType; emit changed(); }

    // startup script
    inline QString startupscript() const { return m_startupscript; }
    void setStartupScript(const QString &startupscript) { m_startupscript = startupscript; emit changed(); }

    // description
    inline QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    void refresh() { emit changed(); }

signals:
    void changed();

private:    QString m_name;
    QString m_fileName;
    CoordinateType m_coordinateType;

    // harmonic
    double m_frequency;

    // transient
    Value m_timeTotal;
    int m_numConstantTimeSteps;
    TimeStepMethod m_timeStepMethod;
    int m_timeOrder;
    Value m_timeMethodTolerance;

    // matrix solver
    Hermes::MatrixSolverType m_matrixSolver;

    // mesh type
    MeshType m_meshType;

    QString m_startupscript;
    QString m_description;

};

#endif // PROBLEM_CONFIG_H
