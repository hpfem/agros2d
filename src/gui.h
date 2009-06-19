#ifndef GUI_H
#define GUI_H

#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QScriptEngine>
#include <QDoubleValidator>

#include "util.h"

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

#endif // GUI_H
