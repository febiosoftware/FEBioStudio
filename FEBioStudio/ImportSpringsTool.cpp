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
#include <QDir>

CImportSpringsTool::CImportSpringsTool(CMainWindow* wnd) : CBasicTool(wnd, "Import Springs", HAS_APPLY_BUTTON)
{
	addResourceProperty(&m_fileName, "Filename");
	addDoubleProperty(&m_tol, "Snap tolerance");
	addBoolProperty(&m_bintersect, "Check for intersections");
	addEnumProperty(&m_type, "Element type")->setEnumValues(QStringList() << "Linear spring" << "Nonlinear spring" << "Hill spring" << "Linear truss");

	m_type = 0;
	m_tol = 1e-6;
	m_bintersect = true;
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
	FILE* fp = fopen(file.c_str(), "rt");
	if (fp == nullptr) return false;

	char szline[512] = { 0 };
	int nid;
	double r0[3], r1[3];
	vector<vec3d> points;
	int readMode = -1; // 0 = read nodes, 1 = read lines
	while (!feof(fp))
	{
		if (fgets(szline, 511, fp))
		{
			if (strstr(szline, "POINTS"))
			{
				int n = atoi(szline + 6);
				points.reserve(n);
				readMode = 0;
			}
			else if (strstr(szline, "LINES"))
			{
				int n = atoi(szline + 5);
				readMode = 1;
			}
			else if (readMode == 0)
			{
				float r[9];
				int nread = sscanf(szline, "%g%g%g%g%g%g%g%g%g", r, r+1, r+2, r+3, r+4, r+5, r+6, r+7, r+8);
				for (int i = 0; i < nread; i += 3)
				{
					vec3d ri(r[i], r[i + 1], r[i + 2]);
					points.push_back(ri);
				}
			}
			else if (readMode == 1)
			{
				int n[3];
				int nread = sscanf(szline, "%d%d%d", n, n + 1, n + 2);
				if (nread == 3)
				{
					SPRING s;
					s.r0 = points[n[1]];
					s.r1 = points[n[2]];

					m_springs.push_back(s);
				}
			}
		}
	}
	fclose(fp);

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
	for (int i = 0; i < N; ++i)
	{
		FSNode& ni = m->Node(i);
		if (ni.IsExterior())
		{
			vec3d ri = m->LocalToGlobal(ni.r);

			double l2 = (r - ri).SqrLength();
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

	int notFound = 0;
	for (size_t i = 0; i < m_springs.size(); ++i)
	{
		SPRING spring = m_springs[i];

		// check for intersections first
		if (m_bintersect)
		{
			Intersect(po, spring);
		}

		// see if the node exists
		int na = findNode(po, spring.r0, m_tol);
		if (na == -1) { notFound++;  na = po->AddNode(spring.r0); }
		int nb = findNode(po, spring.r1, m_tol);
		if (nb == -1) { notFound++; nb = po->AddNode(spring.r1); }

		// add it to the discrete element set
		dset->AddElement(na, nb);
	}

	if (notFound > 0)
	{
		SetErrorString(QString("%1 new vertices were added to the mesh.").arg(notFound));
	}

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

	int notFound = 0;
	for (size_t i = 0; i < m_springs.size(); ++i)
	{
		SPRING& spring = m_springs[i];

		// check for intersections first
		if (m_bintersect)
		{
			Intersect(po, spring);
		}

		// see if the node exists
		int na = findNode(po, spring.r0, m_tol);
		if (na == -1) { notFound++;  na = po->AddNode(spring.r0); }
		int nb = findNode(po, spring.r1, m_tol);
		if (nb == -1) { notFound++; nb = po->AddNode(spring.r1); }

		spring.n0 = na;
		spring.n1 = nb;
	}

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

	if (notFound > 0)
	{
		SetErrorString(QString("%1 new vertices were added to the mesh.").arg(notFound));
	}

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
