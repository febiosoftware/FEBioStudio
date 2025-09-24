/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "MeshInfoPanel.h"
#include <QLabel>
#include <QFormLayout>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <GeomLib/GObject.h>
#include "units.h"
#include <FECore/units.h>

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
		BOX box = po->GetGlobalBox();
		setDimensions(box.Width(), box.Height(), box.Depth());

		FSMesh* pm = po->GetFEMesh();
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

void CSurfaceMeshInfoPanel::setInfo(const FSSurfaceMesh* pm)
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

CPartInfoPanel::CPartInfoPanel(QWidget* parent) : QWidget(parent)
{
	m_solid = new QLabel;
	m_shell = new QLabel;
	m_beam  = new QLabel;

	QFormLayout* form = new QFormLayout;
	form->addRow("Solid elements:", m_solid);
	form->addRow("Shell elements:", m_shell);
	form->addRow("Beam elements:" , m_beam);

	setLayout(form);

	setInfo(0);
}

void CPartInfoPanel::setInfo(GPart* pg)
{
	if (pg == nullptr) { setPartInfo(0, 0, 0); return; }
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	if (po == nullptr) { setPartInfo(0, 0, 0); return; }
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) { setPartInfo(0, 0, 0); return; }

	int nsolid = 0;
	int nshell = 0;
	int nbeam  = 0;
	int pid = pg->GetLocalID();
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_gid == pid)
		{
			if      (el.IsSolid()) nsolid++;
			else if (el.IsShell()) nshell++;
			else if (el.IsBeam ()) nbeam ++;
		}
	}
	setPartInfo(nsolid, nshell, nbeam);
}

void CPartInfoPanel::setPartInfo(int numSolid, int numShell, int numBeam)
{
	m_solid->setText(QString::number(numSolid));
	m_shell->setText(QString::number(numShell));
	m_beam ->setText(QString::number(numBeam ));
}
