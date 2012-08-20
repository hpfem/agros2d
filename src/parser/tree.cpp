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

#include <iostream>
#include <cmath>

#include "./tree.h"


TreeNode::TreeNode(TreeNode * left_node, TreeNode * right_node, QString operation)
{
    this->m_node_type = OPERATOR;
    this->m_operator = operation;
    this->m_leaf = false;
    this->m_left_tree = left_node;
    this->m_right_tree = right_node;
    this->m_value = 0;
    this->m_text = "";
}

TreeNode::TreeNode(double value)
{
    this->m_node_type = NUMBER;
    this->m_operator = "";
    this->m_leaf = true;
    this->m_left_tree = 0;
    this->m_right_tree = 0;
    this->m_value = value;
    this->m_text = "";
}

TreeNode::TreeNode(double * variable)
{
    this->m_node_type = VARIABLE;
    this->m_operator = "";
    this->m_leaf = true;
    this->m_left_tree = 0;
    this->m_right_tree = 0;
    this->m_variable = variable;
    this->m_value = 0;
    this->m_text = "";
}

void SyntacticAnalyser::Parse(QList<Token> tokens)
{
    this->tokens = tokens;
    if(this->tokens.count() != 0)
    {
        this->current_symbol = this->tokens.takeFirst();
        this->syntax_tree = this->parseExpression();
    }
}

TreeNode *  SyntacticAnalyser::parseExpression()
{

    TreeNode * left_node = 0;
    TreeNode * right_node = 0;
    TreeNode * operator_node = 0;

    left_node = parseTerm();

    while (((current_symbol.get_type() == OPERATOR)&&((current_symbol.get_text() == "+") ||
                                                      (current_symbol.get_text() == "-"))))
    {

        QString operation = current_symbol.get_text();
        if (this->tokens.count() != 0)
        {
            current_symbol = tokens.takeFirst();
        }
        right_node = parseTerm();

        if (operator_node == 0)
        {
            if ((operation == "+") || (operation == "-"))
            {
                operator_node = new TreeNode(left_node, right_node, operation);
            }
        }
        else
        {
            TreeNode * tmp_node = operator_node;
            operator_node = new TreeNode(tmp_node, right_node, operation);
        }
    }
    if (operator_node != 0)
        return operator_node;
    else
        return left_node;
}

TreeNode *  SyntacticAnalyser::parseTerm()
{
    TreeNode * left_node = 0;
    TreeNode * right_node = 0;
    TreeNode * operator_node =0;

    left_node = parseExponent();

    while ((tokens.length() != 0) && (current_symbol.get_type() == OPERATOR)&&((current_symbol.get_text() == "*") ||
                                                                               (current_symbol.get_text() == "/")))
    {

        QString operation = current_symbol.get_text();
        if (this->tokens.count() != 0)
        {
            current_symbol = tokens.takeFirst();
        }
        right_node = parseExponent();

        if (operator_node == 0)
        {
            if ((operation == "*") || (operation == "/"))
            {
                operator_node = new TreeNode(left_node, right_node, operation);
            }
        }
        else
        {
            TreeNode * tmp_node = operator_node;
            operator_node = new TreeNode(tmp_node, right_node, operation);
        }

    }
    if (operator_node != 0)
        return operator_node;
    else
        return left_node;
}

TreeNode *  SyntacticAnalyser::parseExponent()
{
    TreeNode * left_node = 0;
    TreeNode * right_node = 0;
    TreeNode * operator_node =0;

    left_node = parseFactor();

    while ((tokens.length() != 0) && (current_symbol.get_type() == OPERATOR)&&((current_symbol.get_text() == "**") ||
                                                                               (current_symbol.get_text() == "^")))
    {

        QString operation = current_symbol.get_text();
        if (this->tokens.count() != 0)
        {
            current_symbol = tokens.takeFirst();
        }
        right_node = parseFactor();

        if (operator_node == 0)
        {
            if ((operation == "**") || (operation == "^"))
            {
                operator_node = new TreeNode(left_node, right_node, operation);
            }
        }
        else
        {
            TreeNode * tmp_node = operator_node;
            operator_node = new TreeNode(tmp_node, right_node, operation);
        }

    }
    if (operator_node != 0)
        return operator_node;
    else
        return left_node;
}

TreeNode *  SyntacticAnalyser::parseFactor()
{
    TreeNode * node = 0;
    static double sign = 1;

    if ((current_symbol.get_text() == "-") || (current_symbol.get_text() == "+"))
    {
        if (current_symbol.get_text() == "-")
        {
            sign = sign * (-1);
        }

        if (this->tokens.count() != 0)
        {
            current_symbol = tokens.takeFirst();
        }
        node = this->parseFactor();
    }
    else
    {

        if (current_symbol.get_type() == NUMBER)
        {
            double value = current_symbol.get_text().toDouble() * sign;
            node = new TreeNode(value);
            if (this->tokens.count() != 0)
            {
                current_symbol = tokens.takeFirst();
            }
        }
        if (current_symbol.get_type() == VARIABLE)
        {
            node = new TreeNode((this->m_variable_map.value(current_symbol.get_text())));
            if (this->tokens.count() != 0)
            {
                current_symbol = tokens.takeFirst();
            }
        }
    }


    if (current_symbol.get_type() == FUNCTION)
    {
        QString function_name = current_symbol.get_text();
        if (this->tokens.count() != 0)
        {
            current_symbol = tokens.takeFirst();
        }
        node = new TreeNode(parseFactor(), 0, function_name);
    }


    if ((current_symbol.get_text() == "("))
    {
        while((current_symbol.get_text() != ")") && (tokens.length() != 0))
        {
            if (this->tokens.count() != 0)
            {
                current_symbol = tokens.takeFirst();
            }

            node = parseExpression();
        }

        if (this->tokens.count() != 0)
        {
            current_symbol = tokens.takeFirst();
        }
    }

    return node;
}

void SyntacticAnalyser::printTree()
{
    QTextStream qout(stdout);
    goThroughTree(this->syntax_tree);
}

void SyntacticAnalyser::goThroughTree(TreeNode * node)
{
    if (node->isLeaf())
        ;
    else {
        goThroughTree(node->leftTree());
        if(node->rightTree() != 0)
            goThroughTree(node->rightTree());
    }
}

SyntacticAnalyser::~SyntacticAnalyser()
{
    deleteTree(this->syntax_tree);
}

void SyntacticAnalyser::deleteTree(TreeNode * node)
{
    if (node->leftTree() != 0)
    {
        deleteTree(node->leftTree());
    }

    if (node->rightTree() != 0)
    {
        deleteTree(node->rightTree());
    }

    delete node;
}

double SyntacticAnalyser::evalTree()
{
    return evalTree(this->syntax_tree);
}

double SyntacticAnalyser::evalTree(TreeNode * node)
{
    double result;
    if (node->isLeaf())
    {
        if (node->getNodeType() == NUMBER)
            result = node->getValue();
        if (node->getNodeType() == VARIABLE)
            result = * node->getVariable();
    }
    else
    {
        if (node->getOperator() == "+")
            result = this->evalTree(node->leftTree()) + this->evalTree(node->rightTree());
        if (node->getOperator() == "-")
            result = this->evalTree(node->leftTree()) - this->evalTree(node->rightTree());
        if (node->getOperator() == "*")
            result = this->evalTree(node->leftTree()) * this->evalTree(node->rightTree());
        if (node->getOperator() == "/")
            result = this->evalTree(node->leftTree()) / this->evalTree(node->rightTree());
        if((node->getOperator() == "^") || (node->getOperator() == "**"))
            result = pow(this->evalTree(node->leftTree()), this->evalTree(node->rightTree()));
        if(node->getOperator() == "cos")
            result = cos(this->evalTree(node->leftTree()));
        if(node->getOperator() == "sin")
            result = sin(this->evalTree(node->leftTree()));
        if(node->getOperator() == "tan")
            result = tan(this->evalTree(node->leftTree()));
        if(node->getOperator() == "exp")
            result = exp(this->evalTree(node->leftTree()));
        if(node->getOperator() == "log")
            result = log(this->evalTree(node->leftTree()));
        if(node->getOperator() == "log10")
            result = log10(this->evalTree(node->leftTree()));
    }
    return result;
}



