#include<iostream>
#include <QTextStream>
#include "lex.h"
#include <QRegExp>

Token::Token(ParserTokenType type, QString text, int nestingLevel, int position)
{
    this->m_text = text;
    this->m_type = type;
    this->m_nestingLevel = nestingLevel;
    this->m_position = position;
}


LexicalAnalyser::LexicalAnalyser()
{
    this->setPatterns();
}

void LexicalAnalyser::sortByLength(QStringList & list)
{
    int n = list.count();
    QString temp;

    for(int i = 0; i < n; i++)
    {
        for(int j = 1; j < n; j++)
        {
            if (list[j].count() > list[j-1].count())
            {
                temp = list[j];
                list[j] = list[j-1];
                list[j-1] = temp;
            }
        }
    }
}

void LexicalAnalyser::printTokens()
{
    QTextStream qout(stdout);
    foreach (Token token, m_tokens)
    {
        qout << token.toString() << endl;
    }
}

QList<Token> LexicalAnalyser::tokens()
{
    return this->m_tokens;
}

QString expression(QList<Token> symbol_que, int position = 0)
{
    int n = symbol_que.count();
    int nesting_level = symbol_que[position].nestingLevel();
    QString expression;
    for(int i = position; i < n; i++)
    {
        if (nesting_level < symbol_que[i].nestingLevel())
        {
            nesting_level++;
        }

        if (nesting_level > symbol_que[i].nestingLevel())
        {
            nesting_level--;
        }
    }
    return expression;
}

void LexicalAnalyser::setPatterns()
{
    QStringList operators;
    operators << "(" << ")" << "+" << "**" << "-" << "*" << "/" << "^" << "==" << "&&" << "||" << "<=" << ">=" << "!=" << "<" << ">" << "=" << "?" << ":" << ",";
    QString pattern = "";
    for(int i = 0; i < operators.length() - 1; i++)
    {
        pattern += QRegExp::escape(operators[i]) + "|";
    }
    pattern += QRegExp::escape(operators[operators.length() - 1]);

    m_patterns << Terminal(ParserTokenType_OPERATOR, pattern);
    m_patterns << Terminal(ParserTokenType_NUMBER, "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)");
    m_patterns << Terminal(ParserTokenType_FUNCTION, "([a-zA-Z][_a-zA-Z0-9]*)(?=\\()");
    m_patterns << Terminal(ParserTokenType_VARIABLE, "([a-zA-Z][_a-zA-Z0-9]*)");
    m_patterns << Terminal(ParserTokenType_KEYWORD, "dx");
}

void LexicalAnalyser::setExpression(const QString &expr)
{
    // ToDo: Don't erase white characters
    // QString exprTrimmed = expr.trimmed().replace(" ", "");
    QString exprTrimmed = expr.trimmed();

    //    QStringList operators;
    //    QStringList functions;
    //    QList<Terminals>  terminals;

    //    sortByLength(m_variables);
    //    terminals.append(Terminals(ParserTokenType_VARIABLE, m_variables));

    //    operators << "(" << ")" << "+" << "**" << "-" << "*" << "/" << "^" << "==" << "&&" << "||" << "<=" << ">=" << "!=" << "<" << ">" << "=" << "?" << ":" << ",";
    //    sortByLength(operators);
    //    terminals.append(Terminals(ParserTokenType_OPERATOR, operators));
    //    functions << "sin" << "cos" << "asin" <<  "acos" << "atan" << "sinh" << "cosh" <<
    //                 "tanh" << "asinh" << "acosh" << "atanh" << "log2" << "log10" << "log" <<
    //                 "exp" << "sqrt" << "sign" << "abs" << "min" << "max" << "sum" << "avg" << "pow";
    //    // TODO: conflict "tan" with "tanx", "tany", "tanr", "tanz" in surface forms

    //    sortByLength(functions);
    //    terminals.append(Terminals(ParserTokenType_FUNCTION, functions));

    //    int pos = 0;

    //    QRegExp r_exp = QRegExp("[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

    //    int loop_counter = 0;
    //    int n = exprTrimmed.count();
    //    int old_pos = 0;
    //    int nesting_level = 0;

    //    while ((pos < n) && (loop_counter < n))
    //    {
    //        loop_counter++;
    //        int index = r_exp.indexIn(exprTrimmed, pos);

    //        if(index == pos)
    //        {
    //            QString text = r_exp.capturedTexts().takeFirst();
    //            Token symbol(ParserTokenType_NUMBER, text, nesting_level, pos);
    //            pos += text.count();
    //            m_tokens.append(symbol);
    //        }

    //        foreach(Terminals terminal_symbols, terminals)
    //        {
    //            terminal_symbols.find(exprTrimmed, m_tokens, pos, nesting_level);
    //        }

    //        if (old_pos == pos)
    //        {
    //            throw ParserException(QString("Unexpected symbol '%1' on position %2 in expression '%3'").arg(exprTrimmed[pos]).arg(pos).arg(expr),
    //                                  expr,
    //                                  pos,
    //                                  exprTrimmed.at(pos));
    //            break;
    //        }
    //        else
    //        {
    //            old_pos = pos;
    //        }
    //    }

    int index = 0;
    int position = 0;
    while(position < exprTrimmed.length())
    {
        bool match = false;
        foreach(Terminal terminal, m_patterns)
        {
            index = terminal.m_pattern.indexIn(exprTrimmed, position);
            if(index == position)
            {
                position =  index + terminal.m_pattern.capturedTexts()[0].count();
                Token token(terminal.m_terminalType,terminal.m_pattern.capturedTexts()[0], index);
                m_tokens.append(token);
                match = true;
            }
        }

        if ((!match) && (position < exprTrimmed.length()))
        {
            if (exprTrimmed[position] == ' ')
            {
                position++;
            }
            else
            {
                throw ParserException(QString("Unexpected symbol '%1' on position %2 in expression '%3'").arg(exprTrimmed[position]).arg(position).arg(expr),
                                      expr,
                                      position,
                                      exprTrimmed.at(position));
                return;
            }
        }
    }

    int i = 0;
    while(i < m_tokens.count() - 1)
    {
        if ((m_tokens[i].type() == ParserTokenType_NUMBER) && (m_tokens[i+1].type() != ParserTokenType_OPERATOR))
        {
            QString lexem = "";
            while(((m_tokens[i+1].type() != ParserTokenType_OPERATOR) && (i < m_tokens.count()-1)))
            {
                lexem += m_tokens[i].toString();
                i++;
            }
            qDebug() << "Invalid number format:" << lexem + m_tokens[i].toString();
        }

        if ((m_tokens[i].type() == ParserTokenType_VARIABLE) && (m_tokens[i+1].type() != ParserTokenType_OPERATOR))
        {
            qDebug() << "Synatax error in expression " << m_tokens[i].toString() + " " + m_tokens[i+1].toString();
        }
        i++;
    }
}

QString LexicalAnalyser::latexVariables(QMap<QString, QString> dict, const QString &expr)
{
    QMap<QString, QString> greek_letters;

    greek_letters["eps"] = "\\varepsilon";
    greek_letters["rho"] = "\\rho";
    greek_letters["gamma"] = "\\gamma";
    greek_letters["mu"] = "\\mu";

    if (!expr.isEmpty())
        setExpression(expr);

    // replace tokens
    QString output;
    foreach (Token token, tokens())
    {
        bool isReplaced = false;
        QString token_text = token.toString();
        // iterate whole m_patterns
        QMapIterator<QString, QString> i(dict);
        while (i.hasNext())
        {
            i.next();

            if (token_text == i.key())
            {
                output += i.value();
                isReplaced = true;
                break;
            }
        }

        // without replacing
        if (!isReplaced)
        {
            QStringList symbol= token_text.split("_");
            if (symbol.length() > 1)
            {
                QMapIterator<QString, QString> i(greek_letters);
                while (i.hasNext())
                {
                    i.next();

                    if (symbol[1] == i.key())
                    {
                        symbol[1] =  i.value();
                        break;
                    }
                }
                output += symbol[1] + "_{" + symbol[0] + "}";
            }

            else
                output += symbol[0];
        }
    }

    return output;
}

QString LexicalAnalyser::replaceVariables(QMap<QString, QString> dict, const QString &expr)
{
    if (!expr.isEmpty())
        setExpression(expr);

    // replace tokens
    QString output;
    foreach (Token token, tokens())
    {
        bool isReplaced = false;

        // iterate whole m_patterns
        QMapIterator<QString, QString> i(dict);
        while (i.hasNext())
        {
            i.next();

            if (token.toString() == i.key())
            {
                output += i.value();
                isReplaced = true;
                break;
            }
        }

        // without replacing
        if (!isReplaced)
            output += token.toString();
    }

    return output;
}

//ToDo: Improve, it should be within Syntax analyzer
QString LexicalAnalyser::replaceOperatorByFunction(QString expression)
{
    // Regular expression for ^any_number
    QRegExp re("(\\^)([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)");
    QString expr = expression;
    int position = 0;
    bool isReplaced = false;
    while (position != -1)
    {
        position = re.indexIn(expr);
        if (position != -1)
        {
            isReplaced = true;
            QString replace = "";
            QString exponent = re.capturedTexts()[2];
            //std::cout << position << " " << exponent.toStdString() << std::endl;
            if (expr[position-1] == ')')
            {
                int j = position - 1;
                int level = 0;
                do
                {
                    if (expr[j] == '(' )
                        level = level - 1;
                    if (expr[j] == ')' )
                        level = level + 1;
                    j = j - 1;
                } while (level != 0);

                for (int k = 0; k <= j; k++ )
                {
                    replace += expr[k];
                }
                replace += "pow";
                for (int k = j+1; k < position - 1; k++ )
                {
                    replace += expr[k];
                }
                replace += ", " + exponent + ")";
                for (int k = position + 1 + exponent.length(); k < expr.length(); k++)
                {
                    replace += expr[k];
                }
            }
            expr = replace;
        }
    }

    if (isReplaced == true)
        return expr;
    else
        return expression;
}


Terminals::Terminals(ParserTokenType terminal_type, QStringList terminal_list)
{

    int n = terminal_list.count();
    for(int i = 0; i < n; i++)
    {
        Token symbol = Token(terminal_type, terminal_list[i]);
        this->m_list.append(symbol);
    }
}

void Terminals::find(const QString &s, QList<Token> &symbol_que, int &pos, int &nesting_level)
{
    Token symbol;
    int n = this->m_list.count();
    for (int i = 0; i < n; i++)
    {
        int loc_pos = s.indexOf(m_list[i].toString(), pos);
        if (loc_pos == pos) {
            symbol = Token(m_list[i].type(), m_list[i].toString(), nesting_level, pos);

            if (symbol.toString() == "(")
            {
                symbol.setNestingLevel(nesting_level + 1);
            } else
                if (symbol.toString() == ")")
                {
                    symbol.setNestingLevel(nesting_level - 1);
                }
                else
                    symbol.setNestingLevel(nesting_level);

            pos += m_list[i].toString().count();
            symbol_que.append(symbol);
            break;
        }
    }
}

void Terminals::print()
{
    QTextStream qout(stdout);
    int n =this->m_list.count();
    for(int i = 0; i < n; i++)
    {
        qout << this->m_list[i].toString() << endl;
    }
}
