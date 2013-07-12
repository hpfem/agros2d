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

#include "meshgenerator_gmsh.h"

#include "util/global.h"
#include "util/loops.h"

#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "sceneview_common.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "logview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

MeshGeneratorGMSH::MeshGeneratorGMSH() : MeshGenerator()
{
}

bool MeshGeneratorGMSH::mesh()
{
    m_isError = !prepare();

    // create gmsh files
    if (writeToGmsh())
    {
        Agros2D::log()->printDebug(tr("Mesh generator"), tr("GMSH geometry file was created"));

        // exec triangle
        m_process = new QProcess();
        m_process->setStandardOutputFile(tempProblemFileName() + ".gmsh.out");
        m_process->setStandardErrorFile(tempProblemFileName() + ".gmsh.err");
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(meshGmshError(QProcess::ProcessError)));
        connect(m_process, SIGNAL(finished(int)), this, SLOT(meshGmshCreated(int)));

        QString gmshBinary = "gmsh";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "gmsh.exe"))
            gmshBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "gmsh.exe\"";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "gmsh"))
            gmshBinary = QApplication::applicationDirPath() + QDir::separator() + "gmsh";

        QString triangleGMSH = "%1 -2 \"%2.geo\"";
        m_process->start(triangleGMSH.
                         arg(gmshBinary).
                         arg(tempProblemFileName()));

        // execute an event loop to process the request (nearly-synchronous)
        QEventLoop eventLoop;
        connect(m_process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
        eventLoop.exec();
    }
    else
    {
        m_isError = true;
    }

    return !m_isError;
}

void MeshGeneratorGMSH::meshGmshError(QProcess::ProcessError error)
{
    Agros2D::log()->printError(tr("Mesh generator"), tr("Could not start GMSH"));
    m_process->kill();
}


void MeshGeneratorGMSH::meshGmshCreated(int exitCode)
{
    if (exitCode == 0)
    {
        Agros2D::log()->printMessage(tr("Mesh generator"), tr("Mesh files were created"));
        // convert gmsh mesh to hermes mesh
        if (readGmshMeshFile())
        {
            Agros2D::log()->printMessage(tr("Mesh generator"), tr("Mesh was converted to Hermes2D mesh file"));

            // copy gmsh files
            if ((!Agros2D::configComputer()->deleteMeshFiles) && (!Agros2D::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Agros2D::problem()->config()->fileName());

                QFile::copy(tempProblemFileName() + ".geo", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".geo");
            }

            // copy hermes files
            if ((!Agros2D::configComputer()->deleteHermesMeshFile) && (!Agros2D::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Agros2D::problem()->config()->fileName());

                QFile::copy(cacheProblemDir() + "/initial.msh", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".msh");
            }

            //  remove gmsh temp files
            QFile::remove(tempProblemFileName() + ".geo");
            QFile::remove(tempProblemFileName() + ".msh");
            QFile::remove(tempProblemFileName() + ".gmsh.out");
            QFile::remove(tempProblemFileName() + ".gmsh.err");
        }
        else
        {
            m_isError = true;
            QFile::remove(Agros2D::problem()->config()->fileName() + ".msh");
        }
    }
    else
    {
        m_isError = true;
        QString errorMessage = readFileContent(tempProblemFileName() + ".gmsh.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        Agros2D::log()->printError(tr("Mesh generator"), errorMessage.replace("\n", "<br/>"), false);
    }
}

bool MeshGeneratorGMSH::writeToGmsh()
{
    // basic check
    if (Agros2D::scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of nodes (%1 < 3)").arg(Agros2D::scene()->nodes->length()));
        return false;
    }
    if (Agros2D::scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of edges (%1 < 3)").arg(Agros2D::scene()->edges->length()));
        return false;
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(tempProblemFileName() + ".geo");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not create GMSH geometry file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);

    // mesh size
    RectPoint rect = Agros2D::scene()->boundingBox();
    out << QString("mesh_size = %1;\n").arg(qMin(rect.width(), rect.height()) / 6.0);
    //out << QString("mesh_size = 0;\n");

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Agros2D::scene()->nodes->length(); i++)
    {
        outNodes += QString("Point(%1) = {%2, %3, 0, mesh_size};\n").
                arg(i).
                arg(Agros2D::scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Agros2D::scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        if (Agros2D::scene()->edges->at(i)->angle() == 0)
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Line(%1) = {%2, %3};\n").
                    arg(edgesCount+1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()));
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Agros2D::scene()->edges->at(i)->center();
            outNodes += QString("Point(%1) = {%2, %3, 0};\n").
                    arg(nodesCount).
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Circle(%1) = {%2, %3, %4};\n").
                    arg(edgesCount+1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(nodesCount - 1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()));

            edgesCount++;
        }
    }

    /*
    // holes
    int holesCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        if (label->markersCount() == 0)
            holesCount++;

    QString outHoles = QString("%1\n").arg(holesCount);
    holesCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->markersCount() == 0)
        {
            outHoles += QString("%1  %2  %3\n").
                    arg(holesCount).
                    // arg(Agros2D::scene()->labels->items().indexOf(label) + 1).
                    arg(label->point().x, 0, 'f', 10).
                    arg(label->point().y, 0, 'f', 10);

            holesCount++;
        }
    }

    // labels
    QString outLabels;
    int labelsCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->markersCount() > 0)
        {
            outLabels += QString("%1  %2  %3  %4  %5\n").
                    arg(labelsCount).
                    arg(label->point().x, 0, 'f', 10).
                    arg(label->point().y, 0, 'f', 10).
                    // arg(labelsCount + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                    arg(Agros2D::scene()->labels->items().indexOf(label) + 1).
                    arg(label->area());
            labelsCount++;
        }
    }

    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").
                     arg(labelsCount)); // - holes
    out << outLabels;
    */

    try
    {
        Agros2D::scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        std::cout << "Missing Label";
        return false;
    }

    QString outLoops;
    for(int i = 0; i < Agros2D::scene()->loopsInfo()->loops().size(); i++)
    {
        if (!Agros2D::scene()->loopsInfo()->outsideLoops().contains(i))
        {
            outLoops.append(QString("Line Loop(%1) = {").arg(i+1));
            for(int j = 0; j < Agros2D::scene()->loopsInfo()->loops().at(i).size(); j++)
            {
                if (Agros2D::scene()->loopsInfo()->loops().at(i)[j].reverse)
                    outLoops.append("-");
                outLoops.append(QString("%1").arg(Agros2D::scene()->loopsInfo()->loops().at(i)[j].edge + 1));
                if (j < Agros2D::scene()->loopsInfo()->loops().at(i).size() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }
    outLoops.append("\n");

    QList<int> surfaces;
    int surfaceCount = 0;
    for (int i = 0; i < Agros2D::scene()->labels->count(); i++)
    {
        surfaceCount++;
        SceneLabel* label = Agros2D::scene()->labels->at(i);
        if(!label->isHole())
        {
            surfaces.push_back(surfaceCount);
            outLoops.append(QString("Plane Surface(%1) = {").arg(surfaceCount));
            for (int j = 0; j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count(); j++)
            {
                outLoops.append(QString("%1").arg(Agros2D::scene()->loopsInfo()->labelLoops()[label][j]+1));
                if (j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }

    //    outLoops.append(QString("Physical Surface(1) = {"));
    //    for(int i = 0; i < surfaceCount; i++)
    //    {
    //        outLoops.append(QString("%1").arg(i+1));
    //        if(i < surfaceCount - 1)
    //            outLoops.append(",");
    //    }
    //    outLoops.append(QString("};\n"));

    // quad mesh
    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad ||
            Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    {
        outLoops.append(QString("Recombine Surface {"));
        for(int i = 0; i <  surfaces.count(); i++)
        {
            outLoops.append(QString("%1").arg(surfaces.at(i)));
            if(i < surfaces.count() - 1)
                outLoops.append(",");
        }
        outLoops.append(QString("};\n"));
    }
    //    QString outLoops;
    //    outLoops.append(QString("Line Loop(1) = {0, 1, 2, 3};\n"));
    //    outLoops.append(QString("Plane Surface(1) = {1};\n"));
    //    outLoops.append(QString("Line Loop(2) = {4, 5, 6, -1};\n"));
    //    outLoops.append(QString("Plane Surface(2) = {2};\n"));
    //    outLoops.append("\n");

    //    // quad mesh
    //    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    //        outLoops.append(QString("Recombine Surface {1, 2};\n"));

    // Mesh.Algorithm - 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    QString outCommands;
    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Triangle)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
    }
    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }
    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    {
        outCommands.append(QString("Mesh.Algorithm = 8;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }

    outNodes.insert(0, QString("\n// nodes\n"));
    out << outNodes;
    outEdges.insert(0, QString("\n// edges\n"));
    out << outEdges;
    outLoops.insert(0, QString("\n// loops\n"));
    out << outLoops;
    outCommands.insert(0, QString("\n// commands\n"));
    out << outCommands;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

bool MeshGeneratorGMSH::readGmshMeshFile()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    int k;

    QFile fileGMSH(tempProblemFileName() + ".msh");
    if (!fileGMSH.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not read GMSH mesh file"));
        return false;
    }
    QTextStream inGMSH(&fileGMSH);

    // nodes
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    sscanf(inGMSH.readLine().toLatin1().data(), "%i", &k);
    for (int i = 0; i < k; i++)
    {
        int n;
        double x, y, z;

        sscanf(inGMSH.readLine().toLatin1().data(), "%i %lf %lf %lf", &n, &x, &y, &z);
        nodeList.append(Point(x, y));
    }

    // elements
    inGMSH.readLine();
    inGMSH.readLine();
    sscanf(inGMSH.readLine().toLatin1().data(), "%i", &k);
    QSet<int> labelMarkersCheck;
    for (int i = 0; i < k; i++)
    {
        int quad[4];
        int n, type, phys, part, marker;

        if (sscanf(inGMSH.readLine().toLatin1().data(), "%i %i %i %i %i %i %i %i %i",
                   &n, &type, &phys, &part, &marker, &quad[0], &quad[1], &quad[2], &quad[3]))
        {
            // edge
            if (type == 1)
                edgeList.append(MeshEdge(quad[0] - 1, quad[1] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
            // triangle
            if (type == 2)
                elementList.append(MeshElement(quad[0] - 1, quad[1] - 1, quad[2] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
            // quad
            if (type == 3)
                elementList.append(MeshElement(quad[0] - 1, quad[1] - 1, quad[2] - 1, quad[3] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
        }
        /*

        if (marker == 0)
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("Some areas have no label marker"));
            return false;
        }
        */
        labelMarkersCheck.insert(marker - 1);
    }

    fileGMSH.close();

    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}

