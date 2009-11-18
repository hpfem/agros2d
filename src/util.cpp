#include "util.h"
#include "scene.h"
#include "scripteditordialog.h"

static bool logFile;

static QHash<PhysicField, QString> physicFieldList;
static QHash<PhysicFieldVariable, QString> physicFieldVariableList;
static QHash<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QHash<PhysicFieldBC, QString> physicFieldBCList;
static QHash<SceneViewPostprocessorShow, QString> sceneViewPostprocessorShowList;
static QHash<AdaptivityType, QString> adaptivityTypeList;

QString physicFieldToStringKey(PhysicField physicField) { return physicFieldList[physicField]; }
PhysicField physicFieldFromStringKey(const QString &physicField) { return physicFieldList.key(physicField); }

QString physicFieldVariableToStringKey(PhysicFieldVariable physicFieldVariable) { return physicFieldVariableList[physicFieldVariable]; }
PhysicFieldVariable physicFieldVariableFromStringKey(const QString &physicFieldVariable) { return physicFieldVariableList.key(physicFieldVariable); }

QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QString physicFieldBCToStringKey(PhysicFieldBC physicFieldBC) { return physicFieldBCList[physicFieldBC]; }
PhysicFieldBC physicFieldBCFromStringKey(const QString &physicFieldBC) { return physicFieldBCList.key(physicFieldBC); }

QString sceneViewPostprocessorShowToStringKey(SceneViewPostprocessorShow sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList[sceneViewPostprocessorShow]; }
SceneViewPostprocessorShow sceneViewPostprocessorShowFromStringKey(const QString &sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList.key(sceneViewPostprocessorShow); }

QString adaptivityTypeToStringKey(AdaptivityType adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

void initLists()
{
    // PHYSICFIELD
    physicFieldList.insert(PHYSICFIELD_UNDEFINED, "");
    physicFieldList.insert(PHYSICFIELD_GENERAL, "general");
    physicFieldList.insert(PHYSICFIELD_ELECTROSTATIC, "electrostatic");
    physicFieldList.insert(PHYSICFIELD_MAGNETOSTATIC, "magnetostatic");
    physicFieldList.insert(PHYSICFIELD_CURRENT, "current");
    physicFieldList.insert(PHYSICFIELD_HEAT, "heat");
    physicFieldList.insert(PHYSICFIELD_ELASTICITY, "elasticity");
    physicFieldList.insert(PHYSICFIELD_HARMONICMAGNETIC, "harmonicmagnetic");

    // PHYSICFIELDVARIABLE
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_UNDEFINED, "");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_GENERAL_VARIABLE, "general_variable");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_GENERAL_GRADIENT, "general_gradient");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_GENERAL_CONSTANT, "general_constant");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL, "electrostatic_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD, "electrostatic_electric_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT, "electrostatic_displacement");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY, "electrostatic_energy_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY, "electrostatic_permittivity");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL, "magnetostatic_vector_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY, "magnetostatic_flux_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD, "magnetostatic_magnetic_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY, "magnetostatic_energy_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY, "magnetostatic_permeability");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL, "harmonicmagnetic_vector_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_REAL, "harmonicmagnetic_vector_potential_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_IMAG, "harmonicmagnetic_vector_potential_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY, "harmonicmagnetic_flux_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_REAL, "harmonicmagnetic_flux_density_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_IMAG, "harmonicmagnetic_flux_density_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD, "harmonicmagnetic_magnetic_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_REAL, "harmonicmagnetic_magnetic_field_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_IMAG, "harmonicmagnetic_magnetic_field_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL, "harmonicmagnetic_total_current_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_REAL, "harmonicmagnetic_total_current_density_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_IMAG, "harmonicmagnetic_total_current_density_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED, "harmonicmagnetic_induced_current_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_REAL, "harmonicmagnetic_induced_current_density_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_IMAG, "harmonicmagnetic_induced_current_density_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_POWER_LOSSES, "harmonicmagnetic_power_losses");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_ENERGY_DENSITY, "harmonicmagnetic_energy_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_PERMEABILITY, "harmonicmagnetic_permeability");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_POTENTIAL, "current_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD, "current_electic_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY, "current_current_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_LOSSES, "current_power_losses");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY, "current_conductivity");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE, "heat_temperature");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT, "heat_temperature_gradient");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_FLUX, "heat_heat_flux");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY, "heat_conductivity");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS, "elasticity_von_mises_stress");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_UNDEFINED, "");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_SCALAR, "scalar");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_MAGNITUDE, "magnitude");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_X, "x");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_Y, "y");

    // PHYSICFIELDBC
    physicFieldBCList.insert(PHYSICFIELDBC_UNDEFINED, "");
    physicFieldBCList.insert(PHYSICFIELDBC_NONE, "none");
    physicFieldBCList.insert(PHYSICFIELDBC_GENERAL_VALUE, "general_value");
    physicFieldBCList.insert(PHYSICFIELDBC_GENERAL_DERIVATIVE, "general_derivative");
    physicFieldBCList.insert(PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL, "electrostatic_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE, "electrostatic_surface_charge_density");
    physicFieldBCList.insert(PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL, "magnetostatic_vector_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT, "magnetostatic_surface_current_density");
    physicFieldBCList.insert(PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL, "harmonicmagnetic_vector_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT, "harmonicmagnetic_surface_current_density");
    physicFieldBCList.insert(PHYSICFIELDBC_HEAT_TEMPERATURE, "heat_temperature");
    physicFieldBCList.insert(PHYSICFIELDBC_HEAT_HEAT_FLUX, "heat_heat_flux");
    physicFieldBCList.insert(PHYSICFIELDBC_CURRENT_POTENTIAL, "current_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW, "current_inward_current_flow");
    physicFieldBCList.insert(PHYSICFIELDBC_ELASTICITY_FIXED, "elasticity_fixed");
    physicFieldBCList.insert(PHYSICFIELDBC_ELASTICITY_FREE, "elasticity_free");

    // SCENEVIEW_POSTPROCESSOR_SHOW
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED, "");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_NONE, "none");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW, "scalar");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D, "scalar3d");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_ORDER, "order");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(ADAPTIVITYTYPE_UNDEFINED, "");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_NONE, "disabled");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_H, "h-adaptivity");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_P, "p-adaptivity");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_HP, "hp-adaptivity");    
}

bool Value::evaluate()
{
    ScriptResult scriptResult = runPythonExpression(text);
    if (!scriptResult.isError)
    {
        number = scriptResult.value;
    }
    else
    {
        QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error"), scriptResult.text);
    }

    return !scriptResult.isError;
};

void enableLogFile(bool enable)
{
    logFile = enable;
}

void setGUIStyle(const QString &styleName)
{
    QStyle *style = QStyleFactory::create(styleName);

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware())
    {
        QApplication::setPalette(QApplication::palette());
    }
}

void setLanguage(const QString &locale)
{
    // non latin-1 chars
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator *translator = new QTranslator();
    translator->load(datadir() + "/lang/" + locale + ".qm");
    QApplication::installTranslator(translator);
}

QStringList availableLanguages()
{
    QDir dir;
    dir.setPath(datadir() + "/lang");

    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    return list;
}

QIcon icon(const QString &name)
{
    QString fileName;

#ifdef Q_WS_WIN
    if (QFile::exists(":/images/" + name + "-windows.png")) return QIcon(":/images/" + name + "-windows.png");
#endif

#ifdef Q_WS_X11
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

    if (QFile::exists(":images/" + name + ".svg")) return QIcon(":images/" + name + ".svg");
    if (QFile::exists(":images/" + name + ".png")) return QIcon(":images/" + name + ".png");

    return QIcon();
}

QString datadir()
{
    // actual data dir
    QDir dirData;
    dirData.setPath(QDir::current().absolutePath() + "/data");
    if (dirData.exists())
        return QDir::current().absolutePath();

    // linux
    dirData.setPath(QApplication::applicationDirPath() + "/../share/agros2d/data");
    if (dirData.exists())
        return QApplication::applicationDirPath() + "/../share/agros2d";

    // windows
    dirData.setPath(QApplication::applicationDirPath() + "/data");
    if (dirData.exists())
        return QApplication::applicationDirPath() + "/data";

    cerr << "Datadir not found." << endl;
    exit(1);
}

QString tempProblemDir()
{
    QDir(QDir::temp().absolutePath()).mkpath("agros2d/" + QString::number(QApplication::applicationPid()));

    return QString("%1/agros2d/%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QString tempProblemFileName()
{
    return tempProblemDir() + "/temp";
}

QTime milliSecondsToTime(int ms)
{
    // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floor(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floor(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floor(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floor(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}

bool removeDirectory(const QDir &dir)
{
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
    QWaitCondition w;
    QMutex sleepMutex;
    sleepMutex.lock();
    w.wait(&sleepMutex, msecs);
    sleepMutex.unlock();
}

void log(const QString &message)
{
    if (logFile)
    {
        QFile file(QApplication::applicationDirPath() + "/log.txt");

        if (file.open(QIODevice::Append))
        {
            QTextStream outFile(&file);
            outFile << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") << ": " << message << endl;

            file.close();
        }
    }
}

QString readFileContent(const QString &fileName)
{
    QString content;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
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
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(content);

        file.waitForBytesWritten(0);
        file.close();
    }
}

void exception_global()
{
    try
    {
        throw;
    }
    catch (exception& e)
    {
        cerr << "Exiting due to error: " << e.what() << endl;
    }
    return;
}
