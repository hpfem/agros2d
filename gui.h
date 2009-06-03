#ifndef GUI_H
#define GUI_H

#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QScriptEngine>
#include <QDoubleValidator>

class SLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    SLineEdit(QWidget *parent = 0);
    SLineEdit(const QString &contents, bool hasValidator = true, QWidget *parent = 0);

    double value();
};

#endif // GUI_H
