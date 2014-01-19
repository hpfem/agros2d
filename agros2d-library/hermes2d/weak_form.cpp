#include "util/global.h"
#include "problem.h"
#include "field.h"
#include "block.h"
#include "weak_form.h"

void checkDuplicities(QList<FormInfo> list)
{
    for(int i = 0; i < list.size(); i++)
    {
        for(int j = 0; j < list.size(); j++)
        {
            if(i != j)
            {
                if(list[i].id == list[j].id)
                    throw AgrosGeneratorException("Duplicities in forms");
            }
        }
    }
}

FormInfo findFormInfo(QList<FormInfo> list, QString id)
{
    foreach(FormInfo form, list)
    {
        if(form.id == id)
            return form;
    }
    throw AgrosGeneratorException(QString("Form %1 not found").arg(id));
}

// todo: implement properly. What if uval is part of some identifier?
void replaceForVariant(QString& str, WeakFormVariant variant)
{
    if(variant == WeakFormVariant_Normal)
    {
    }
    else if(variant == WeakFormVariant_Residual)
    {
        str.replace("uval", "upval");
        str.replace("udx", "updx");
        str.replace("udy", "updy");
        str.replace("udr", "updr");
        str.replace("udz", "updz");
    }
    else if(variant == WeakFormVariant_TimeResidual)
    {
        str.replace("uval", "uptval");
        str.replace("udx", "uptdx");
        str.replace("udy", "uptdy");
        str.replace("udr", "uptdr");
        str.replace("udz", "uptdz");
    }
    else
        throw AgrosGeneratorException("Unknown form variant");
}

QList<FormInfo> generateSeparated(QList<FormInfo> elements, QList<FormInfo> templates, QList<FormInfo> templatesForResidual)
{
    checkDuplicities(templates);
    checkDuplicities(elements);
    QList<FormInfo> listResult;
    foreach(FormInfo formElement, elements)
    {
        FormInfo formTemplate;
        try
        {
            formTemplate = findFormInfo(templates, formElement.id);
        }
        catch(AgrosGeneratorException &err)
        {
            if(templatesForResidual.empty())
                throw;
            else
                formTemplate = findFormInfo(templatesForResidual, formElement.id);
        }

        FormInfo formResult(formTemplate.id, formTemplate.i, formTemplate.j, formTemplate.sym_planar, formTemplate.sym_axi);
        formResult.condition = formTemplate.condition;

        if (formElement.coefficient != 1.)
        {
            formResult.expr_axi = QString("%1*(%2)").arg(formElement.coefficient).arg(formTemplate.expr_axi);
            formResult.expr_planar = QString("%1*(%2)").arg(formElement.coefficient).arg(formTemplate.expr_planar);
        }
        else
        {
            formResult.expr_axi = formTemplate.expr_axi;
            formResult.expr_planar = formTemplate.expr_planar;
        }
        replaceForVariant(formResult.expr_axi, formElement.variant);
        replaceForVariant(formResult.expr_planar, formElement.variant);

        listResult.push_back(formResult);
    }

    return listResult;
}

PositionInfo::PositionInfo() :
    formsOffset(INVALID_POSITION_INFO_VALUE),
    quantAndSpecOffset(INVALID_POSITION_INFO_VALUE),
    numQuantAndSpecFun(0),
    previousSolutionsOffset(INVALID_POSITION_INFO_VALUE),
    numPreviousSolutions(0),
    isSource(false)
{

}

template <typename Scalar>
Offset WeakFormAgros<Scalar>::offsetInfo(const Marker *sourceMarker, const Marker *targetMarker) const
{
    assert(targetMarker);

    if(sourceMarker)
        return offsetInfo(sourceMarker->fieldInfo(), targetMarker->fieldInfo());
    else
        return offsetInfo(nullptr, targetMarker->fieldInfo());
}

template <typename Scalar>
Offset WeakFormAgros<Scalar>::offsetInfo(const FieldInfo *sourceFieldInfo, const FieldInfo *targetFieldInfo) const
{
    Offset offset;
    assert(targetFieldInfo != nullptr);

    const int fieldID = targetFieldInfo->numberId();
    offset.forms = positionInfo(fieldID)->formsOffset;
    offset.prevSol = positionInfo(fieldID)->previousSolutionsOffset;
    offset.quant = positionInfo(fieldID)->quantAndSpecOffset;

    if(sourceFieldInfo)
    {
        const int fieldIDSource = sourceFieldInfo->numberId();
        offset.sourceForms = positionInfo(fieldIDSource)->formsOffset;
        offset.sourcePrevSol = positionInfo(fieldIDSource)->previousSolutionsOffset;
        offset.sourceQuant = positionInfo(fieldIDSource)->quantAndSpecOffset;
    }

    return offset;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::outputPositionInfos()
{
    qDebug() << "*************Block************************";
    qDebug() << "weak coupling sources:";
    foreach(FieldInfo* fieldInfo, m_block->sourceFieldInfosCoupling())
    {
        int id = fieldInfo->numberId();
        qDebug() << "fieldInfo " << fieldInfo->fieldId() << ", " << id;
        qDebug() << "field offset " << m_positionInfos[id].formsOffset << ", is source " << m_positionInfos[id].isSource;
        qDebug() << "quantities num " << m_positionInfos[id].numQuantAndSpecFun << ", offset " << m_positionInfos[id].quantAndSpecOffset;
        qDebug() << "previous solutions num " << m_positionInfos[id].numPreviousSolutions << ", offset " << m_positionInfos[id].previousSolutionsOffset;
    }
    qDebug() << "block members:";
    foreach(FieldInfo* fieldInfo, m_block->fieldInfos())
    {
        int id = fieldInfo->numberId();
        qDebug() << "fieldInfo " << fieldInfo->fieldId() << ", " << id;
        qDebug() << "field offset " << m_positionInfos[id].formsOffset << ", is source " << m_positionInfos[id].isSource;
        qDebug() << "quantities num " << m_positionInfos[id].numQuantAndSpecFun << ", offset " << m_positionInfos[id].quantAndSpecOffset;
        qDebug() << "previous solutions num " << m_positionInfos[id].numPreviousSolutions << ", offset " << m_positionInfos[id].previousSolutionsOffset;
    }
    qDebug() << "**************End*************************";
}

template class WeakFormAgros<double>;
