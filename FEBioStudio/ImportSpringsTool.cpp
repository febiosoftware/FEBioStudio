#include "stdafx.h"
#include "ImportSpringsTool.h"
#include "Document.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/GModel.h>
#include <QDir>

CImportSpringsTool::CImportSpringsTool(CMainWindow* wnd) : CBasicTool(wnd, "Import Springs", HAS_APPLY_BUTTON)
{
	addResourceProperty(&m_fileName, "Filename");
	addDoubleProperty(&m_tol, "Snap tolerance");

	m_tol = 1e-6;
}

bool CImportSpringsTool::OnApply()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return false;

	// get the current object
	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return false;

	// Make sure it is a GMeshObject
	GMeshObject* mo = dynamic_cast<GMeshObject*>(po);
	if (mo == nullptr) return false;

	// read the file
	if (ReadFile() == false) return false;

	// apply the springs
	if (mo) return AddSprings(doc->GetGModel(), mo);

	return false;
}

bool CImportSpringsTool::ReadFile()
{
	std::string file = m_fileName.toStdString();

	FILE* fp = fopen(file.c_str(), "rt");
	if (fp == nullptr) return false;

	// clear the spring list
	m_springs.clear();

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
	FEMesh* m = po->GetFEMesh();
	int N = m->Nodes();
	imin = -1;
	for (int i = 0; i < N; ++i)
	{
		FENode& ni = m->Node(i);
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
	if (l2min < tol*tol)
	{
		return po->MakeGNode(imin);
	}

	return -1;
}

bool CImportSpringsTool::AddSprings(GModel* fem, GMeshObject* po)
{
	GDiscreteSpringSet* dset = new GDiscreteSpringSet;
	dset->SetMaterial(new FELinearSpringMaterial);

	// extract the name from the file name
	QFileInfo file(m_fileName);
	QString baseName = file.baseName();
	dset->SetName(baseName.toStdString());

	// add the discrete set to the model
	fem->AddDiscreteObject(dset);

	for (size_t i = 0; i < m_springs.size(); ++i)
	{
		SPRING& spring = m_springs[i];

		// see if the node exists
		int na = findNode(po, spring.r0, m_tol);
		if (na == -1) na = po->AddNode(spring.r0);
		int nb = findNode(po, spring.r1, m_tol);
		if (nb == -1) nb = po->AddNode(spring.r1);

		// add it to the discrete element set
		dset->AddElement(na, nb);
	}

	return true;
}
