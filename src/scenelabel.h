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

#ifndef SCENELABEL_H
#define SCENELABEL_H

#include "util.h"
#include "scenebasic.h"

class SceneLabelCommandRemove;
class FieldInfo;

class SceneLabel : public MarkedSceneBasic<SceneMaterial>
{
public:
    SceneLabel(const Point &m_point, double m_area);

    inline virtual SceneMaterial* marker(FieldInfo *fieldInfo) { return MarkedSceneBasic<SceneMaterial>::marker(fieldInfo); }
    inline Point point() const { return m_point; }
    inline void setPoint(const Point &point) { m_point = point; }
    inline double area() const { return m_area; }
    inline void setArea(double area) { m_area = area; }

    double distance(const Point &m_point) const;

    SceneLabelCommandRemove* getRemoveCommand();

    int showDialog(QWidget *parent, bool isNew = false);

private:
    Point m_point;
    double m_area;
};

class SceneLabelContainer : public MarkedSceneBasicContainer<SceneMaterial, SceneLabel>
{
public:
    /// if container contains the same label, returns it. Otherwise returns NULL
    SceneLabel* get(SceneLabel* label) const;
    SceneLabel* get(const Point& point) const;

    /// finds label at position i, but NOT COUNTING those labels, that have none marker in this field
    /// "inverse" function to the mean, in which we skip labels with none materials in solver and register weak forms
    /// this aditional mapping of markers between agros and hermes will be rewised when implementing subdomains
    // SceneLabel* atNotNoneHack(int i, FieldInfo* fieldInfo);

};


// *************************************************************************************************************************************

class SceneLabelMarker : public QGroupBox
{
    Q_OBJECT

public:
    SceneLabelMarker(SceneLabel *label, FieldInfo *fieldInfo, QWidget *parent);

    void load();
    bool save();
    void fillComboBox();

private:
    FieldInfo *m_fieldInfo;
    SceneLabel *m_label;

    QComboBox *cmbMaterial;
    QPushButton *btnMaterial;

    QSpinBox *txtPolynomialOrder;
    QCheckBox *chkPolynomialOrder;

private slots:
    void doMaterialChanged(int index);
    void doMaterialClicked();
    void doPolynomialOrder(int);
};

class SceneLabelDialog : public SceneBasicDialog
{
    Q_OBJECT

public:
    SceneLabelDialog(SceneLabel *label, QWidget *parent, bool m_isNew = false);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    ValueLineEdit *txtArea;
    QCheckBox *chkArea;

    QList<SceneLabelMarker *> m_labelMarkers;

    void fillComboBox();

private slots:
    void doArea(int);
};

class SceneLabelSelectDialog : public QDialog
{
    Q_OBJECT

public:
    SceneLabelSelectDialog(MarkedSceneBasicContainer<SceneMaterial, SceneLabel> labels, QWidget *parent);

protected:
    void load();
    bool save();

private:
    MarkedSceneBasicContainer<SceneMaterial, SceneLabel> m_labels;
    QMap<FieldInfo*, QComboBox *> cmbMaterials;

    void fillComboBox();

private slots:
    void doAccept();
    void doReject();
};

// undo framework *******************************************************************************************************************

class SceneLabelCommandAdd : public QUndoCommand
{
public:
    SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
};

class SceneLabelCommandEdit : public QUndoCommand
{
public:
    SceneLabelCommandEdit(const Point &point, const Point &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    Point m_pointNew;
};


#endif // SCENELABEL_H
