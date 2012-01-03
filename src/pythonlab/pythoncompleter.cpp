#include "pythoncompleter.h"

#include <QCompleter>

PythonCompleterListView::PythonCompleterListView(QWidget *parent)
    : QListView(parent)
{
}

void PythonCompleterDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QString str = index.model()->data(index.model()->index(index.row(), 0)).toString();

    int leftbracket = str.indexOf("(");
    int rigthbracket = str.indexOf(")");
    int comma = str.indexOf(",");
    QString kind = str.mid(leftbracket + 1, comma - leftbracket - 1);
    QString type = str.mid(comma + 2, rigthbracket - 1);

    // draw correct background
    opt.text = "";
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    QRect rect = opt.rect;
    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    // set pen color
    QBrush brush(Qt::red);
    if (kind.contains("global"))
        brush = QBrush(Qt::red);
    if (kind.contains("keyword"))
        brush = QBrush(Qt::blue);
    if (type.contains("variable"))
        brush = QBrush(Qt::darkGreen);

    painter->setBrush(brush);
    painter->fillRect(rect.left() + 7, rect.top() + 7, rect.height() - 15, rect.height() - 15, brush);

    // draw line of text
    painter->setPen(opt.palette.color(cg, QPalette::Text));
    if (type.contains("function"))
        painter->setPen(QColor(66, 0, 0));

    painter->drawText(QRect(rect.left() + 20, rect.top(), rect.width(), rect.height()),
                      opt.displayAlignment, str);
}

QSize PythonCompleterDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    result.setHeight(result.height());
    return result;
}

QCompleter *createCompleter()
{
    PythonCompleterListView *lstView = new PythonCompleterListView();
    lstView->setItemDelegateForColumn(0, new PythonCompleterDelegate());

    QCompleter *completer = new QCompleter();
    completer->setPopup(lstView);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

    return completer;
}
