#include <QTextStream>
#include "lex.h"


Token::Token(TokenType type, QString text)
{
    this->m_text = text;
    this->m_type = type;
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

void LexicalAnalyser::print(const QStringList & list)
{
    int n = list.count();
    QTextStream qout(stdout);
    for(int i = 0; i < n; i++)
    {
        qout << list[i] << endl;
    }
}

QList<Token> LexicalAnalyser::tokens()
{
    return this->m_tokens;
}

QString get_expression( QList<Token> const & symbol_que, int position = 0)
{
    int n = symbol_que.count();
    int nesting_level = symbol_que[position].nestingLevel;
    QString expression =  "neco";
    for(int i = position; i < n; i++)
    {
        if (nesting_level < symbol_que[i].nestingLevel)
        {
            nesting_level++;
        }

        if (nesting_level > symbol_que[i].nestingLevel)
        {
            nesting_level--;
        }

    }
    return expression;
}


LexicalAnalyser::LexicalAnalyser(const QString &expression)
{
    QTextStream qout(stdout);
    QStringList variables;
    QStringList operators;
    QStringList functions;
    QList<Terminals>  terminals;

    variables << "x" << "yy" << "y" << "xx";
    sortByLength(variables);
    terminals.append(Terminals(TokenType_VARIABLE, variables));

    operators << "(" << ")" << "+" << "**" << "-" << "*" << "/" << "^" ;
    sortByLength(operators);
    terminals.append(Terminals(TokenType_OPERATOR, operators));
    functions << "sin" << "cos" << "log" << "log10";
    sortByLength(functions);
    terminals.append(Terminals(TokenType_FUNCTION, functions));

    int pos = 0;


    QRegExp r_exp = QRegExp("[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

    int loop_counter = 0;
    int n = expression.count();
    int old_pos = 0;
    int nesting_level = 0;
    while ((pos < n) && (loop_counter < n))
    {
        loop_counter++;
        int index = r_exp.indexIn(expression, pos);

        if(index == pos)
        {
            QString text = r_exp.capturedTexts().takeFirst();
            Token symbol(TokenType_NUMBER, text);
            symbol.nestingLevel = nesting_level;
            pos += text.count();
            m_tokens.append(symbol);
        }

        foreach(Terminals terminal_symbols, terminals)
        {
            terminal_symbols.find(expression, m_tokens, pos, nesting_level);
        }

        if (old_pos == pos)
        {
            qout << "Unexpected symbol:" << "\"z\"" << " on position: " << pos << endl;
            qout << expression.at(pos).toAscii() << endl;
            break;
        }
        else
        {
            old_pos = pos;
        }
    }



/*    for(int i = 0; i < tokens.count();i++)
    {
        qout << tokens[i].text << "  " << tokens[i].terminal_type << "  " << tokens[i].nesting_level << endl;
    } */


    get_expression(m_tokens);
}


Terminals::Terminals(TokenType terminal_type, QStringList terminal_list)
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
        int loc_pos = s.indexOf(m_list[i].text(), pos);
        if (loc_pos == pos) {
            symbol = Token(m_list[i].type(), m_list[i].text());

            if (symbol.text() == "(")
            {
                symbol.nestingLevel = nesting_level++;
            } else
                if (symbol.text() == ")")
                {
                    symbol.nestingLevel = --nesting_level;
                }
                else
                    symbol.nestingLevel = nesting_level;

            pos += m_list[i].text().count();
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
        qout << this->m_list[i].text() << endl;
    }
}
