#include "FEBYUExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>

FEBYUExport::FEBYUExport(void)
{
}

FEBYUExport::~FEBYUExport(void)
{
}

bool FEBYUExport::Export(FEProject& prj, const char* szfile)
{
	int i, j, k, n;

	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	FEModel* ps = &prj.GetFEModel();
	GModel& model = ps->GetModel();

	// for now we put everything in one part
	int parts = 1;

	// count total nr of faces
	int faces = model.FEFaces();

	// count nr of face edges
	int edges = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		FEMesh& m = *pm;
		for (j=0; j<m.Nodes(); ++j) m.Node(j).m_ntag = 0;
		for (j=0; j<m.Faces(); ++j)
		{
			FEFace& f = m.Face(j);
			n = f.Nodes();
			edges += n;
			for (k=0; k<n; ++k) m.Node(f.n[k]).m_ntag = 1;
		}
	}

	// count nr of nodes
	int nodes = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Nodes(); ++j) if (m.Node(j).m_ntag == 1) m.Node(j).m_ntag = ++nodes;
	}

	// --- H E A D E R ---
	fprintf(fp, "%d %d %d %d\n", parts, nodes, faces, edges);

	// --- P A R T ---
	int nfirst = 1;
	int nlast = faces;
	fprintf(fp, "%d %d\n", nfirst, nlast);

	// --- N O D E S ---
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Nodes(); ++j)
		{
			FENode& n = m.Node(j);
			if (n.m_ntag)
				fprintf(fp, "%g %g %g\n", n.r.x, n.r.y, n.r.z);
		}
	}

	// --- E D G E S ---
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Faces(); ++j)
		{
			FEFace& f = m.Face(j);
			n = f.Nodes();
			if (n == 3)
				fprintf(fp, "%d %d %d\n", m.Node(f.n[0]).m_ntag, m.Node(f.n[1]).m_ntag, -m.Node(f.n[2]).m_ntag);
			else if (n == 4)
				fprintf(fp, "%d %d %d %d\n", m.Node(f.n[0]).m_ntag, m.Node(f.n[1]).m_ntag, m.Node(f.n[2]).m_ntag, -m.Node(f.n[3]).m_ntag);
			else 
				assert(false);
		}
	}

	fclose(fp);

	return true;
}
