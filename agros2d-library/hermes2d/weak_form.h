#ifndef WEAK_FORM_H
#define WEAK_FORM_H

#include "form_info.h"

class BDF2Table;
class Block;
class Field;
class Marker;
class Boundary;
class SceneMaterial;
class CouplingInfo;


namespace XMLModule
{
class module;
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

void findVolumeLinearityOption(XMLModule::linearity_option& option, XMLModule::module *module, AnalysisType analysisType, LinearityType linearityType);

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
    static QList<FormInfo> wfMatrixVolumeSeparated(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);
    static QList<FormInfo> wfVectorVolumeSeparated(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);

private:
    // materialTarget has to be specified for coupling forms. couplingInfo only for weak couplings
    void registerForm(WeakFormKind type, Field *field, QString area, FormInfo form, int offsetI, int offsetJ, Marker *marker);

    // offsetCouplingExt defines position in Ext field where coupling solutions start
    void registerFormCoupling(WeakFormKind type, QString area, FormInfo form, int offsetI, int offsetJ, SceneMaterial *materialSource,
                              SceneMaterial *materialTarget, CouplingInfo *couplingInfo);
    void addForm(WeakFormKind type, Hermes::Hermes2D::Form<Scalar>* form);

    virtual Hermes::Hermes2D::WeakForm<Scalar>* clone() const { return new WeakFormAgros<Scalar>(m_block); }

    Block* m_block;

    BDF2Table* m_bdf2Table;

    // index in EXT field, where start solutions from previous time levels. ( == number of Value ext functions)
    int m_offsetPreviousTimeExt;

    // index in EXT field, where start solutions from weakly coupled fields ( == number of Value ext functions + time components)
    // we have to pass pointer to individual forms, since it may change during the calculation (the order of the BDF method may vary)
    int m_offsetCouplingExt;

    int m_numberOfForms;
};

#endif // WEAK_FORM_H
