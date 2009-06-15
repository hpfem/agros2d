#ifndef QSCRIPTSYNTAXHIGHLIGHTER_P_H
#define QSCRIPTSYNTAXHIGHLIGHTER_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER

#include <QtGui/qsyntaxhighlighter.h>

#include <QtGui/qtextformat.h>

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QScriptSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    QScriptSyntaxHighlighter(QTextDocument *document = 0);
    ~QScriptSyntaxHighlighter();

protected:
    void highlightBlock(const QString &text);

private:
    void highlightWord(int currentPos, const QString &buffer);

    enum ScriptFormats {
        ScriptTextFormat, ScriptNumberFormat,
        ScriptStringFormat, ScriptTypeFormat,
        ScriptKeywordFormat, ScriptPreprocessorFormat,
        ScriptLabelFormat, ScriptCommentFormat,
        NumScriptFormats
    };
    QTextCharFormat m_formats[NumScriptFormats];

private:
    Q_DISABLE_COPY(QScriptSyntaxHighlighter)
};

QT_END_NAMESPACE

#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif
