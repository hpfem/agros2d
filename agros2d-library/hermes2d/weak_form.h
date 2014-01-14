#ifndef WEAK_FORM_H
#define WEAK_FORM_H

#include "form_info.h"

class BDF2Table;
class Block;
class Field;
class Marker;
class Boundary;
class Material;
class SceneMaterial;
class CouplingInfo;
class FieldInfo;

namespace XMLModule
{
class field;
class quantity;
class boundary;
class surface;
class force;
class localvariable;
class gui;
class space;
class calculator;
class linearity_option;
}

struct PositionInfo
{
    PositionInfo();
    int formsOffset;
    int quantAndSpecOffset;
    int numQuantAndSpecFun;
    int previousSolutionsOffset;
    int numPreviousSolutions;
    bool isSource;
};

struct Offset
{
    int forms;
    int quant;
    int prevSol;

    int sourceForms;
    int sourceQuant;
    int sourcePrevSol;
};



struct ProblemID
{
    ProblemID() :
        sourceFieldId(""), targetFieldId(""),
        analysisTypeSource(AnalysisType_Undefined), analysisTypeTarget(AnalysisType_Undefined),
        coordinateType(CoordinateType_Undefined), linearityType(LinearityType_Undefined),
        couplingType(CouplingType_Undefined) {}

    // TODO: set/get methods
    QString sourceFieldId;
    QString targetFieldId;
    AnalysisType analysisTypeSource;
    AnalysisType analysisTypeTarget;
    CoordinateType coordinateType;
    LinearityType linearityType;
    CouplingType couplingType;

    QString toString()
    {
        // TODO: implement toString() method
        return "TODO";
    }
};


const int INVALID_POSITION_INFO_VALUE = -223344;
// maximal number of existing modules
const int MAX_FIELDS = 10;

void findVolumeLinearityOption(XMLModule::linearity_option& option, XMLModule::field *module, AnalysisType analysisType, LinearityType linearityType);

QList<FormInfo> generateSeparated(QList<FormInfo> elements, QList<FormInfo> templates, QList<FormInfo> templatesForResidual = QList<FormInfo>());

template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTemplates(SectionWithTemplates *section);

template <typename Scalar>
class AGROS_LIBRARY_API WeakFormAgros : public Hermes::Hermes2D::WeakForm<Scalar>
{
public:
    WeakFormAgros(Block* block);
    ~WeakFormAgros();

    void registerForms();
    void updateExtField();
    inline BDF2Table* bdf2Table() { return m_bdf2Table; }

    // prepares individual forms for given analysis and linearity type, as specified in Elements, using information form Templates
    static QList<FormInfo> wfMatrixVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType);
    static QList<FormInfo> wfVectorVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType);

    inline const PositionInfo* positionInfo(int index) const {return &m_positionInfos[index]; }
    void outputPositionInfos();

    Offset offsetInfo(const FieldInfo *sourceFieldInfo, const FieldInfo *targetFieldInfo) const;
    Offset offsetInfo(const Marker *sourceMarker, const Marker *targetMarker) const;

private:
    Hermes::Hermes2D::Form<Scalar> *factoryForm(WeakFormKind type, const ProblemID problemId,
                                                const QString &area, FormInfo *form,
                                                Marker* markerSource, Material *markerTarget);

    // materialTarget has to be specified for coupling forms. couplingInfo only for weak couplings
    void registerForm(WeakFormKind type, Field *field, QString area, FormInfo form, Marker *marker);

    // offsetCouplingExt defines position in Ext field where coupling solutions start
    void registerFormCoupling(WeakFormKind type, QString area, FormInfo form, SceneMaterial *materialSource,
                              SceneMaterial *materialTarget, CouplingInfo *couplingInfo);
    void addForm(WeakFormKind type, Hermes::Hermes2D::Form<Scalar>* form);

    virtual Hermes::Hermes2D::WeakForm<Scalar>* clone() const { return new WeakFormAgros<Scalar>(m_block); }

    Block* m_block;

    BDF2Table* m_bdf2Table;

    PositionInfo m_positionInfos[MAX_FIELDS];

    int m_numberOfForms;

    Hermes::vector<Hermes::Hermes2D::UExtFunctionSharedPtr<Scalar> > quantitiesAndSpecialFunctions(const FieldInfo* fieldInfo) const;
    Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > previousTimeLevelsSolutions(const FieldInfo* fieldInfo) const;
    Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > sourceCouplingSolutions(const FieldInfo* fieldInfo) const;
};

#endif // WEAK_FORM_H
