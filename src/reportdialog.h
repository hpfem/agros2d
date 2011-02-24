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

#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include "util.h"

class SLineEditValue;

class SceneView;

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    ReportDialog(SceneView *sceneView, QWidget *parent = 0);
    ~ReportDialog();

    void showDialog();

private:
    SceneView *m_sceneView;

    void createControls();
    void setControls();

    void generateIndex();
    void generateFigures();

    QCheckBox *chkDescription;
    QCheckBox *chkProblemInformation;
    QCheckBox *chkStartupScript;
    QCheckBox *chkPhysicalProperties;
    QCheckBox *chkGeometry;
    QCheckBox *chkMeshAndSolution;
    QCheckBox *chkScript;

    QCheckBox *chkFigureGeometry;
    QCheckBox *chkFigureMesh;
    QCheckBox *chkFigureOrder;
    QCheckBox *chkFigureScalarView;
    QCheckBox *chkShowGrid;
    QCheckBox *chkShowRulers;

    QLineEdit *txtTemplate;
    QLineEdit *txtStyleSheet;

    SLineEditValue *txtFigureWidth;
    SLineEditValue *txtFigureHeight;

    QPushButton *btnShowReport;
    QPushButton *btnDefault;
    QPushButton *btnClose;

    QString replaceTemplates(const QString &source);
    QString htmlFigure(const QString &fileName, const QString &caption);
    QString htmlMaterials();
    QString htmlBoundaries();
    QString htmlGeometryNodes();
    QString htmlGeometryEdges();
    QString htmlGeometryLabels();

private slots:
    void resetControls();
    void defaultValues();
    void checkPaths();

    void doClose();
    void doShowReport();
};

#endif // REPORTDIALOG_H
