#include "coupling.h"
#include "scene.h"
#include "util.h"
#include "module.h"
#include "weakform_parser.h"

#include <dirent.h>

CouplingInfo::CouplingInfo(FieldInfo *sourceField, FieldInfo *targetField) :
    m_sourceField(sourceField), m_targetField(targetField)
{
    assert(m_sourceField->problemInfo() == m_targetField->problemInfo());
    m_problemInfo = m_sourceField->problemInfo();

    //TODO in each module should be implicit value
    m_couplingType = CouplingType_Weak;
    m_coupling = NULL;

    reload();
}

CouplingInfo::~CouplingInfo()
{
//    cout << "DESTRUCTOR !!!!!!!!!!!!!!!!" << endl;
    if(m_coupling)
        delete m_coupling;
}

void CouplingInfo::setCouplingType(CouplingType couplingType)
{
    m_couplingType = couplingType;

//    cout << "set type " << couplingTypeString(m_couplingType).toStdString() << endl;
    reload();
}

void CouplingInfo::reload()
{
    if(m_coupling)
        delete m_coupling;

    m_coupling = couplingFactory(m_sourceField, m_targetField, m_couplingType);
}

bool isCouplingAvailable(FieldInfo* sourceField, FieldInfo* targetField)
{
    DIR *dp;
    if ((dp = opendir((datadir()+ COUPLINGROOT).toStdString().c_str())) == NULL)
        error("Couplings dir '%s' doesn't exists", (datadir() + COUPLINGROOT).toStdString().c_str());

    struct dirent *dirp;
    while ((dirp = readdir(dp)) != NULL)
    {
        std::string filename = dirp->d_name;

        // skip current and parent dir
        if (filename == "." || filename == "..")
            continue;

        if (filename.substr(filename.size() - 4, filename.size() - 1) == ".xml")
        {
            // read name
            rapidxml::file<> file_data((datadir().toStdString() + COUPLINGROOT.toStdString() + "/" + filename).c_str());

            // parse xml
            rapidxml::xml_document<> doc;
            doc.parse<0>(file_data.data());

            // module name
            QString sourceFieldStr(doc.first_node("coupling")->first_node("general")->first_node("modules")->first_node("source")->first_attribute("id")->value());
            QString targetFieldStr(doc.first_node("coupling")->first_node("general")->first_node("modules")->first_node("target")->first_attribute("id")->value());

            if((sourceFieldStr == sourceField->fieldId()) && (targetFieldStr == targetField->fieldId()))
            {
                //check whether coupling is available for values of source and target fields such as analysis type
                for (rapidxml::xml_node<> *weakform = doc.first_node("coupling")->first_node("volume")->first_node("weakforms")->first_node("weakform");
                     weakform; weakform = weakform->next_sibling())
                {

                    if ((weakform->first_attribute("sourceanalysis")->value() == Hermes::analysis_type_tostring(sourceField->analysisType())) &&
                        (weakform->first_attribute("targetanalysis")->value() == Hermes::analysis_type_tostring(targetField->analysisType())))
                    {
                        return true;
                    }
                }
            }
        }
    }
    closedir(dp);

    return false;
}

Coupling::Coupling(CoordinateType coordinateType, CouplingType couplingType, AnalysisType sourceFieldAnalysis, AnalysisType targetFieldAnalysis)
{
    m_coordinateType = coordinateType;
    m_couplingType = couplingType;
    m_sourceFieldAnalysis = sourceFieldAnalysis;
    m_targetFieldAnalysis = targetFieldAnalysis;

    clear();
}

Coupling::~Coupling()
{
    clear();
}

void Coupling::clear()
{
}

void Coupling::read(std::string filename)
{
    std::cout << "reading coupling: " << filename << std::endl << std::flush;

    clear();

    if (ifstream(filename.c_str()))
    {
        // save current locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        rapidxml::file<> file_data(filename.c_str());

        // parse document
        rapidxml::xml_document<> doc;
        doc.parse<0>(file_data.data());

        // problem
        rapidxml::xml_node<> *general = doc.first_node("coupling")->first_node("general");
        id = general->first_attribute("id")->value();
        name = QObject::tr(general->first_attribute("name")->value()).toStdString(); // FIXME - Qt dependence
        description = general->first_node("description")->value();


//        rapidxml::xml_node<> *source = general->first_node("modules")->first_node("source");
//        this->sourceField = Util::scene()->fieldInfo(source->first_attribute("id")->value());
//        rapidxml::xml_node<> *object = general->first_node("modules")->first_node("target");
//        this->targetField = Util::scene()->fieldInfo(object->first_attribute("id")->value());



        // volumetric weakforms
//        Hermes::vector<Hermes::Module::MaterialTypeVariable> material_type_variables_tmp;
//        for (rapidxml::xml_node<> *quantity = doc.first_node("module")->first_node("volume")->first_node("quantity");
//             quantity; quantity = quantity->next_sibling())
//            if (std::string(quantity->name()) == "quantity")
//                material_type_variables_tmp.push_back(Hermes::Module::MaterialTypeVariable(quantity));

        //TODO temporary
        //m_analysisType = AnalysisType_SteadyState;

        for (rapidxml::xml_node<> *weakform = doc.first_node("coupling")->first_node("volume")->first_node("weakforms")->first_node("weakform");
             weakform; weakform = weakform->next_sibling())
        {

            if ((weakform->first_attribute("couplingtype")->value() == Hermes::coupling_type_tostring(m_couplingType)) &&
                (weakform->first_attribute("sourceanalysis")->value() == Hermes::analysis_type_tostring(m_sourceFieldAnalysis)) &&
                (weakform->first_attribute("targetanalysis")->value() == Hermes::analysis_type_tostring(m_targetFieldAnalysis)))
            {

//            if (weakform->first_attribute("analysistype")->value() == Hermes::analysis_type_tostring(m_analysisType))
//            {
                // quantities
//                for (rapidxml::xml_node<> *quantity = weakform->first_node("quantity");
//                     quantity; quantity = quantity->next_sibling())
//                {
//                    if (std::string(quantity->name()) == "quantity")
//                        for (Hermes::vector<Hermes::Module::MaterialTypeVariable>::iterator it = material_type_variables_tmp.begin();
//                             it < material_type_variables_tmp.end(); ++it )
//                        {
//                            Hermes::Module::MaterialTypeVariable old = (Hermes::Module::MaterialTypeVariable) *it;
//                            if (old.id == quantity->first_attribute("id")->value())
//                            {
//                                Hermes::Module::MaterialTypeVariable *var = new Hermes::Module::MaterialTypeVariable(
//                                            old.id, old.shortname, old.default_value);
//                                material_type_variables.push_back(var);
//                            }
//                        }
//                }
//                material_type_variables_tmp.clear();

                // weakforms
                for (rapidxml::xml_node<> *matrix = weakform->first_node("matrix");
                     matrix; matrix = matrix->next_sibling())
                {
                    if (std::string(matrix->name()) == "matrix")
                        weakform_matrix_volume.push_back(new ParserFormExpression(matrix, m_coordinateType));
                }

                for (rapidxml::xml_node<> *vector = weakform->first_node("vector");
                     vector; vector = vector->next_sibling())
                    if (std::string(vector->name()) == "vector")
                        weakform_vector_volume.push_back(new ParserFormExpression(vector, m_coordinateType));
            }
        }

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }
}



// ****************************************************************************************************

Coupling *couplingFactory(FieldInfo* sourceField, FieldInfo* targetField, CouplingType couplingType,
                                           std::string filename_custom)
{
    // std::cout << filename_custom << std::endl;

    CoordinateType coordinateType = sourceField->coordinateType();
    Coupling *coupling = new Coupling(coordinateType, couplingType, sourceField->analysisType(), targetField->analysisType());

    // open default module
    std::string filename_default = (datadir() + COUPLINGROOT + "/" + sourceField->fieldId() + "-" +
                                    targetField->fieldId() + ".xml").toStdString();
    ifstream ifile_default(filename_default.c_str());
    if (ifile_default)
    {
        coupling->read(filename_default);
        return coupling;
    }

    std::cout << "Coupling doesn't exists." << std::endl;
    return NULL;
}

void CouplingInfo::synchronizeCouplings(QMap<QString, FieldInfo *>& fieldInfos, QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >& couplingInfos)
{
    // add missing
    foreach (FieldInfo* sourceField, fieldInfos)
    {
        foreach (FieldInfo* targetField, fieldInfos)
        {
            if(sourceField == targetField)
                continue;
            QPair<FieldInfo*, FieldInfo*> fieldInfosPair(sourceField, targetField);
            if (isCouplingAvailable(sourceField, targetField)){
                if (!couplingInfos.contains(fieldInfosPair))
                {
                    couplingInfos[fieldInfosPair] = new CouplingInfo(sourceField, targetField);
                }
            }
        }
    }

    // remove extra
    foreach (CouplingInfo* couplingInfo, couplingInfos)
    {
        if(! (fieldInfos.contains(couplingInfo->sourceField()->fieldId()) &&
              fieldInfos.contains(couplingInfo->targetField()->fieldId()) &&
              isCouplingAvailable(couplingInfo->sourceField(), couplingInfo->targetField())))
        {
            couplingInfos.remove(QPair<FieldInfo*, FieldInfo*>(couplingInfo->sourceField(), couplingInfo->targetField()));
        }
    }

}

