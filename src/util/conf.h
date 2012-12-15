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

#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"
#include "util/enums.h"

class ConfigComputer : public QObject
{
    Q_OBJECT

public:
    ConfigComputer();
    ~ConfigComputer();

    // general
    QString guiStyle;
    QString language;
    QString defaultPhysicField;

    bool showLogStdOut;

    bool checkVersion;
    bool lineEditValueShowResult;
    bool saveProblemWithSolution;

    QString collaborationServerURL;

    // global script
    QString globalScript;

    // delete files
    bool deleteMeshFiles;
    bool deleteHermesMeshFile;

    // discrete
    bool saveMatrixRHS;

    // cache
    int cacheSize;

    // number of threads
    int numberOfThreads;

    void load();
    void save();
};

class Config : public QObject
{
    Q_OBJECT

public:
    Config();
    ~Config();

    // font
    QString rulersFont;
    QString postFont;

    // zoom
    bool zoomToMouse;

    // geometry
    double nodeSize;
    double edgeWidth;
    double labelSize;

    // rulers
    bool showRulers;

    // colors
    QColor colorBackground;
    QColor colorGrid;
    QColor colorCross;
    QColor colorNodes;
    QColor colorEdges;
    QColor colorLabels;
    QColor colorContours;
    QColor colorVectors;
    QColor colorInitialMesh;
    QColor colorSolutionMesh;
    QColor colorHighlighted;
    QColor colorCrossed;
    QColor colorSelected;
    QColor colorNotConnected;

    // grid
    bool showGrid;
    double gridStep;
    bool snapToGrid;

    // axes
    bool showAxes;

    // deformations
    bool deformScalar;
    bool deformContour;
    bool deformVector;

    // 3d
    bool scalarView3DLighting;
    double scalarView3DAngle;
    bool scalarView3DBackground;
    double scalarView3DHeight;
    bool scalarView3DBoundingBox;

    // active field
    QString activeField;

    // post3d
    SceneViewPost3DMode showPost3D;

    // linearizer quality
    PaletteQuality linearizerQuality;

    // contour
    bool showContourView;
    QString contourVariable;
    int contoursCount;
    double contourWidth;

    // scalar view
    bool showScalarView;
    QString scalarVariable;
    PhysicFieldVariableComp scalarVariableComp;
    bool scalarRangeAuto;
    double scalarRangeMin;
    double scalarRangeMax;
    bool scalarRangeLog;
    double scalarRangeBase;
    int scalarDecimalPlace;

    // vector view
    bool showVectorView;
    QString vectorVariable;
    bool vectorProportional;
    bool vectorColor;
    int vectorCount;
    double vectorScale;
    VectorType vectorType;
    VectorCenter vectorCenter;

    // mesh
    bool showInitialMeshView;
    bool showSolutionMeshView;
    int angleSegmentsCount;
    bool curvilinearElements;

    // order view
    bool showOrderView;
    bool showOrderColorBar;
    PaletteOrderType orderPaletteOrderType;
    bool orderLabel;

    // palette
    bool showScalarColorBar;
    PaletteType paletteType;
    int paletteSteps;
    bool paletteFilter;

    // particle tracing
    bool showParticleView;
    bool particleIncludeRelativisticCorrection;
    double particleMass;
    double particleConstant;
    double particleMaximumRelativeError;
    double particleMinimumStep;
    int particleMaximumNumberOfSteps;
    bool particleReflectOnDifferentMaterial;
    bool particleReflectOnBoundary;
    double particleCoefficientOfRestitution;
    Point particleStart;
    Point particleStartVelocity;
    Point particleAcceleration;
    Point3 particleCustomForce;
    int particleNumberOfParticles;
    double particleStartingRadius;
    bool particleColorByVelocity;
    bool particleShowPoints;
    double particleDragDensity;
    double particleDragCoefficient;
    double particleDragReferenceArea;

    // solid view
    QStringList solidViewHide;

    // adaptivity
    int maxDofs;
    bool isoOnly;
    double convExp;
    double threshold;
    int strategy;
    int meshRegularity;
    Hermes::Hermes2D::ProjNormType projNormType;
    bool useAniso;
    bool finerReference;

    // command argument
    QString commandTriangle;
    QString commandGmsh;

    void load(QDomElement *config);
    void save(QDomElement *config);

private:
    QDomElement *eleConfig;

    bool readConfig(const QString &key, bool defaultValue);
    int readConfig(const QString &key, int defaultValue);
    double readConfig(const QString &key, double defaultValue);
    QString readConfig(const QString &key, const QString &defaultValue);
    QColor readConfig(const QString &key, const QColor &defaultValue);
    QStringList readConfig(const QString &key, const QStringList &defaultValue);

    void writeConfig(const QString &key, bool value);
    void writeConfig(const QString &key, int value);
    void writeConfig(const QString &key, double value);
    void writeConfig(const QString &key, const QString &value);
    void writeConfig(const QString &key, const QColor &value);
    void writeConfig(const QString &key, const QStringList &value);
};

#endif // CONFIG_H
