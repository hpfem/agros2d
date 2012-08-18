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

class TreeNode
{
public:
    TreeNode(double value);
    TreeNode(TreeNode *leftNode, TreeNode *rightNode, QString operation);

    inline void setLeftTree(TreeNode *left_tree) { m_left_tree = left_tree; }
    inline void setRightTree(TreeNode *right_tree) { m_right_tree = right_tree; }
    inline TreeNode *leftTree() { return m_left_tree;}
    inline TreeNode *rightTree() { return m_right_tree;}

    inline void setNodeType(TokenType node_type) { this->m_node_type = node_type; }
    inline TokenType nodeType() { return m_node_type;}

    inline void setLeaf(bool leaf) { this->m_leaf = leaf; }
    inline bool isLeaf() { return m_leaf; }

    inline double value() { return m_value; }
    inline QString getOperator() { return m_operator; }

private:
    bool m_leaf;
    TreeNode * m_left_tree;
    TreeNode * m_right_tree;
    TokenType m_node_type;
    double m_value;
    QString m_text;
    QString m_operator;
};


class SyntacticAnalyser
{
public:
    SyntacticAnalyser(QList<Token> tokens);
    ~SyntacticAnalyser();

    void deleteTree();
    void printTree();
    void goThroughTree(TreeNode *node);
    TreeNode *parseExpression();
    TreeNode *parseTerm();
    TreeNode *parseFactor();
    TreeNode *buildNode(TreeNode *left_node, TreeNode *right_node, QString operation);
    TreeNode *bildLeaf(TreeNode *left_node, TreeNode *right_node, QString operation);
    double evalTree(TreeNode *node);

private:
    QList<Token> m_tokens;
    Token m_currentSymbol;
    TreeNode *m_syntaxTree;

    void deleteTree(TreeNode * node);
};

#endif // TREE_H
