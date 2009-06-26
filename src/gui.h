#ifndef GUI_H
#define GUI_H

#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QScriptEngine>
#include <QScriptValue>
#include <QDoubleValidator>

#include "util.h"
#include "scene.h"

class Scene;

void fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField);

class SLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    SLineEdit(QWidget *parent = 0);
    SLineEdit(const QString &contents, bool hasValidator = true, QWidget *parent = 0);

    double value();
    double value(const QString &script);
    void setValue(double value);
private:
    QString m_scriptStartup;
};

#endif // GUI_H
