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

class SceneBoundary;
class SceneMaterial;

Q_DECLARE_METATYPE(SceneBoundary *);
Q_DECLARE_METATYPE(SceneMaterial *);

class SceneBoundary
{
public:
    QString name;
    PhysicFieldBC type;

    SceneBoundary(const QString &name, PhysicFieldBC type);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    QString html();
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneBoundaryNone : public SceneBoundary
{
public:
    SceneBoundaryNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

class SceneBoundarySelectDialog : public QDialog
{
    Q_OBJECT
public:
    SceneBoundarySelectDialog(QWidget *parent = 0);

    inline SceneBoundary *boundary() { return (cmbBoundary->currentIndex() >= 0) ? cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>() : NULL; }

protected slots:
    void doAccept();

private:
    QComboBox *cmbBoundary;
};

// *************************************************************************************************************************************

class SceneMaterial
{
public:
    QString name;

    SceneMaterial(const QString &name);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    QString html();
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneMaterialNone : public SceneMaterial
{
public:
    SceneMaterialNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

class SceneMaterialSelectDialog : public QDialog
{
    Q_OBJECT
public:
    SceneMaterialSelectDialog(QWidget *parent = 0);

    inline SceneMaterial *marker() { return (cmbMaterial->currentIndex() >= 0) ? cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>() : NULL; }

protected slots:
    void doAccept();

private:
    QComboBox *cmbMaterial;
};

// *************************************************************************************************************************************

class SceneBoundaryDialog: public QDialog
{
    Q_OBJECT

public:
    SceneBoundaryDialog(QWidget *parent);

protected:
    QGridLayout *layout;
    QDialogButtonBox *buttonBox;

    QLineEdit *txtName;
    QLabel *lblEquation;
    QLabel *lblEquationImage;
    SceneBoundary *m_boundary;

    virtual void createContent() = 0;
    void createDialog();

    virtual void load();
    virtual bool save();
    void setSize();

protected slots:
    void evaluated(bool isError);
    void readEquation(QLabel *lblEquation, PhysicFieldBC type);

private slots:
    void doAccept();
    void doReject();
};

class SceneMaterialDialog: public QDialog
{
    Q_OBJECT

public:
    SceneMaterialDialog(QWidget *parent);

protected:
    QGridLayout *layout;
    QDialogButtonBox *buttonBox;

    QLineEdit *txtName;
    QLabel *lblEquation;
    QLabel *lblEquationImage;
    SceneMaterial *m_material;

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
