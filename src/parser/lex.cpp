#include <QTextStream>
#include "lex.h"


Token::Token(Token_type type, QString text)
{
    this->text = text;
    this->type = type;
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

QList<Token> LexicalAnalyser::getTokens()
{
    return this->tokens;
}

QString get_expression( QList<Token> const & symbol_que, int position = 0)
{
    int n = symbol_que.count();
    int nesting_level = symbol_que[position].nesting_level;
    QString expression =  "neco";
    for(int i = position; i < n; i++)
    {
        if (nesting_level < symbol_que[i].nesting_level)
        {
            nesting_level++;
        }

        if (nesting_level > symbol_que[i].nesting_level)
        {
            nesting_level--;
        }

    }
    return expression;
}


LexicalAnalyser::LexicalAnalyser(QString expression)
{
    QTextStream qout(stdout);
    QStringList variables;
    QStringList operators;
    QStringList functions;
    QList<Terminals>  terminals;

    variables << "x" << "yy" << "y" << "xx";
    sortByLength(variables);
    terminals.append(Terminals(VARIABLE, variables));

    operators << "(" << ")" << "+" << "**" << "-" << "*" << "/" << "^" ;
    sortByLength(operators);
    terminals.append(Terminals(OPERATOR, operators));
    functions << "sin" << "cos" << "log" << "log10";
    sortByLength(functions);
    terminals.append(Terminals(FUNCTION, functions));

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
            Token symbol(NUMBER, text);
            symbol.nesting_level = nesting_level;
            pos += text.count();
            tokens.append(symbol);
        }

        foreach(Terminals terminal_symbols, terminals)
        {
            terminal_symbols.find(expression, tokens, pos, nesting_level);
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


    get_expression(tokens);
}


Terminals::Terminals(Token_type terminal_type, QStringList terminal_list)
{

    int n = terminal_list.count();
    for(int i = 0; i < n; i++)
    {
        Token symbol = Token(terminal_type, terminal_list[i]);
        this->list.append(symbol);
    }
}

void Terminals::find(QString s, QList<Token> & symbol_que, int & pos, int & nesting_level)
{
    Token symbol;
    int n = this->list.count();
    for (int i = 0; i < n; i++)
    {
        int loc_pos = s.indexOf(list[i].get_text(), pos);
        if (loc_pos == pos) {
            symbol = Token(list[i].get_type(), list[i].get_text());

            if (symbol.get_text() == "(")
            {
                symbol.nesting_level = nesting_level++;
            } else
                if (symbol.get_text() == ")")
                {
                    symbol.nesting_level = --nesting_level;
                }
                else
                    symbol.nesting_level = nesting_level;

            pos += list[i].get_text().count();
            symbol_que.append(symbol);
            break;
        }
    }
}

void Terminals::print()
{
    QTextStream qout(stdout);
    int n =this->list.count();
    for(int i = 0; i < n; i++)
    {
        qout << this->list[i].get_text() << endl;
    }
}
