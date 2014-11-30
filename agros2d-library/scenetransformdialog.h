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
    void doCopyChecked(bool checked);

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
    QCheckBox *chkWithMarkers;
};

#endif // SCENETRANSFORMDIALOG_H
