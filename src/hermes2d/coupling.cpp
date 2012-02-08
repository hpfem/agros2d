#include "coupling.h"
#include "scene.h"
#include "util.h"
#include "module.h"
#include "weakform_parser.h"

//std::map<std::string, std::string> availableModules()
//{
//    static std::map<std::string, std::string> modules;

//    // read modules
//    if (modules.size() == 0)
//    {
//        DIR *dp;
//        if ((dp = opendir((datadir()+ MODULEROOT).toStdString().c_str())) == NULL)
//            error("Modules dir '%s' doesn't exists", (datadir() + MODULEROOT).toStdString().c_str());

//        struct dirent *dirp;
//        while ((dirp = readdir(dp)) != NULL)
//        {
//            std::string filename = dirp->d_name;

//            // skip current and parent dir
//            if (filename == "." || filename == "..")
//                continue;

//            if (filename.substr(filename.size() - 4, filename.size() - 1) == ".xml")
//            {
//                // read name
//                rapidxml::file<> file_data((datadir().toStdString() + MODULEROOT.toStdString() + "/" + filename).c_str());

//                // parse xml
//                rapidxml::xml_document<> doc;
//                doc.parse<0>(file_data.data());

//                // module name
//                modules[filename.substr(0, filename.size() - 4)] =
//                        QObject::tr(doc.first_node("module")->first_node("general")->first_attribute("name")->value()).toStdString();
//            }
//        }
//        closedir(dp);
//    }

//    // custom module
//    // modules["custom"] = "Custom field";

//    return modules;
//}

Coupling::Coupling(CoordinateType coordinateType, CouplingType couplingType)
{
    m_coordinateType = coordinateType;
    m_couplingType = couplingType;

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


        rapidxml::xml_node<> *source = general->first_node("modules")->first_node("source");
        this->sourceField = Util::scene()->fieldInfo(source->first_attribute("id")->value());
        rapidxml::xml_node<> *object = general->first_node("modules")->first_node("target");
        this->targetField = Util::scene()->fieldInfo(object->first_attribute("id")->value());



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
//            }
        }

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }
}



// ****************************************************************************************************

Coupling *couplingFactory(std::string id, CoordinateType coordinate_type, CouplingType coupling_type,
                                           std::string filename_custom)
{
    // std::cout << filename_custom << std::endl;

    Coupling *coupling = new Coupling(coordinate_type, coupling_type);

    // try to open custom module
    if (id == "custom")
    {
        ifstream ifile_custom(filename_custom.c_str());
        if (!ifile_custom)
            coupling->read(datadir().toStdString() + "/resources/custom.xml");
        else
            coupling->read(filename_custom);

        return coupling;
    }

    // open default module
    std::string filename_default = (datadir() + COUPLINGROOT + "/" + QString::fromStdString(id) + ".xml").toStdString();
    ifstream ifile_default(filename_default.c_str());
    if (ifile_default)
    {
        coupling->read(filename_default);
        return coupling;
    }

    std::cout << "Coupling doesn't exists." << std::endl;
    return NULL;
}
