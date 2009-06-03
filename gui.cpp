#include "gui.h"

SLineEdit::SLineEdit(QWidget *parent) : QLineEdit(parent)
{
    SLineEdit::SLineEdit("", true, parent);
}

SLineEdit::SLineEdit(const QString &contents, bool hasValidator, QWidget *parent) : QLineEdit(contents, parent)
{
    if (hasValidator)
        this->setValidator(new QDoubleValidator(this));
}

double SLineEdit::value()
{
    QScriptEngine engine;

    if (engine.evaluate(text()).isNumber())
        return engine.evaluate(text()).toNumber();
}
