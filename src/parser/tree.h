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

#ifndef TREE_H
#define TREE_H

#include <QString>
#include "lex.h"


// ToDo: Make derivred classes for diferent types of nodes i.e
// opreator node, variable node, konstant node, function node etc.

class TreeNode
{
private:
    bool m_leaf;
    TreeNode * m_left_tree;
    TreeNode * m_right_tree;
    Token_type m_node_type;
    double m_value;
    double * m_variable;
    QString m_text;
    QString m_operator;


public:
    TreeNode(double value);
    TreeNode(double * variable);
    TreeNode(TreeNode * leftNode, TreeNode * rightNode, QString operation);
    bool isLeaf() {return m_leaf;}
    TreeNode * leftTree(){return m_left_tree;}
    TreeNode * rightTree(){return m_right_tree;}

    void setLeftTree(TreeNode * left_tree){m_left_tree = left_tree;}
    void setRightTree(TreeNode * right_tree){m_right_tree = right_tree;}

    QString getText(){return this->m_text;}
    double * getVariable() {return this->m_variable;}
    Token_type getNodeType(){return m_node_type;}
    void setNodeType(Token_type node_type){this->m_node_type = node_type;}
    void setLeaf(bool leaf) {this->m_leaf = leaf;}
    double getValue() {return m_value;}
    QString getOperator() {return m_operator;}
};


class SyntacticAnalyser
{
private:
    QList<Token> tokens;
    Token current_symbol;
    TreeNode * syntax_tree;
    void deleteTree(TreeNode * node);


public:
    QHash<QString, double *> m_variable_map; // ToDo: Make it private
    SyntacticAnalyser(){}
    void Parse(QList<Token> tokens);
    ~SyntacticAnalyser();
    void deleteTree();
    void printTree();
    void goThroughTree(TreeNode * node);
    TreeNode * parseExpression();
    TreeNode * parseTerm();
    TreeNode * parseFactor();
    TreeNode * parseExponent();
    TreeNode * buildNode(TreeNode * left_node, TreeNode * right_node, QString operation);
    TreeNode * bildLeaf(TreeNode * left_node, TreeNode * right_node, QString operation);
    double evalTree(TreeNode * node);
    double evalTree();
};

#endif // TREE_H
