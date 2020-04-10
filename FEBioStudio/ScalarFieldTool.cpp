#include "stdafx.h"
#include "ScalarFieldTool.h"
#include "Document.h"
#include <MeshTools/LaplaceSolver.h>
#include <GeomLib/GObject.h>

CScalarFieldTool::CScalarFieldTool() : CBasicTool("Scalar Field", HAS_APPLY_BUTTON)
{
	m_ngen[0] = 0;
	m_ngen[1] = 0;

	m_weight[0] = 0.0;
	m_weight[1] = 1.0;

	addStringProperty(&m_name, "Name");
	addIntProperty(&m_ngen[0], "Nodeset 1");
	addDoubleProperty(&m_weight[0], "Value 1");
	addIntProperty(&m_ngen[1], "Nodeset 2");
	addDoubleProperty(&m_weight[1], "Value 2");

	SetApplyButtonText("Create");
}

bool CScalarFieldTool::OnApply()
{
	// get the document
	CDocument* pdoc = GetDocument();

	// get the currently selected object
	GObject* po = pdoc->GetActiveObject();
	if (po == 0)
	{
		SetErrorString("You must first select an object.");
		return false;
	}

	// make sure there is a mesh
	FEMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		SetErrorString("The object needs to be meshed before you can apply this tool.");
		return false;
	}

	//get the model and nodeset
	FEModel* ps = pdoc->GetFEModel();
	GModel& model = ps->GetModel();

	int NN = pm->Nodes();
	vector<int> bn(NN, 0);
	vector<double> val(NN, 0.0);

	for (int i = 0; i<2; i++)
	{
		//get the node set of each name selection
		FENodeSet* pg = po->GetFENodeSet(m_ngen[i]);
		if (pg == nullptr) return false;

		for (FEItemListBuilder::Iterator it = pg->begin(); it != pg->end(); it++)
		{
			int nid = *it;
			bn[nid] = 1;
			val[nid] = m_weight[i];
		}
	}

	// solve Laplace equation
	LaplaceSolver L;
	L.Solve(pm, val, bn);

	// create node data
	FENodeData* pdata = pm->AddNodeDataField(m_name.toStdString());
	for (int i = 0; i<NN; i++) pdata->set(i, val[i]);

	return true;
}
