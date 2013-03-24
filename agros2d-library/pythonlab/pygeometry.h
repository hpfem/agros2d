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

#ifndef PYTHONLABGEOMETRY_H
#define PYTHONLABGEOMETRY_H

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

class SceneViewPreprocessor;

class PyGeometry
{
    public:
        PyGeometry() {}
        ~PyGeometry() {}

        void activate();

        // elements
        int addNode(double x, double y);
        int addEdge(double x1, double y1, double x2, double y2, double angle, map<char *, int> refinements, map<char*, char*> boundaries);
        int addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, double angle, map<char *, int> refinements, map<char*, char*> boundaries);
        int addLabel(double x, double y, double area, map<char *, int> refinements, map<char*, int> orders, map<char*, char*> materials);

        int nodesCount() { return Agros2D::scene()->nodes->count(); }
        int edgesCount() { return Agros2D::scene()->edges->count(); }
        int labelsCount() { return Agros2D::scene()->labels->count(); }

        // remove operations
        void removeNodes(vector<int> nodes);
        void removeEdges(vector<int> edges);
        void removeLabels(vector<int> labels);

        void removeNodePoint(double x, double y);
        void removeEdgePoint(double x1, double y1, double x2, double y2, double angle);
        void removeLabelPoint(double x, double y);

        // select operations
        void selectNodes(vector<int> nodes);
        void selectEdges(vector<int> edges);
        void selectLabels(vector<int> labels);

        void selectNodePoint(double x, double y);
        void selectEdgePoint(double x, double y);
        void selectLabelPoint(double x, double y);

        void selectNone();

        // transform operations
        void moveSelection(double dx, double dy, bool copy);
        void rotateSelection(double x, double y, double angle, bool copy);
        void scaleSelection(double x, double y, double scale, bool copy);
        void removeSelection();

        // zoom operations
        void zoomBestFit();
        void zoomIn();
        void zoomOut();
        void zoomRegion(double x1, double y1, double x2, double y2);
private:
        void setBoundaries(SceneEdge *edge, map<char *, char *> boundaries);
        void setMaterials(SceneLabel *label, map<char *, char *> materials);
        void setRefinementsOnEdge(SceneEdge *edge, map<char *, int> refinements);
        void setRefinements(SceneLabel *label, map<char *, int> refinements);
        void setPolynomialOrders(SceneLabel *label, map<char *, int> orders);
};

#endif // PYTHONLABGEOMETRY_H
