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

#ifndef COUPLING_H
#define COUPLING_H

#include "util.h"
#include "util/enums.h"

#include "hermes2d.h"

class FieldInfo;
class ProblemConfig;
class FormInfo;

// available couplings
QMap<QString, QString> availableCouplings();

struct Coupling
{
    Coupling(const QString &couplingId,
             CoordinateType coordinateType, CouplingType couplingType,
             AnalysisType sourceFieldAnalysis, AnalysisType targetFieldAnalysis);
    ~Coupling();

    // id
    inline QString couplingId() const { return m_couplingId; }
    // name
    inline QString name() const { return m_name; }
    // description
    inline QString description() const { return m_description; }

    // constants
    inline QMap<QString, double> constants() const { return m_constants; }

    // weak forms
    inline QList<FormInfo *> wfMatrixVolumeExpression() const { return m_wfMatrixVolumeExpression; }
    inline QList<FormInfo *> wfVectorVolumeExpression() const { return m_wfVectorVolumeExpression; }

    inline CoordinateType coordinateType() const { return m_coordinateType; }

private:
    CouplingType m_couplingType;
    CoordinateType m_coordinateType;

    AnalysisType m_sourceFieldAnalysis;
    AnalysisType m_targetFieldAnalysis;

    // id
    QString m_couplingId;
    // name
    QString m_name;
    // description
    QString m_description;

    // constants
    QMap<QString, double> m_constants;

    // weak forms
    QList<FormInfo *> m_wfMatrixVolumeExpression;
    QList<FormInfo *> m_wfVectorVolumeExpression;

    void read(const QString &filename);
    void clear();
};

bool isCouplingAvailable(FieldInfo* sourceField, FieldInfo* targetField);

// coupling factory
Coupling *couplingFactory(FieldInfo* sourceField, FieldInfo* targetField, CouplingType couplingType);

class CouplingInfo
{

signals:
    void invalidated();

public:
    CouplingInfo(FieldInfo* sourceField, FieldInfo* targetField,
                 CouplingType couplingType = CouplingType_Weak);
    ~CouplingInfo();

    inline Coupling *coupling() const { return m_coupling; }

    QString couplingId() { if (m_coupling) return m_coupling->couplingId(); return "None"; }
    CouplingType couplingType() { return m_couplingType; }
    inline bool isHard() { return m_couplingType == CouplingType_Hard;}
    inline bool isWeak() { return m_couplingType == CouplingType_Weak;}
    void setCouplingType(CouplingType couplingType);

    inline FieldInfo* sourceField() {return m_sourceField; }
    inline FieldInfo* targetField() {return m_targetField; }

    bool isRelated(FieldInfo* fieldInfo) { return((fieldInfo == sourceField()) || (fieldInfo == targetField())); }

    /// reloads the Coupling ("module"). Should be called when couplingType or AnalysisType of either fields changes
    void reload();

    LinearityType linearityType();

private:
    /// module
    Coupling *m_coupling;

    /// pointer to problem info
    ProblemConfig *m_problemInfo;

    FieldInfo* m_sourceField;
    FieldInfo* m_targetField;

    /// coupling type
    CouplingType m_couplingType;
};

//}


#endif // COUPLING_H
