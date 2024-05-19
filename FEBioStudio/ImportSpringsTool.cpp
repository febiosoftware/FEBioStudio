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
#include "ImportSpringsTool.h"
#include "ModelDocument.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GModel.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FEMeshBuilder.h>
#include <FEBioLink/FEBioClass.h>
#include <VTKLib/VTKLegacyFileReader.h>
#include <QDir>

// in GMaterial.cpp
extern GLColor col[GMaterial::MAX_COLORS];

CImportSpringsTool::CImportSpringsTool(CMainWindow* wnd) : CBasicTool(wnd, "Import Springs", HAS_APPLY_BUTTON)
{
	m_snap = true;
	m_type = 0;
	m_tol = 1e-6;
	m_bintersect = false;

	addResourceProperty(&m_fileName, "Filename");
	addBoolProperty(&m_snap, "Snap end-points to geometry");
	addDoubleProperty(&m_tol, "Snap tolerance");
	addBoolProperty(&m_bintersect, "Check for intersections");
	addEnumProperty(&m_type, "Element type")->setEnumValues(QStringList() << "Linear spring" << "Nonlinear spring" << "Hill spring" << "Linear truss");
}

bool CImportSpringsTool::OnApply()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return SetErrorString("No document open");

	// get the current object
	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return SetErrorString("You need to select the object where the springs will be added.");

	// Make sure it is a GMeshObject
	GMeshObject* mo = dynamic_cast<GMeshObject*>(po);
	if (mo == nullptr) return SetErrorString("This tool only works with editable meshes.");

	// read the file
	if (ReadFile() == false) return SetErrorString("There was a problem reading the file.");

	// check if we have springs
	if (m_springs.empty()) return SetErrorString("The file did not contain any springs or was not properly formatted.");

	// process the springs
	int newNodes = ProcessSprings(mo);
	if (newNodes > 0)
	{
		SetErrorString(QString("%1 new vertices were added to the mesh.").arg(newNodes));
	}

	// apply the springs
	if (mo)
	{
		if (m_type < 3)
			return AddSprings(doc->GetGModel(), mo);
		else 
			return AddTrusses(doc->GetGModel(), mo);
	}

	return false;
}

bool CImportSpringsTool::ReadFile()
{
	// clear the spring list
	m_springs.clear();

	std::string file = m_fileName.toStdString();

	// if the extension is vtk, we read the vtk file
	const char* szext = strrchr(file.c_str(), '.');
	if (strcmp(szext, ".vtk") == 0) return ReadVTKFile();
	else return ReadTXTFile();
}

bool CImportSpringsTool::ReadVTKFile()
{
	std::string file = m_fileName.toStdString();
	VTK::vtkLegacyFileReader vtk;
	if (!vtk.Load(file.c_str())) return false;

	const VTK::vtkPiece& piece = vtk.GetVTKModel().Piece(0);
	size_t NP = piece.Points();
	vector<vec3d> points(NP);
	for (size_t i = 0; i < NP; ++i)
	{
		VTK::vtkPoint p = piece.Point(i);
		points[i] = vec3d(p.x, p.y, p.z);
	}

	size_t NC = piece.Cells();
	for (size_t i = 0; i < NC; ++i)
	{
		VTK::vtkCell c = piece.Cell(i);
		if (c.m_cellType == VTK::vtkCell::VTK_LINE)
		{
			SPRING s;
			s.r0 = points[c.m_node[0]];
			s.r1 = points[c.m_node[1]];
			m_springs.push_back(s);
		}
	}

	return true;
}

bool CImportSpringsTool::ReadTXTFile()
{
	std::string file = m_fileName.toStdString();

	FILE* fp = fopen(file.c_str(), "rt");
	if (fp == nullptr) return false;

	char szline[512] = { 0 };
	int nid;
	double r0[3], r1[3];
	while (!feof(fp))
	{
		if (fgets(szline, 511, fp))
		{
			int nread = sscanf(szline, "%d,%lg,%lg,%lg,%lg,%lg,%lg", &nid, r0, r0 + 1, r0 + 2, r1, r1 + 1, r1 + 2);
			if (nread == 7)
			{
				SPRING s;
				s.r0 = vec3d(r0[0], r0[1], r0[2]);
				s.r1 = vec3d(r1[0], r1[1], r1[2]);

				m_springs.push_back(s);
			}
		}
	}

	fclose(fp);
	return true;
}

int findNode(GMeshObject* po, const vec3d& r, double tol)
{
	// find closest node
	int imin = -1;
	double l2min = 0.0;
	FSMesh* m = po->GetFEMesh();
	int N = m->Nodes();
	imin = -1;
	vec3d r_local = m->GlobalToLocal(r);
	for (int i = 0; i < N; ++i)
	{
		FSNode& ni = m->Node(i);
		if (ni.IsExterior())
		{
			vec3d dr = r_local - ni.r;
			double l2 = dr.SqrLength();
			if ((imin == -1) || (l2 < l2min))
			{
				imin = i;
				l2min = l2;
			}
		}
	}
	if ((imin!=-1) && (l2min < tol*tol))
	{
		return po->MakeGNode(imin);
	}

	return -1;
}

int findPoint(const vector<pair<vec3d, int>>& points, const vec3d& r)
{
	const double eps = 1e-12;
	int n = -1;
	for (size_t i = 0; i < points.size(); ++i)
	{
		const vec3d& ri = points[i].first;
		if ((ri - r).Length() < eps)
		{
			n = (int)i;
			break;
		}
	}
	return n;
}

int CImportSpringsTool::ProcessSprings(GMeshObject* po)
{
	// process intersections first
	if (m_bintersect)
	{
		for (size_t i = 0; i < m_springs.size(); ++i)
		{
			SPRING& spring = m_springs[i];
			Intersect(po, spring);
		}
	}

	// build node list
	vector<pair<vec3d, int>> rt;
	for (SPRING& s : m_springs)
	{
		s.n0 = findPoint(rt, s.r0);
		if (s.n0 == -1) { rt.push_back({ s.r0, -1 }); s.n0 = rt.size() - 1; }
		else rt[s.n0].second -= 1;

		s.n1 = findPoint(rt, s.r1);
		if (s.n1 == -1) { rt.push_back({ s.r1, 1 }); s.n1 = rt.size() - 1; }
		else rt[s.n1].second += 1;
	}

	// add unique nodes to object (only snap end-points)
	if (m_snap)
	{
		for (auto& rt_i : rt)
		{
			int n = -1;
			if (rt_i.second > 0)
			{
				// this is an end node, so first try to snap it
				n = findNode(po, rt_i.first, m_tol);
			}
			rt_i.second = n;
		}
	}

	int newNodes = 0;
	for (auto& rt_i : rt)
	{
		if (rt_i.second == -1)
		{
			rt_i.second = po->AddNode(rt_i.first); 
			newNodes++; 
		}
	}

	for (SPRING& s : m_springs)
	{
		s.n0 = rt[s.n0].second;
		s.n1 = rt[s.n1].second;
	}

	return newNodes;
}

bool CImportSpringsTool::AddSprings(GModel* gm, GMeshObject* po)
{
	// create the discrete set
	GDiscreteSpringSet* dset = new GDiscreteSpringSet(gm);

	// set the spring material
	FSModel* fem = gm->GetFSModel();
	switch (m_type)
	{
	case 0: dset->SetMaterial(FEBio::CreateDiscreteMaterial("linear spring", fem)); break;
	case 1: dset->SetMaterial(FEBio::CreateDiscreteMaterial("nonlinear spring", fem)); break;
	case 2: dset->SetMaterial(FEBio::CreateDiscreteMaterial("Hill", fem)); break;
	default:
		assert(false);
		return false;
	}	

	// extract the name from the file name
	QFileInfo file(m_fileName);
	QString baseName = file.baseName();
	dset->SetName(baseName.toStdString());

	// add the discrete set to the model
	gm->AddDiscreteObject(dset);

	for (size_t i = 0; i < m_springs.size(); ++i)
	{
		SPRING& spring = m_springs[i];
	
		// add it to the discrete element set
		dset->AddElement(spring.n0, spring.n1);
	}

	int n = gm->DiscreteObjects();
	dset->SetColor(col[n % GMaterial::MAX_COLORS]);

	return true;
}

bool CImportSpringsTool::AddTrusses(GModel* gm, GMeshObject* po)
{
	// set the spring material
	FSModel* fem = gm->GetFSModel();

	// extract the name from the file name
	QFileInfo file(m_fileName);
	QString baseName = file.baseName();

	FSMesh& m = *po->GetFEMesh();

	// add new elements to the mesh
	int gid = m.CountElementPartitions();
	int cid = m.CountEdgePartitions();
	int NE0 = m.Elements();
	int NC0 = m.Edges();
	m.Create(m.Nodes(), NE0 + m_springs.size(), 0, NC0 + m_springs.size());
	for (int i = 0; i < m.Nodes(); ++i) m.Node(i).m_ntag = i;
	m.Edge(NC0).m_gid = m.CountEdgePartitions();
	for (int i = 0; i < m_springs.size(); ++i)
	{
		SPRING& spring = m_springs[i];
		FSElement& el = m.Element(NE0 + i);
		el.SetType(FE_BEAM2);
		el.m_gid = gid;
		int na = po->FindNode(spring.n0)->GetLocalID();
		int nb = po->FindNode(spring.n1)->GetLocalID();
		na = m.FindNodeFromID(na)->m_ntag;
		nb = m.FindNodeFromID(nb)->m_ntag;
		el.m_node[0] = na;
		el.m_node[1] = nb;

		FSEdge& ed = m.Edge(NC0 + i);
		ed.SetType(FEEdgeType::FE_EDGE2);
		ed.SetExterior(true);
		ed.m_gid = cid++;
		ed.n[0] = na;
		ed.n[1] = nb;
	}
	po->Update(true);
	GPart* pg = po->Part(gid);
	pg->SetName(baseName.toStdString());
	GBeamSection* pb = dynamic_cast<GBeamSection*>(pg->GetSection()); assert(pb);

	// add a truss material
	FSMaterial* pmat = FEBio::CreateMaterial("linear truss", fem);
	GMaterial* gmat = new GMaterial(pmat);
	fem->AddMaterial(gmat);
	po->AssignMaterial(pg->GetID(), gmat->GetID());

	FEBeamFormulation* bf = FEBio::CreateBeamFormulation("linear-truss", fem);
	pb->SetElementFormulation(bf);

	return true;
}
void CImportSpringsTool::Intersect(GMeshObject* po, CImportSpringsTool::SPRING& spring)
{
	FSMesh* mesh = po->GetFEMesh();

	vec3d n = spring.r1 - spring.r0; n.Normalize();

	vector<vec3d> intersections = FindAllIntersections(*mesh, spring.r0, n);
	for (int i=0; i<intersections.size(); ++i)
	{
		vec3d q = intersections[i];

		// does the projection lie within tolerance
		double d = (q - spring.r0).Length();
		if (d < m_tol)
		{
			// does it decrease the distance
			if ((spring.r1 - q).Length() < (spring.r1 - spring.r0).Length())
			{
				spring.r0 = q;
			}
		}
	}

	intersections = FindAllIntersections(*mesh, spring.r1, -n);
	for (int i = 0; i<intersections.size(); ++i)
	{
		vec3d q = intersections[i];

		// does the projection lie within tolerance
		double d = (q - spring.r1).Length();
		if (d < m_tol)
		{
			// does it decrease the distance
			if ((q - spring.r0).Length() < (spring.r1 - spring.r0).Length())
			{
				spring.r1 = q;
			}
		}
	}
}
