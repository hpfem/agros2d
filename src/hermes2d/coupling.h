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
#include "hermes2d/module.h"

class FieldInfo;
class ProblemConfig;
class PluginInterface;

// available couplings
AGROS_API QMap<QString, QString> availableCouplings();

AGROS_API bool isCouplingAvailable(FieldInfo* sourceField, FieldInfo* targetField);

class AGROS_API CouplingInfo
{
public:
    CouplingInfo(FieldInfo* sourceField, FieldInfo* targetField,
                 CouplingType couplingType = CouplingType_Weak);
    ~CouplingInfo();

    inline PluginInterface *plugin() const { assert(m_plugin); return m_plugin; }

    QString couplingId() { return m_couplingId; }

    CouplingType couplingType() { return m_couplingType; }
    void setCouplingType(CouplingType couplingType);

    inline bool isHard() { return m_couplingType == CouplingType_Hard;}
    inline bool isWeak() { return m_couplingType == CouplingType_Weak;}

    inline FieldInfo* sourceField() {return m_sourceField; }
    inline FieldInfo* targetField() {return m_targetField; }

    bool isRelated(FieldInfo* fieldInfo) { return((fieldInfo == sourceField()) || (fieldInfo == targetField())); }

    /// reloads the Coupling ("module"). Should be called when couplingType or AnalysisType of either fields changes
    void reload();

    LinearityType linearityType();

    // xml coupling

    // name
    QString name() const;
    // description
    QString description() const;

    // constants
    QMap<QString, double> constants();

    // weak forms
    QList<FormInfo> wfMatrixVolume() const;
    QList<FormInfo> wfVectorVolume() const;

private:
    /// plugin
    PluginInterface *m_plugin;

    /// unique field info
    QString m_couplingId;

    /// pointers to problem infos
    FieldInfo* m_sourceField;
    FieldInfo* m_targetField;

    /// coupling type
    CouplingType m_couplingType;

signals:
    void invalidated();
};

//}


#endif // COUPLING_H
