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
#include "scenemarkerdialog.h"

class SceneLabelCommandAdd;
class SceneLabelCommandRemove;
class FieldInfo;

class AGROS_API SceneLabel : public MarkedSceneBasic<SceneMaterial>
{
public:
    SceneLabel(const Point &point, double area);

    inline virtual SceneMaterial* marker(FieldInfo *fieldInfo) { return MarkedSceneBasic<SceneMaterial>::marker(fieldInfo); }
    inline Point point() const { return m_point; }
    inline void setPoint(const Point &point) { m_point = point; }
    inline double area() const { return m_area; }
    inline void setArea(double area) { m_area = area; }

    double distance(const Point &m_point) const;

    SceneLabelCommandAdd* getAddCommand();
    SceneLabelCommandRemove* getRemoveCommand();

    // returns true if markers for all fields are noneMarkers
    bool isHole();

    int showDialog(QWidget *parent, bool isNew = false);

    static SceneLabel *findClosestLabel(const Point &point);

private:
    Point m_point;
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
    SceneLabelCommandAdd(const Point &point, const QMap<QString, QString> &markers, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QMap<QString, QString> m_markers;
    double m_area;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const Point &point, const QMap<QString, QString> &markers, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QMap<QString, QString> m_markers;
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

class SceneLabelCommandRemoveMulti : public QUndoCommand
{
public:
    SceneLabelCommandRemoveMulti(QList<Point> points, QList<QMap<QString, QString> > markers, QList<double> areas, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // nodes
    QList<Point> m_points;
    QList<QMap<QString, QString> > m_markers;
    QList<double> m_areas;
};

class SceneLabelCommandMoveMulti : public QUndoCommand
{
public:
    SceneLabelCommandMoveMulti(QList<Point> points, QList<Point> pointsNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    static void moveAll(QList<Point> moveFrom, QList<Point> moveTo);

    QList<Point> m_points;
    QList<Point> m_pointsNew;
};

class SceneLabelCommandAddMulti : public QUndoCommand
{
public:
    SceneLabelCommandAddMulti(QList<Point> points, QList<QMap<QString, QString> > markers,  QList<double> areas, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QList<Point> m_points;
    QList<double> m_areas;
    QList<QMap<QString, QString> > m_markers;
};

#endif // SCENELABEL_H
