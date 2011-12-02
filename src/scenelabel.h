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

class SceneLabel : public MarkedSceneBasic<SceneMaterial>
{
public:
    Point point;
    double area;
    int polynomialOrder;

    SceneLabel(const Point &point, double area, int polynomialOrder);

    double distance(const Point &point) const;

    SceneLabelCommandRemove* getRemoveCommand();

    int showDialog(QWidget *parent, bool isNew = false);
};

class SceneLabelContainer : public MarkedSceneBasicContainer<SceneMaterial, SceneLabel>
{
public:
    /// if container contains the same label, returns it. Otherwise returns NULL
    SceneLabel* get(SceneLabel* label) const;
    SceneLabel* get(const Point& point) const;

};


// *************************************************************************************************************************************

class SceneLabelDialog : public DSceneBasic
{
    Q_OBJECT

public:
    SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew = false);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    ValueLineEdit *txtArea;
    QSpinBox *txtPolynomialOrder;
    QCheckBox *chkArea;
    QCheckBox *chkPolynomialOrder;

    QList<QComboBox *> cmbMaterials;
    QList<QPushButton *> btnMaterials;

    void fillComboBox();

private slots:
    void doMaterialChanged(int index);
    void doMaterialClicked();
    void doArea(int);
    void doPolynomialOrder(int);
};


// undo framework *******************************************************************************************************************

class SceneLabelCommandAdd : public QUndoCommand
{
public:
    SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
    int m_polynomialOrder;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
    int m_polynomialOrder;
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
