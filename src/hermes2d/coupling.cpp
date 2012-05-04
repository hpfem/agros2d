#include "coupling.h"
#include "scene.h"
#include "util.h"
#include "module.h"
#include "weakform_parser.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

#include <dirent.h>

#include "../../resources_source/classes/coupling_xml.h"

CouplingInfo::CouplingInfo(FieldInfo *sourceField, FieldInfo *targetField) :
    m_sourceField(sourceField), m_targetField(targetField)
{
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

LinearityType CouplingInfo::linearityType()
{
    assert(sourceField()->linearityType() == targetField()->linearityType());
    return sourceField()->linearityType();
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
            std::auto_ptr<XMLCoupling::coupling> couplings_xsd = XMLCoupling::coupling_((datadir().toStdString() + COUPLINGROOT.toStdString() + "/" + filename).c_str());
            XMLCoupling::coupling *coup = couplings_xsd.get();

            // module name
            QString sourceFieldStr(QString::fromStdString(coup->general().modules().source().id()));
            QString targetFieldStr(QString::fromStdString(coup->general().modules().target().id()));

            if ((sourceFieldStr == sourceField->fieldId()) && (targetFieldStr == targetField->fieldId()))
            {
                //check whether coupling is available for values of source and target fields such as analysis type
                for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
                {
                    XMLCoupling::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

                    if ((wf.sourceanalysis() == analysisTypeToStringKey(sourceField->analysisType()).toStdString()) &&
                        (wf.targetanalysis() == analysisTypeToStringKey(targetField->analysisType()).toStdString()))
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

mu::Parser* Coupling::getParser()
{
    mu::Parser *parser = new mu::Parser();

    // pi
    parser->DefineConst("PI", M_PI);

    // frequency
    parser->DefineConst("f", Util::problem()->config()->frequency());

    // timestep
    parser->DefineConst("dt", Util::problem()->config()->timeStep().number());

    for (std::map<std::string, double>::iterator it = constants.begin(); it != constants.end(); ++it)
        parser->DefineConst(it->first, it->second);

    parser->EnableOptimizer(true);

    return parser;
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

        std::auto_ptr<XMLCoupling::coupling> couplings_xsd = XMLCoupling::coupling_(filename.c_str());
        XMLCoupling::coupling *coup = couplings_xsd.get();

        // problem
        id = coup->general().id();
        name = coup->general().name();
        description = coup->general().description();


//        rapidxml::xml_node<> *source = general->first_node("modules")->first_node("source");
//        this->sourceField = Util::problem()->fieldInfo(source->first_attribute("id")->value());
//        rapidxml::xml_node<> *object = general->first_node("modules")->first_node("target");
//        this->targetField = Util::problem()->fieldInfo(object->first_attribute("id")->value());



        // volumetric weakforms
//        Hermes::vector<Module::MaterialTypeVariable> material_type_variables_tmp;
//        for (rapidxml::xml_node<> *quantity = doc.first_node("module")->first_node("volume")->first_node("quantity");
//             quantity; quantity = quantity->next_sibling())
//            if (std::string(quantity->name()) == "quantity")
//                material_type_variables_tmp.push_back(Module::MaterialTypeVariable(quantity));

        //TODO temporary
        //m_analysisType = AnalysisType_SteadyState;

        // constants
        for (int i = 0; i < coup->constants().constant().size(); i++)
        {
            XMLCoupling::constant cnst = coup->constants().constant().at(i);
            constants[cnst.id()] = cnst.value();
        }

        for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
        {
            XMLCoupling::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

            if ((wf.couplingtype() == couplingTypeToStringKey(m_couplingType).toStdString()) &&
                (wf.sourceanalysis() == analysisTypeToStringKey(m_sourceFieldAnalysis).toStdString()) &&
                (wf.targetanalysis() == analysisTypeToStringKey(m_targetFieldAnalysis).toStdString()))
            {

//            if (weakform->first_attribute("analysistype")->value() == Hermes::analysis_type_tostring(m_analysisType))
//            {
                // quantities
//                for (rapidxml::xml_node<> *quantity = weakform->first_node("quantity");
//                     quantity; quantity = quantity->next_sibling())
//                {
//                    if (std::string(quantity->name()) == "quantity")
//                        for (Hermes::vector<Module::MaterialTypeVariable>::iterator it = material_type_variables_tmp.begin();
//                             it < material_type_variables_tmp.end(); ++it )
//                        {
//                            Module::MaterialTypeVariable old = (Module::MaterialTypeVariable) *it;
//                            if (old.id == quantity->first_attribute("id")->value())
//                            {
//                                Module::MaterialTypeVariable *var = new Module::MaterialTypeVariable(
//                                            old.id, old.shortname, old.default_value);
//                                material_type_variables.push_back(var);
//                            }
//                        }
//                }
//                material_type_variables_tmp.clear();

                // weakforms
                /*
                for (rapidxml::xml_node<> *matrix = weakform->first_node("matrix_form");
                     matrix; matrix = matrix->next_sibling())
                {
                    if (std::string(matrix->name()) == "matrix")
                        weakform_matrix_volume.push_back(new ParserFormExpression(matrix, m_coordinateType));
                }

                for (rapidxml::xml_node<> *vector = weakform->first_node("vector_form");
                     vector; vector = vector->next_sibling())
                    if (std::string(vector->name()) == "vector_form")
                        weakform_vector_volume.push_back(new ParserFormExpression(vector, m_coordinateType));
                */
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

    CoordinateType coordinateType = Util::problem()->config()->coordinateType();
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

void CouplingInfo::synchronizeCouplings(const QMap<QString, FieldInfo *>& fieldInfos, QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >& couplingInfos)
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

