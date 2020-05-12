#include "stdafx.h"
#include "MeshInfoPanel.h"
#include <QLabel>
#include <QFormLayout>
#include <MeshLib/FEMesh.h>
#include <MeshLib/FESurfaceMesh.h>
#include <GeomLib/GObject.h>
#include "units.h"
#include <FSCore/paramunit.h>

CMeshInfoPanel::CMeshInfoPanel(QWidget* parent) : QWidget(parent)
{
	m_nodes = new QLabel;
	m_faces = new QLabel;
	m_elems = new QLabel;

	m_Dx = new QLabel;
	m_Dy = new QLabel;
	m_Dz = new QLabel;

	QFormLayout* form = new QFormLayout;
	form->setLabelAlignment(Qt::AlignRight);

	form->addRow("<b>Dimensions:</b>", (QWidget*)0);
	form->addRow("X:", m_Dx);
	form->addRow("Y:", m_Dy);
	form->addRow("Z:", m_Dz);

	form->addRow(new QLabel("<b>Mesh info:</b>"));
	form->addRow("Nodes:", m_nodes);
	form->addRow("Faces:", m_faces);
	form->addRow("Elements:", m_elems);

	setLayout(form);

	setMeshInfo(0, 0, 0);
}

void CMeshInfoPanel::setMeshInfo(int nodes, int faces, int elems)
{
	m_nodes->setNum(nodes);
	m_faces->setNum(faces);
	m_elems->setNum(elems);
}

void CMeshInfoPanel::setDimensions(double dx, double dy, double dz)
{
	QString unit = Units::GetUnitString(UNIT_LENGTH);
	m_Dx->setText(QString("%1 %2").arg(dx).arg(unit));
	m_Dy->setText(QString("%1 %2").arg(dy).arg(unit));
	m_Dz->setText(QString("%1 %2").arg(dz).arg(unit));
}

void CMeshInfoPanel::setInfo(GObject* po)
{
	if (po)
	{
		BOX box = po->GetLocalBox();
		setDimensions(box.Width(), box.Height(), box.Depth());

		FEMesh* pm = po->GetFEMesh();
		if (pm) setMeshInfo(pm->Nodes(), pm->Faces(), pm->Elements());
		else setMeshInfo(0, 0, 0);
	}
	else
	{
		setMeshInfo(0, 0, 0);
	}
}

CSurfaceMeshInfoPanel::CSurfaceMeshInfoPanel(QWidget* parent) : QWidget(parent)
{
	m_nodes = new QLabel;
	m_edges = new QLabel;
	m_faces = new QLabel;

	QFormLayout* form = new QFormLayout;
	form->addRow("Nodes:", m_nodes);
	form->addRow("Edges:", m_edges);
	form->addRow("Faces:", m_faces);

	setLayout(form);

	setInfo(0, 0, 0);
}

void CSurfaceMeshInfoPanel::setInfo(int nodes, int edges, int faces)
{
	m_nodes->setText(QString::number(nodes));
	m_edges->setText(QString::number(edges));
	m_faces->setText(QString::number(faces));
}

void CSurfaceMeshInfoPanel::setInfo(const FESurfaceMesh* pm)
{
	if (pm)
	{
		setInfo(pm->Nodes(), pm->Edges(), pm->Faces());
	}
	else
	{
		setInfo(0, 0, 0);
	}
}
