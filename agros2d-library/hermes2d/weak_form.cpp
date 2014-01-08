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

