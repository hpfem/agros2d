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
public:
    TreeNode(double value);
    TreeNode(double * variable);
    TreeNode(TreeNode * leftNode, TreeNode * rightNode, QString operation);
    bool isLeaf() {return m_leaf;}
    TreeNode * leftTree(){return m_leftTree;}
    TreeNode * rightTree(){return m_rightTree;}

    void setLeftTree(TreeNode * left_tree){m_leftTree = left_tree;}
    void setRightTree(TreeNode * right_tree){m_rightTree = right_tree;}

    inline QString text() { return this->m_text; }
    inline double *variable() { return this->m_variable; }
    TokenType nodeType() { return m_nodeType; }
    void setNodeType(TokenType node_type) { this->m_nodeType = node_type; }
    void setLeaf(bool leaf) { this->m_leaf = leaf; }
    inline double value() { return m_value; }
    inline QString getOperator() { return m_operator; }

private:
    bool m_leaf;
    TreeNode *m_leftTree;
    TreeNode *m_rightTree;
    TokenType m_nodeType;
    double m_value;
    double *m_variable;
    QString m_text;
    QString m_operator;
};


class SyntacticAnalyser
{
public:
    SyntacticAnalyser() {}
    ~SyntacticAnalyser();

    void parse(QList<Token> tokens);
    void deleteTree();
    void printTree();
    void goThroughTree(TreeNode *node);

    TreeNode *parseExpression();
    TreeNode *parseTerm();
    TreeNode *parseFactor();
    TreeNode *parseExponent();
    TreeNode *buildNode(TreeNode *left_node, TreeNode *right_node, QString operation);
    TreeNode *buildLeaf(TreeNode *left_node, TreeNode *right_node, QString operation);

    double evalTree(TreeNode *node);
    double evalTree();

private:
    QHash<QString, double *> m_variableMap;
    QList<Token> m_tokens;
    Token m_currentSymbol;
    TreeNode * m_syntaxTree;

    void deleteTree(TreeNode *node);
};

#endif // TREE_H
