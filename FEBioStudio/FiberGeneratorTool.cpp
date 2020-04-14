#include "stdafx.h"
#include "FiberGeneratorTool.h"
#include "Document.h"
#include <MeshTools/GradientMap.h>
#include <GeomLib/GObject.h>

CFiberGeneratorTool::CFiberGeneratorTool() : CBasicTool("Fiber generator", HAS_APPLY_BUTTON)
{
	m_niter = 0;
	m_ndata = 0;

	addIntProperty(&m_ndata, "Data field");
	addIntProperty(&m_niter, "smoothing iterations");
}

bool CFiberGeneratorTool::OnApply()
{
	CDocument* pdoc = GetDocument();
	GObject* po = pdoc->GetActiveObject();
	if (po == 0) 
	{
		SetErrorString("You must select an object first.");
		return false;
	}

	FEMesh* pm = po->GetFEMesh();
	if (pm == 0) 
	{
		SetErrorString("The selected object does not have a mesh.");
		return false;
	}

	int N = pm->MeshDataFields();
	if ((m_ndata >= 0) && (m_ndata < N))
	{
		// get node data field
		FENodeData& D = dynamic_cast<FENodeData&>(*pm->GetMeshDataField(m_ndata));

		// calculate gradient and assign to element fiber
		vector<vec3d> grad;
		GradientMap G;
		G.Apply(D, grad, m_niter);

		// assign to element fibers
		int NE = pm->Elements();
		for (int i = 0; i<NE; ++i)
		{
			FEElement& el = pm->Element(i);
			el.m_fiber = grad[i];
		}
	}

	return true;
}
