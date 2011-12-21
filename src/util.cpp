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

#ifdef WITH_UNITY
#include <unity.h>
#endif

#include "util.h"
#include "scene.h"
#include "scripteditordialog.h"
#include "style/manhattanstyle.h"

#include "hermes2d/module.h"

bool verbose = false;

static QHash<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QHash<Mode, QString> modeList;
static QHash<SceneViewPostprocessorShow, QString> sceneViewPostprocessorShowList;
static QHash<WeakFormsType, QString> weakFormsTypeList;
static QHash<AdaptivityType, QString> adaptivityTypeList;
static QHash<AnalysisType, QString> analysisTypeList;
static QHash<LinearityType, QString> linearityTypeList;
static QHash<MeshType, QString> meshTypeList;
static QHash<Hermes::MatrixSolverType, QString> matrixSolverTypeList;

QString analysisTypeToStringKey(AnalysisType analysisType) { return analysisTypeList[analysisType]; }
AnalysisType analysisTypeFromStringKey(const QString &analysisType) { return analysisTypeList.key(analysisType); }

QString weakFormsTypeToStringKey(WeakFormsType weakFormsType) { return weakFormsTypeList[weakFormsType]; }
WeakFormsType weakFormsTypeFromStringKey(const QString &weakFormsType) { return weakFormsTypeList.key(weakFormsType); }

QString meshTypeToStringKey(MeshType meshType) { return meshTypeList[meshType]; }
MeshType meshTypeFromStringKey(const QString &meshType) { return meshTypeList.key(meshType); }

QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QString modeToStringKey(Mode mode) { return modeList[mode]; }
Mode modeFromStringKey(const QString &mode) { return modeList.key(mode); }

QString sceneViewPostprocessorShowToStringKey(SceneViewPostprocessorShow sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList[sceneViewPostprocessorShow]; }
SceneViewPostprocessorShow sceneViewPostprocessorShowFromStringKey(const QString &sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList.key(sceneViewPostprocessorShow); }

QString adaptivityTypeToStringKey(AdaptivityType adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

QString linearityTypeToStringKey(LinearityType linearityType) { return linearityTypeList[linearityType]; }
LinearityType linearityTypeFromStringKey(const QString &linearityType) { return linearityTypeList.key(linearityType); }

QString matrixSolverTypeToStringKey(Hermes::MatrixSolverType matrixSolverType) { return matrixSolverTypeList[matrixSolverType]; }
Hermes::MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType) { return matrixSolverTypeList.key(matrixSolverType); }

void initLists()
{
    logMessage("initLists()");

    // Analysis Type
    analysisTypeList.insert(AnalysisType_Undefined, "");
    analysisTypeList.insert(AnalysisType_SteadyState, "steadystate");
    analysisTypeList.insert(AnalysisType_Transient, "transient");
    analysisTypeList.insert(AnalysisType_Harmonic, "harmonic");

    // WeakForms Type
    weakFormsTypeList.insert(WeakFormsType_Undefined, "");
    weakFormsTypeList.insert(WeakFormsType_Interpreted, "interpreted");
    weakFormsTypeList.insert(WeakFormsType_Compiled, "compiled");

    // Mesh Type
    meshTypeList.insert(MeshType_Triangle, "triangle");
    meshTypeList.insert(MeshType_QuadFineDivision, "quad_fine_division");
    meshTypeList.insert(MeshType_QuadRoughDivision, "quad_rough_division");
    meshTypeList.insert(MeshType_QuadJoin, "quad_join");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Undefined, "");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Scalar, "scalar");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Magnitude, "magnitude");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_X, "x");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Y, "y");

    // PHYSICFIELDBC
    /*§
    physicFieldBCList.insert(PhysicFieldBC_Undefined, "");
    physicFieldBCList.insert(PhysicFieldBC_None, "none");
    physicFieldBCList.insert(PhysicFieldBC_General_Value, "general_value");
    physicFieldBCList.insert(PhysicFieldBC_General_Derivative, "general_derivative");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Velocity, "flow_velocity");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Pressure, "flow_pressure");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Outlet, "flow_outlet");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Wall, "flow_wall");
    physicFieldBCList.insert(PhysicFieldBC_RF_ElectricField, "rf_electric_field");
    physicFieldBCList.insert(PhysicFieldBC_RF_SurfaceCurrent, "rf_surface_current");
    physicFieldBCList.insert(PhysicFieldBC_RF_MatchedBoundary, "rf_matched_boundary");
    physicFieldBCList.insert(PhysicFieldBC_RF_Port, "rf_port");
    */

    // TEMODE
    modeList.insert(Mode_0, "mode_0");
    modeList.insert(Mode_01, "mode_01");
    modeList.insert(Mode_02, "mode_02");

    // SCENEVIEW_POSTPROCESSOR_SHOW
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_Undefined, "");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_None, "none");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_ScalarView, "scalar");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_ScalarView3D, "scalar3d");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_Order, "order");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(AdaptivityType_Undefined, "");
    adaptivityTypeList.insert(AdaptivityType_None, "disabled");
    adaptivityTypeList.insert(AdaptivityType_H, "h-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_P, "p-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_HP, "hp-adaptivity");

    // MatrixSolverType
    matrixSolverTypeList.insert(Hermes::SOLVER_UMFPACK, "umfpack");
    matrixSolverTypeList.insert(Hermes::SOLVER_PETSC, "petsc");
    matrixSolverTypeList.insert(Hermes::SOLVER_MUMPS, "mumps");
    matrixSolverTypeList.insert(Hermes::SOLVER_SUPERLU, "superlu");
    matrixSolverTypeList.insert(Hermes::SOLVER_AMESOS, "trilinos_amesos");
    matrixSolverTypeList.insert(Hermes::SOLVER_AZTECOO, "trilinos_aztecoo");

    // LinearityType
    linearityTypeList.insert(LinearityType_Undefined, "");
    linearityTypeList.insert(LinearityType_Linear, "linear");
    linearityTypeList.insert(LinearityType_Picard, "picard");
    linearityTypeList.insert(LinearityType_Newton, "newton");
}

QString analysisTypeString(AnalysisType analysisType)
{
    logMessage("analysisTypeString()");

    switch (analysisType)
    {
    case AnalysisType_SteadyState:
        return QObject::tr("Steady state");
    case AnalysisType_Transient:
        return QObject::tr("Transient");
    case AnalysisType_Harmonic:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

QString teModeString(Mode teMode)
{
    logMessage("TEModeString()");

    switch (teMode)
    {
    case Mode_0:
        return QObject::tr("TE Mode 0");
    case Mode_01:
        return QObject::tr("TE Mode 01");
    case Mode_02:
        return QObject::tr("TE Mode 02");
    default:
        std::cerr << "TE mode '" + QString::number(teMode).toStdString() + "' is not implemented. TEModeString(TEMode teMode)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    logMessage("physicFieldVariableCompString()");

    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Scalar:
        return QObject::tr("Scalar");
    case PhysicFieldVariableComp_Magnitude:
        return QObject::tr("Magnitude");
    case PhysicFieldVariableComp_X:
        return Util::scene()->problemInfo()->labelX();
    case PhysicFieldVariableComp_Y:
        return Util::scene()->problemInfo()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString coordinateTypeString(CoordinateType coordinateType)
{
    logMessage("problemTypeString()");

    return ((coordinateType == CoordinateType_Planar) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    logMessage("adaptivityTypeString()");

    switch (adaptivityType)
    {
    case AdaptivityType_None:
        return QObject::tr("Disabled");
    case AdaptivityType_H:
        return QObject::tr("h-adaptivity");
    case AdaptivityType_P:
        return QObject::tr("p-adaptivity");
    case AdaptivityType_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

QString weakFormsTypeString(WeakFormsType weakFormsType)
{
    logMessage("weakFormsTypeString()");

    switch (weakFormsType)
    {
    case WeakFormsType_Interpreted:
        return QObject::tr("Interpreted");
    case WeakFormsType_Compiled:
        return QObject::tr("Compiled");
    default:
        std::cerr << "Weak forms type '" + QString::number(weakFormsType).toStdString() + "' is not implemented. weakFormsTypeString(WeakFormsType weakFormsType)" << endl;
        throw;
    }
}

QString meshTypeString(MeshType meshType)
{
    logMessage("meshTypeString()");

    switch (meshType)
    {
    case MeshType_Triangle:
        return QObject::tr("Triangle");
    case MeshType_QuadFineDivision:
        return QObject::tr("Quad fine div.");
    case MeshType_QuadRoughDivision:
        return QObject::tr("Quad rough div.");
    case MeshType_QuadJoin:
        return QObject::tr("Quad join");
    default:
        std::cerr << "Mesh type '" + QString::number(meshType).toStdString() + "' is not implemented. meshTypeString(MeshType meshType)" << endl;
        throw;
    }
}

QString matrixSolverTypeString(Hermes::MatrixSolverType matrixSolverType)
{
    logMessage("matrixSolverTypeString()");

    switch (matrixSolverType)
    {
    case Hermes::SOLVER_UMFPACK:
        return QObject::tr("UMFPACK");
    case Hermes::SOLVER_PETSC:
        return QObject::tr("PETSc");
    case Hermes::SOLVER_MUMPS:
        return QObject::tr("MUMPS");
    case Hermes::SOLVER_SUPERLU:
        return QObject::tr("SuperLU");
    case Hermes::SOLVER_AMESOS:
        return QObject::tr("Trilinos/Amesos");
    case Hermes::SOLVER_AZTECOO:
        return QObject::tr("Trilinos/AztecOO");
    default:
        std::cerr << "Matrix solver type '" + QString::number(matrixSolverType).toStdString() + "' is not implemented. matrixSolverTypeString(MatrixSolverType matrixSolverType)" << endl;
        throw;
    }
}

QString linearityTypeString(LinearityType linearityType)
{
    logMessage("linearityTypeString()");

    switch (linearityType)
    {
    case LinearityType_Linear:
        return QObject::tr("Linear");
    case LinearityType_Picard:
        return QObject::tr("Picard's method");
    case LinearityType_Newton:
        return QObject::tr("Newton's method");
    default:
        std::cerr << "Linearity type '" + QString::number(linearityType).toStdString() + "' is not implemented. linearityTypeString(LinearityType linearityType)" << endl;
        throw;
    }
}

void fillComboBoxPhysicField(QComboBox *cmbPhysicField)
{
    logMessage("fillComboBoxPhysicField()");

    // block signals
    cmbPhysicField->blockSignals(true);

    cmbPhysicField->clear();
    std::map<std::string, std::string> modules = availableModules();
    for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
        cmbPhysicField->addItem(QString::fromStdString(it->second), QString::fromStdString(it->first));

    // unblock signals
    cmbPhysicField->blockSignals(false);

    // FIXME - experimental features
    // if (Util::config()->showExperimentalFeatures)

    // default physic field
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(Util::config()->defaultPhysicField));
    if (cmbPhysicField->currentIndex() == -1)
        cmbPhysicField->setCurrentIndex(0);
}

void setGUIStyle(const QString &styleName)
{
    logMessage("setGUIStyle()");

    QStyle *style = NULL;
    if (styleName == "Manhattan")
    {
        QString styleName = "";
        QStringList styles = QStyleFactory::keys();

#ifdef Q_WS_X11
        // kde 3
        if (getenv("KDE_FULL_SESSION") != NULL)
            styleName = "Plastique";
        // kde 4
        if (getenv("KDE_SESSION_VERSION") != NULL)
        {
            if (styles.contains("Oxygen"))
                styleName = "Oxygen";
            else
                styleName = "Plastique";
        }

        // gtk+
        if (styleName == "")
            styleName = "GTK+";
#else
        styleName = "Plastique";
#endif

        style = new ManhattanStyle(styleName);
    }
    else
    {
        // standard style
        style = QStyleFactory::create(styleName);
    }

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware())
    {
        QApplication::setPalette(QApplication::palette());
    }
}

void setLanguage(const QString &locale)
{
    logMessage("setLanguage()");

    // non latin-1 chars
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator *qtTranslator = new QTranslator();
    QTranslator *appTranslator = new QTranslator();

    QString country = locale.section('_',0,0);
    if (QFile::exists(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm"))
        qtTranslator->load(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/qt_" + country + ".qm"))
        qtTranslator->load(datadir() + LANGUAGEROOT + "/qt_" + country + ".qm");
    else
        qDebug() << "Qt language file not found.";

    if (QFile::exists(datadir() + LANGUAGEROOT + "/" + locale + ".qm"))
        appTranslator->load(datadir() + LANGUAGEROOT + "/" + locale + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/en_US.qm"))
        appTranslator->load(datadir() + LANGUAGEROOT + "/en_US.qm");
    else
        qDebug() << "Language file not found.";

    QApplication::installTranslator(qtTranslator);
    QApplication::installTranslator(appTranslator);
}

QStringList availableLanguages()
{
    logMessage("availableLanguages()");

    QDir dir;
    dir.setPath(datadir() + LANGUAGEROOT);

    // add all translations
    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // remove extension
    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    // remove system translations
    foreach (QString str, list)
        if (str.startsWith("qt_"))
            list.removeOne(str);

    return list;
}

QIcon icon(const QString &name)
{
    QString fileName;

#ifdef Q_WS_WIN
    if (QFile::exists(":/" + name + "-windows.png")) return QIcon(":/" + name + "-windows.png");
#endif

#ifdef Q_WS_X11
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    return QIcon::fromTheme(name, QIcon(":/" + name + ".png"));
#endif

    QDir dir;

    QString style = "";
    QStringList styles = QStyleFactory::keys();

    // kde 3
    if (getenv("KDE_FULL_SESSION") != NULL)
    {}
    // kde 4
    if (getenv("KDE_SESSION_VERSION") != NULL)
    {
        // oxygen
        fileName = "/usr/share/icons/oxygen/22x22/actions/" + name;
        if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
    // gtk+
    if (style == "")
    {
        // humanity (disabled - corrupted svg reader - Qt 4.6 has new method QIcon::fromTheme)
        // fileName = "/usr/share/icons/Humanity/actions/24/" + name;
        // if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        // if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
#endif

    if (QFile::exists(":/" + name + ".svg")) return QIcon(":/" + name + ".svg");
    if (QFile::exists(":/" + name + ".png")) return QIcon(":/" + name + ".png");

    return QIcon();
}

QString datadir()
{
    logMessage("datadir()");

    // windows and local installation
    if (QFile::exists(QApplication::applicationDirPath() + "/functions.py"))
        return QApplication::applicationDirPath();

    // linux
    if (QFile::exists(QApplication::applicationDirPath() + "/../share/agros2d/functions.py"))
        return QApplication::applicationDirPath() + "/../share/agros2d";

    qCritical() << "Datadir not found.";
    exit(1);
}

QString tempProblemDir()
{
    logMessage("tempProblemDir()");

    QDir(QDir::temp().absolutePath()).mkpath("agros2d/" + QString::number(QApplication::applicationPid()));

    return QString("%1/agros2d/%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QString tempProblemFileName()
{
    logMessage("tempProblemFileName()");

    return tempProblemDir() + "/temp";
}

QTime milisecondsToTime(int ms)
{
    logMessage("milisecondsToTime()");

    // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floorf(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floorf(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floorf(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floorf(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}

bool removeDirectory(const QDir &dir)
{
    logMessage("removeDirectory()");

    bool error = false;

    if (dir.exists())
    {
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                error = removeDirectory(QDir(path));
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    error = true;
                    break;
                }
            }
        }
        if (!dir.rmdir(dir.absolutePath()))
            error = true;
    }

    return error;
}

void msleep(unsigned long msecs)
{
    logMessage("msleep()");

    QWaitCondition w;
    QMutex sleepMutex;
    sleepMutex.lock();
    w.wait(&sleepMutex, msecs);
    sleepMutex.unlock();
}

// verbose
void setVerbose(bool verb)
{
    verbose = verb;
}

QString formatLogMessage(QtMsgType type, const QString &msg)
{
    QString msgType = "";

    switch (type) {
    case QtDebugMsg:
        msgType = "Debug";
        break;
    case QtWarningMsg:
        msgType = "Warning";
        break;
    case QtCriticalMsg:
        msgType = "Critical";
        break;
    case QtFatalMsg:
        msgType = "Fatal";
        break;
    }

    QString str = QString("%1 %2: %3").
            arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")).
            arg(msgType).
            arg(msg);

    return str;
}

void appendToFile(const QString &fileName, const QString &str)
{
    QFile file(fileName);

    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream outFile(&file);
        outFile << str << endl;

        file.close();
    }
}

void logOutput(QtMsgType type, const char *msg)
{
    QString str = formatLogMessage(type, msg);

    // string
    fprintf(stderr, "%s\n", str.toStdString().c_str());

    if (Util::config()->enabledApplicationLog)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QDir("/").mkpath(location);

        appendToFile(location + "/app.log", str);
    }

    if (type == QtFatalMsg)
        abort();
}

void logMessage(const QString &msg)
{
    if (verbose)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QDir("/").mkpath(location);

        appendToFile(location + "/app.log", formatLogMessage(QtDebugMsg, msg));
    }
}

void showPage(const QString &str)
{
    logMessage("showPage()");

    if (str.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/index.html"));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/" + str));
}


QString readFileContent(const QString &fileName)
{
    logMessage("readFileContent()");

    QString content;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContent(const QString &fileName, QString *content)
{
    logMessage("writeStringContent()");

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << *content;

        file.waitForBytesWritten(0);
        file.close();
    }
}

QByteArray readFileContentByteArray(const QString &fileName)
{
    logMessage("eadFileContentByteArray()");

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContentByteArray(const QString &fileName, QByteArray content)
{
    logMessage("writeStringContentByteArray()");

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(content);

        file.waitForBytesWritten(0);
        file.close();
    }
}

Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle)
{
    double distance = (pointEnd - pointStart).magnitude();
    Point t = (pointEnd - pointStart) / distance;
    double R = distance / (2.0*sin(angle/180.0*M_PI / 2.0));

    Point p = Point(distance/2.0, sqrt(Hermes::sqr(R) - Hermes::sqr(distance)/4.0 > 0.0 ? Hermes::sqr(R) - Hermes::sqr(distance)/4.0 : 0.0));
    Point center = pointStart + Point(p.x*t.x - p.y*t.y, p.x*t.y + p.y*t.x);

    return center;
}

static CheckVersion *checkVersion = NULL;
void checkForNewVersion(bool quiet)
{
    logMessage("checkForNewVersion()");

    // download version
    QUrl url("http://agros2d.org/version/version.xml");
    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url);

    checkVersion->run(quiet);
}

QString unitToHTML(const QString &str)
{
    // dirty hack
    QString out = str;

    out.replace("-2", "<sup>&#8722;2</sup>");
    out.replace("-3", "<sup>&#8722;3</sup>");
    out.replace("2", "<sup>2</sup>");
    out.replace("3", "<sup>3</sup>");

    return out;
}

CheckVersion::CheckVersion(QUrl url) : QObject()
{
    logMessage("CheckVersion::CheckVersion()");

    m_url = url;

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    logMessage("CheckVersion::~CheckVersion()");

    delete m_manager;
}

void CheckVersion::run(bool quiet)
{
    logMessage("CheckVersion::run()");

    m_quiet = quiet;
    m_networkReply = m_manager->get(QNetworkRequest(m_url));

    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(showProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    logMessage("CheckVersion::downloadFinished()");

    QString text = networkReply->readAll();

    if (!text.isEmpty())
    {
        QDomDocument doc;
        doc.setContent(text);

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problems
        QDomNode eleVersion = eleDoc.toElement().elementsByTagName("version").at(0);

        int beta = eleVersion.toElement().attribute("beta").toInt() == 1;
        int major = eleVersion.toElement().attribute("major").toInt();
        int minor = eleVersion.toElement().attribute("minor").toInt();
        int sub = eleVersion.toElement().attribute("sub").toInt();
        int git = eleVersion.toElement().attribute("git").toInt();
        int year = eleVersion.toElement().attribute("year").toInt();
        int month = eleVersion.toElement().attribute("month").toInt();
        int day = eleVersion.toElement().attribute("day").toInt();

        QDomNode eleUrl = eleDoc.toElement().elementsByTagName("url").at(0);

        if (!m_quiet && git == 0)
        {
            QMessageBox::critical(QApplication::activeWindow(), tr("New version"), tr("File is corrupted or network is disconnected."));
            return;
        }

        QString downloadUrl = eleUrl.toElement().text();
        if (git > VERSION_GIT)
        {
            QString str(tr("<b>New version available.</b><br/><br/>"
                           "Actual version: %1<br/>"
                           "New version: %2<br/><br/>"
                           "URL: <a href=\"%3\">%3</a>").
                        arg(QApplication::applicationVersion()).
                        arg(versionString(major, minor, sub, git, year, month, day, beta)).
                        arg(downloadUrl));

            QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QMessageBox::information(QApplication::activeWindow(), tr("New version"), tr("You are using actual version."));
        }
    }
}

void CheckVersion::showProgress(qint64 dl, qint64 all)
{
    logMessage("CheckVersion::showProgress()");

    // qDebug() << QString("\rDownloaded %1 bytes of %2).").arg(dl).arg(all);
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    logMessage("CheckVersion::handleError()");

    qDebug() << "An error ocurred (code #" << error << ").";
}

QString transformXML(const QString &fileName, const QString &stylesheetFileName)
{
    QString out;

    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(QUrl(fileName));
    query.setQuery(QUrl(stylesheetFileName));
    query.evaluateTo(&out);

    return out;
}

ErrorResult validateXML(const QString &fileName, const QString &schemaFileName)
{
    QXmlSchema schema;
    schema.load(QUrl(schemaFileName));

    MessageHandler schemaMessageHandler;
    schema.setMessageHandler(&schemaMessageHandler);

    if (!schema.isValid())
        return ErrorResult(ErrorResultType_Critical, QObject::tr("Schena '%1' is not valid. %2").
                           arg(schemaFileName).
                           arg(schemaMessageHandler.statusMessage()));

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QXmlSchemaValidator validator(schema);
    MessageHandler validatorMessageHandler;
    validator.setMessageHandler(&validatorMessageHandler);

    if (!validator.validate(&file, QUrl::fromLocalFile(file.fileName())))
        return ErrorResult(ErrorResultType_Critical, QObject::tr("File '%1' is not valid Agros2D problem file. Error (line %3, column %4): %2").
                           arg(fileName).
                           arg(validatorMessageHandler.statusMessage()).
                           arg(validatorMessageHandler.line()).
                           arg(validatorMessageHandler.column()));

    return ErrorResult();
}
