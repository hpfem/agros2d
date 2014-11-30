// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SCENELABEL_H
#define SCENELABEL_H

#include "util.h"
#include "scenebasic.h"
#include "scenemarkerdialog.h"

class SceneLabelCommandAdd;
class SceneLabelCommandRemove;
class FieldInfo;

class AGROS_LIBRARY_API SceneLabel : public MarkedSceneBasic<SceneMaterial>
{
public:
    SceneLabel(const Point &point, double area);
    SceneLabel(const PointValue &pointValue, double area);

    inline virtual SceneMaterial* marker(const FieldInfo *fieldInfo) { return MarkedSceneBasic<SceneMaterial>::marker(fieldInfo); }

    inline Point point() const { return m_point.point(); }
    inline PointValue pointValue() const { return m_point; }
    void setPointValue(const PointValue &point);

    inline double area() const { return m_area; }
    inline void setArea(double area) { m_area = area; }

    double distance(const Point &m_point) const;

    SceneLabelCommandAdd* getAddCommand();
    SceneLabelCommandRemove* getRemoveCommand();

    // returns true if markers for all fields are noneMarkers
    bool isHole();

    int showDialog(QWidget *parent, bool isNew = false);

    static SceneLabel *findClosestLabel(const Point &point);

    void addMarkersFromStrings(QMap<QString, QString> markers);

private:
    PointValue m_point;

    double m_area;
};

Q_DECLARE_METATYPE(SceneLabel *)

class SceneLabelContainer : public MarkedSceneBasicContainer<SceneMaterial, SceneLabel>
{
public:
    /// if container contains object with the same coordinates as label, returns it. Otherwise returns NULL
    SceneLabel* get(SceneLabel* label) const;

    /// returns label with given coordinates or NULL
    SceneLabel* get(const Point& point) const;

    /// returns bounding box, assumes container not empty
    RectPoint boundingBox() const;
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

    QSpinBox *txtAreaRefinement;
    QCheckBox *chkAreaRefinement;

    QSpinBox *txtPolynomialOrder;
    QCheckBox *chkPolynomialOrder;

private slots:
    void doMaterialChanged(int index);
    void doMaterialClicked();
    void doAreaRefinement(int);
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
    QMap<const FieldInfo*, QComboBox *> cmbMaterials;

    void fillComboBox();

private slots:
    void doAccept();
    void doReject();
};

// undo framework *******************************************************************************************************************

class SceneLabelCommandAdd : public QUndoCommand
{
public:
    SceneLabelCommandAdd(const PointValue &pointValue, const QMap<QString, QString> &markers, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;

    QMap<QString, QString> m_markers;
    double m_area;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const PointValue &pointValue, const QMap<QString, QString> &markers, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;

    QMap<QString, QString> m_markers;
    double m_area;
};

class SceneLabelCommandEdit : public QUndoCommand
{
public:
    SceneLabelCommandEdit(const PointValue &point, const PointValue &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
    PointValue m_pointNew;
};

class SceneLabelCommandAddOrRemoveMulti : public QUndoCommand
{
public:
    SceneLabelCommandAddOrRemoveMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers, QList<double> areas, QUndoCommand *parent = 0);
    void add();
    void remove();

private:
    // nodes
    QList<PointValue> m_points;
    QList<QMap<QString, QString> > m_markers;
    QList<double> m_areas;
};

class SceneLabelCommandAddMulti : public SceneLabelCommandAddOrRemoveMulti
{
public:
    SceneLabelCommandAddMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers,  QList<double> areas, QUndoCommand *parent = 0) :
        SceneLabelCommandAddOrRemoveMulti(points, markers, areas, parent) {}

    void undo() { remove(); }
    void redo() { add(); }
};

class SceneLabelCommandRemoveMulti : public SceneLabelCommandAddOrRemoveMulti
{
public:
    SceneLabelCommandRemoveMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers,  QList<double> areas, QUndoCommand *parent = 0) :
        SceneLabelCommandAddOrRemoveMulti(points, markers, areas, parent) {}

    void undo() { add(); }
    void redo() { remove(); }
};

class SceneLabelCommandMoveMulti : public QUndoCommand
{
public:
    SceneLabelCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    static void moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo);

    QList<PointValue> m_points;
    QList<PointValue> m_pointsNew;
};

#endif // SCENELABEL_H
