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

#ifndef SCENETRANSFORMDIALOG_H
#define SCENETRANSFORMDIALOG_H

#include "util.h"
#include "sceneview_geometry.h"

class Scene;
class ValueLineEdit;
class SceneViewPreprocessor;

class SceneTransformDialog : public QDialog
{
    Q_OBJECT
public:
    SceneTransformDialog(SceneViewPreprocessor *sceneViewPreprocessor, QWidget *parent = 0);
    ~SceneTransformDialog();

    void showDialog();

private slots:
    void doClose();
    void doTransform();

protected:
    void createControls();

private:
    SceneViewPreprocessor *m_sceneViewPreprocessor;

    QTabWidget *tabWidget;

    QWidget *widTranslate;
    QWidget *widRotate;
    QWidget *widScale;

    QLabel *lstTranslateX;
    QLabel *lstTranslateY;
    ValueLineEdit *txtTranslateY;
    ValueLineEdit *txtTranslateX;

    QLabel *lstRotateBasePointX;
    QLabel *lstRotateBasePointY;
    ValueLineEdit *txtRotateBasePointX;
    ValueLineEdit *txtRotateBasePointY;
    ValueLineEdit *txtRotateAngle;

    QLabel *lstScaleBasePointX;
    QLabel *lstScaleBasePointY;
    ValueLineEdit *txtScaleBasePointX;
    ValueLineEdit *txtScaleBasePointY;
    ValueLineEdit *txtScaleFactor;

    QCheckBox *chkCopy;
};

#endif // SCENETRANSFORMDIALOG_H
