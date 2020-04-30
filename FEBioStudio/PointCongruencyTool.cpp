#include "stdafx.h"
#include "PointCongruencyTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <MeshTools/SphereFit.h>
#include "PropertyListView.h"
#include <PostLib/FEPointCongruency.h>

// constructor
CPointCongruencyTool::CPointCongruencyTool(CMainWindow* wnd) : CBasicTool(wnd, "Pt. Congruency", CBasicTool::HAS_APPLY_BUTTON)
{
	addIntProperty(&m_node, "Node");
	addDoubleProperty(&m_smooth, "Smoothness");

	addIntProperty(&m_face, "Face")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_H1, "H1")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_G1, "G1")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_H2, "H2")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_G2, "G2")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_alpha, "alpha")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_delta, "delta")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_Kemin, "Ke_min")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_Kemax, "Ke_max")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_cong, "Congruency")->setFlags(CProperty::Visible);
}

bool CPointCongruencyTool::OnApply()
{
	// get the nodal coordinates (surface only)
	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;

	int node = m_node - 1;
	if ((node >= 0)&&(node<mesh->Nodes()))
	{
		Post::FEPointCongruency tool;
		tool.SetLevels(1);
		Post::FEPointCongruency::CONGRUENCY_DATA d = tool.Congruency(mesh, node);

		m_face = d.nface + 1;
		m_H1 = d.H1;
		m_G1 = d.G1;
		m_H2 = d.H2;
		m_G2 = d.G2;

		m_alpha = 180 * d.a / PI;
		m_delta = d.D;

		m_Kemin = d.Kemin;
		m_Kemax = d.Kemax;
		m_cong = d.Ke;
        return true;
	}
    return false;
}
