#include "stdafx.h"
#include "ConchoidFitTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "Document.h"
#include <MeshTools/ConchoidFit.h>
#include <GeomLib/GObject.h>

// constructor
CConchoidFitTool::CConchoidFitTool(CMainWindow* wnd) : CBasicTool(wnd, "Conchoid Fit", HAS_APPLY_BUTTON)
{
	m_bsel = false;
	m_x = m_y = m_z = 0.0;
	m_A = m_B = 0.0;
	m_obj = 0.0;

	addBoolProperty(&m_bsel, "selection only");
	addDoubleProperty(&m_x, "x")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_y, "y")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_z, "z")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_A, "A")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_B, "B")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_obj, "obj")->setFlags(CProperty::Visible);

	SetApplyButtonText("Fit");
}

bool CConchoidFitTool::OnApply()
{
	// get the nodal coordinates (surface only)
	CDocument* doc = GetDocument();
	if (doc && doc->IsValid())
	{
		GObject* po = doc->GetActiveObject();
		if ((po == 0) || (po->GetFEMesh() == 0)) 
		{
			SetErrorString("You must select an object with a mesh.");
			return false;
		}

		FEMesh& mesh = *po->GetFEMesh();

		int N = mesh.Nodes();
		int F = mesh.Faces();
		for (int i = 0; i<N; ++i) mesh.Node(i).m_ntag = 0;
		for (int i = 0; i<F; ++i)
		{
			FEFace& f = mesh.Face(i);
			if ((m_bsel == false) || (f.IsSelected()))
			{
				int nf = f.Nodes();
				for (int j = 0; j<nf; ++j) mesh.Node(f.n[j]).m_ntag = 1;
			}
		}

		vector<vec3d> y;
		for (int i = 0; i<N; ++i)
		{
			if (mesh.Node(i).m_ntag == 1) y.push_back(mesh.Node(i).r);
		}

		// find the best fit sphere
		ConchoidFit fit;
		fit.Fit(y);
		vec3d sc = fit.m_rc;
		double A = fit.m_a;
		double B = fit.m_b;

		// calculate the objective function
		double objs = fit.ObjFunc(0);

		// update GUI
		m_x = sc.x;
		m_y = sc.y;
		m_z = sc.z;
		m_A = A;
		m_B = B;
		m_obj = objs;
	}

	return true;
}
