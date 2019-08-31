#include "stdafx.h"
#include "modelcheck.h"
#include "Document.h"
#include <PostGL/GLModel.h>
#include <GeomLib/GObject.h>

void check_A001(CDocument* doc, std::vector<std::string>& errorList)
{
	// are there any objects?
	GModel& mdl = *doc->GetGModel();
	if (mdl.Objects() == 0)
	{
		errorList.push_back("A001 : This model does not contain any geometry.");
	}
}

void check_A002(CDocument* doc, std::vector<std::string>& errorList)
{
	GModel& mdl = *doc->GetGModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		if (po->GetFEMesh() == nullptr)
		{
			errorList.push_back("A002 : The object \"" + po->GetName() + "\" is not meshed.");
		}
	}
}

void check_A003(CDocument* doc, std::vector<std::string>& errorList)
{
	FEModel& fem = *doc->GetFEModel();
	if (fem.Materials() == 0)
	{
		errorList.push_back("A003 : There are no materials defined in this model.");
	}
}

void check_A004(CDocument* doc, std::vector<std::string>& errorList)
{
	GModel& mdl = *doc->GetGModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		for (int j = 0; j < po->Parts(); ++j)
		{
			GPart* pj = po->Part(j);
			if (pj->GetMaterialID() == -1)
			{
				errorList.push_back("A004 : Part \"" + pj->GetName() + "\" of object \"" + po->GetName() + "\" does not have a material assigned.");
			}
		}
	}
}

void check_A005(CDocument* doc, std::vector<std::string>& errorList)
{
	FEModel& fem = *doc->GetFEModel();
	if (fem.Steps() <= 1)
	{
		errorList.push_back("A005 : There are no analysis steps defined.");
	}
}

void check_B001(CDocument* doc, std::vector<std::string>& errorList)
{
	GModel& mdl = *doc->GetGModel();
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* mat = fem.GetMaterial(i);

		// see if this material is used
		list<GPart*> partList = mdl.FindPartsFromMaterial(mat->GetID());
		if (partList.empty())
		{
			errorList.push_back("B001 : Material \"" + mat->GetName() + "\" is not used by this model.");
		}
	}
}

void checkAll(CDocument* doc, std::vector<std::string>& errorList)
{
	// are there any objects?
	check_A001(doc, errorList);

	// are all geometries meshed?
	check_A002(doc, errorList);

	// are there any materials
	check_A003(doc, errorList);
		
	// is a material assigned to all parts ?
	check_A004(doc, errorList);

	// Are there any analysis steps? 
	check_A005(doc, errorList);

	// check for unused materials
	check_B001(doc, errorList);
}
