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

#include "valuelineedit.h"

#include "util/global.h"
#include "util/conf.h"
#include "gui/valuetimedialog.h"
#include "gui/valuedatatabledialog.h"
#include "gui/chart.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "pythonlab/pythonengine.h"
#include "pythonlab/pythonengine_agros.h"


ValueLineEdit::ValueLineEdit(QWidget *parent, bool hasTimeDep, bool hasNonlin)
    : QWidget(parent), m_hasTimeDep(hasTimeDep), m_hasNonlin(hasNonlin),
      m_minimum(-numeric_limits<double>::max()),
      m_minimumSharp(-numeric_limits<double>::max()),
      m_maximum(numeric_limits<double>::max()),
      m_maximumSharp(numeric_limits<double>::max())
{
    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using variables."));
    txtLineEdit->setText("0");
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));
    //connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
    connect(txtLineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    lblValue = new QLabel(this);
    lblInfo = new QLabel();

#ifdef Q_WS_MAC
    btnDataTableDelete = new QToolButton();
    btnDataTableDelete->setIcon(icon("remove-item"));
    btnDataTableDelete->setMaximumHeight(txtLineEdit->height() - 4);
#else
    btnDataTableDelete = new QPushButton(icon("remove-item"), "");
    btnDataTableDelete->setMaximumSize(btnDataTableDelete->sizeHint());
#endif
    connect(btnDataTableDelete, SIGNAL(clicked()), this, SLOT(doOpenDataTableDelete()));

#ifdef Q_WS_MAC
    btnDataTableDialog = new QToolButton();
    btnDataTableDialog->setIcon(icon("three-dots"));
    btnDataTableDialog->setMaximumHeight(txtLineEdit->height() - 4);
#else
    btnDataTableDialog = new QPushButton(icon("three-dots"), "");
    btnDataTableDialog->setMaximumSize(btnDataTableDialog->sizeHint());
#endif
    connect(btnDataTableDialog, SIGNAL(clicked()), this, SLOT(doOpenDataTableDialog()));

    // timedep value
#ifdef Q_WS_MAC
    btnEditTimeDep = new QToolButton();
    btnEditTimeDep->setIcon(icon("three-dots"));
    btnEditTimeDep->setMaximumHeight(txtLineEdit->height() - 4);
#else
    btnEditTimeDep = new QPushButton(icon("three-dots"), "");
#endif
    connect(btnEditTimeDep, SIGNAL(clicked()), this, SLOT(doOpenValueTimeDialog()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblInfo, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);
    layout->addWidget(btnEditTimeDep, 0, Qt::AlignRight);
    layout->addWidget(btnDataTableDelete, 0, Qt::AlignRight);
    layout->addWidget(btnDataTableDialog, 0, Qt::AlignRight);

    setLayout(layout);

    setLayoutValue();
    evaluate();
}

ValueLineEdit::~ValueLineEdit()
{
}

void ValueLineEdit::setNumber(double value)
{
    txtLineEdit->setText(QString::number(value));
    evaluate();
}

double ValueLineEdit::number()
{
    if (evaluate())
        return m_number;
    else
        return 0.0;
}

void ValueLineEdit::setValue(const Value &value)
{
    txtLineEdit->setText(value.text());

    m_table = DataTable(value.table().pointsVector(),
                        value.table().valuesVector());

    setLayoutValue();
    evaluate();
}

Value ValueLineEdit::value()
{
    return Value(txtLineEdit->text(),
                 DataTable(m_table.pointsVector(),
                           m_table.valuesVector()));
}

bool ValueLineEdit::evaluate(bool quiet)
{
    bool isOk = false;
    bool valueChanged = false;

    if (!m_hasNonlin || m_table.size() == 0)
    {
        Value val = value();

        btnEditTimeDep->setVisible(m_hasTimeDep && Agros2D::problem()->isTransient());

        if (val.evaluate())
        {
            if (val.number() <= m_minimumSharp)
            {
                setValueLabel(QString("<= %1").arg(m_minimumSharp), QColor(Qt::blue), true);
            }
            else if (val.number() >= m_maximumSharp)
            {
                setValueLabel(QString(">= %1").arg(m_maximumSharp), QColor(Qt::blue), true);
            }
            else if (val.number() < m_minimum)
            {
                setValueLabel(QString("< %1").arg(m_minimum), QColor(Qt::blue), true);
            }
            else if (val.number() > m_maximum)
            {
                setValueLabel(QString("> %1").arg(m_maximum), QColor(Qt::blue), true);
            }
            else if (!checkCondition(val.number()))
            {
                setValueLabel(QString("%1").arg(m_condition), QColor(Qt::red), true);
            }
            else
            {
                double evaluatedNumber = val.number();
                valueChanged = (evaluatedNumber != m_number);
                m_number = evaluatedNumber;
                setValueLabel(QString("%1").arg(m_number, 0, 'g', 3), QApplication::palette().color(QPalette::WindowText),
                              Agros2D::configComputer()->lineEditValueShowResult);
                isOk = true;
            }
        }
        else
        {
            setValueLabel(tr("error"), QColor(Qt::red), true);
            setFocus();
        }
    }
    else
    {
        // table
        isOk = true;
    }

    if (isOk)
    {
        emit evaluated(false);
        if(valueChanged)
        {
            QString textValue = QString("%1").arg(m_number);
            emit textChanged(textValue);
        }
        return true;
    }
    else
    {
        emit evaluated(true);
        return false;
    }
}

bool ValueLineEdit::checkCondition(double value)
{
    if (m_condition.isEmpty()) return true;

    bool isOK = false;

    // FIXME: (Franta) replace -> LEX?
    QString condition = m_condition;
    condition.replace(QString("value"), QString::number(value));

    ExpressionResult result = currentPythonEngineAgros()->runExpression(condition, true);

    if (result.error.isEmpty())
    {
        if (!(fabs(result.value) < EPS_ZERO))
            isOK = true;
    }
    else
    {
        QPalette palette = txtLineEdit->palette();
        palette.setColor(QPalette::Text, QColor(Qt::red));
        txtLineEdit->setPalette(palette);

        txtLineEdit->setToolTip(tr("Condition couldn't be evaluated:\n%1").arg(result.error));
        isOK = true;
    }

    return isOK;
}

void ValueLineEdit::setLayoutValue()
{
    txtLineEdit->setVisible(false);
    lblValue->setVisible(false);
    lblInfo->setVisible(false);
    btnDataTableDelete->setVisible(false);
    btnDataTableDialog->setVisible(false);

    if ((!m_hasNonlin) || (m_hasNonlin && m_table.size() == 0))
    {
        txtLineEdit->setVisible(true);
        lblValue->setVisible(true);
    }
    if (m_hasNonlin && m_table.size() > 0)
    {
        if (!m_labelX.isEmpty() && !m_labelY.isEmpty())
            lblInfo->setText(tr("nonlinear %1(%2)").arg(m_labelY).arg(m_labelX));
        else
            lblInfo->setText(tr("nonlinear"));
        lblInfo->setVisible(true);
        btnDataTableDelete->setVisible(true);
    }

    btnDataTableDialog->setVisible(m_hasNonlin);
    btnEditTimeDep->setVisible(m_hasTimeDep && Agros2D::problem()->isTransient());
}

void ValueLineEdit::setValueLabel(const QString &text, QColor color, bool isVisible)
{
    lblValue->setText(text);
    QPalette palette = lblValue->palette();
    palette.setColor(QPalette::WindowText, color);
    lblValue->setPalette(palette);
    lblValue->setVisible(isVisible);
}

QSize ValueLineEdit::sizeHint() const
{
    return QSize(100, 10);
}

void ValueLineEdit::focusInEvent(QFocusEvent *event)
{
    txtLineEdit->setFocus(event->reason());
}

void ValueLineEdit::doOpenValueTimeDialog()
{
    ValueTimeDialog dialog;
    dialog.setValue(Value(txtLineEdit->text()));

    if (dialog.exec() == QDialog::Accepted)
    {
        txtLineEdit->setText(dialog.value().text());
        evaluate();
    }
}

void ValueLineEdit::doOpenDataTableDelete()
{
    m_table.clear();

    setLayoutValue();
    evaluate();
}

void ValueLineEdit::doOpenDataTableDialog()
{
    ValueDataTableDialog dataTableDialog(this, m_labelX, m_labelY);
    dataTableDialog.setDataTable(m_table);
    if (dataTableDialog.exec() == QDialog::Accepted)
        m_table = dataTableDialog.table();

    setLayoutValue();
    evaluate();
}
