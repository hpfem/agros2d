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

#ifndef SCENEMARKER_H
#define SCENEMARKER_H

#include "util.h"

class SLineEditDouble;
class ValueLineEdit;

class SceneEdgeMarker;
class SceneLabelMarker;

Q_DECLARE_METATYPE(SceneEdgeMarker *);
Q_DECLARE_METATYPE(SceneLabelMarker *);

class SceneEdgeMarker
{
public:
    QString name;
    PhysicFieldBC type;

    SceneEdgeMarker(const QString &name, PhysicFieldBC type);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    QString html();
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneEdgeMarkerNone : public SceneEdgeMarker
{
public:
    SceneEdgeMarkerNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

class EdgeMarkerDialog : public QDialog
{
    Q_OBJECT
public:
    EdgeMarkerDialog(QWidget *parent = 0);
    ~EdgeMarkerDialog();

    inline SceneEdgeMarker *marker() { return (cmbMarker->currentIndex() >= 0) ? cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>() : NULL; }

protected slots:
    void doAccept();

private:
    QComboBox *cmbMarker;
};

// *************************************************************************************************************************************

class SceneLabelMarker
{
public:
    QString name;

    SceneLabelMarker(const QString &name);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    QString html();
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneLabelMarkerNone : public SceneLabelMarker
{
public:
    SceneLabelMarkerNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

class LabelMarkerDialog : public QDialog
{
    Q_OBJECT
public:
    LabelMarkerDialog(QWidget *parent = 0);
    ~LabelMarkerDialog();

    inline SceneLabelMarker *marker() { return (cmbMarker->currentIndex() >= 0) ? cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>() : NULL; }

protected slots:
    void doAccept();

private:
    QComboBox *cmbMarker;
};

// *************************************************************************************************************************************

class DSceneEdgeMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneEdgeMarker(QWidget *parent);
    ~DSceneEdgeMarker();

protected:
    QGridLayout *layout;
    QDialogButtonBox *buttonBox;

    QLineEdit *txtName;
    QLabel *lblEquation;
    QLabel *lblEquationImage;
    SceneEdgeMarker *m_edgeMarker;

    virtual void createContent() = 0;
    void createDialog();

    virtual void load();
    virtual bool save();
    void setSize();

protected slots:
    void evaluated(bool isError);

private slots:
    void doAccept();
    void doReject();
};

class DSceneLabelMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneLabelMarker(QWidget *parent);
    ~DSceneLabelMarker();

protected:
    QGridLayout *layout;
    QDialogButtonBox *buttonBox;

    QLineEdit *txtName;
    QLabel *lblEquation;
    QLabel *lblEquationImage;
    SceneLabelMarker *m_labelMarker;

    virtual void createContent() = 0;
    void createDialog();
    void setSize();

    virtual void load();
    virtual bool save();

protected slots:
    void evaluated(bool isError);

private slots:
    void doAccept();
    void doReject();
};

#endif // SCENEMARKER_H
