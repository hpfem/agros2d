#ifndef PYTHONCOMPLETER_H
#define PYTHONCOMPLETER_H

#include <QtGui>

class PythonCompleterListView : public QListView
{
    Q_OBJECT
public:
    PythonCompleterListView(QWidget *parent = 0);
};

class PythonCompleterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PythonCompleterDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

// return completer
QCompleter *createCompleter();

#endif // PYTHONCOMPLETER_H
