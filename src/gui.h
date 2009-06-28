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

void fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField);

class SLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    SLineEdit(QWidget *parent = 0);
    SLineEdit(const QString &contents, bool hasValidator = true, QWidget *parent = 0);

    double value();
    void setValue(double value);
};

class SLineEditValue : public QLineEdit
{
    Q_OBJECT
public:
    SLineEditValue(QWidget *parent = 0);

    Value value();
    double number();
    bool evaluate();
    void setValue(Value value);
private:
    double m_number;
};

#endif // GUI_H
