#include "FETetGenExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>

FETetGenExport::FETetGenExport()
{
}

FETetGenExport::~FETetGenExport()
{
}

bool FETetGenExport::Export(FEProject &prj, const char *szfile)
{
	// get the model
	FEModel* ps = &prj.GetFEModel();
	GModel& model = ps->GetModel();

	char sznode[256] = {0};
	strncpy(sznode, szfile, strlen(szfile));
	char* ch = strrchr(sznode, '.');
	if (ch == 0) ch = sznode+strlen(sznode);
	sprintf(ch, ".node");
	FILE* fp = fopen(sznode, "wt");

	// write the nodes
	fprintf(fp, "%d %d %d %d\n", model.FENodes(), 3, 0, 0);
	int n = 1;
	for (int i=0; i<model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		int NN = pm->Nodes();
		for (int j=0; j<NN; ++j, ++n)
		{
			FENode& nd = pm->Node(j);
			nd.m_ntag = n;
			fprintf(fp, "%d %lg %lg %lg\n", n, nd.r.x, nd.r.y, nd.r.z);
		}
	}
	fclose(fp);

	// write the elements
	fp = fopen(szfile, "wt");
	fprintf(fp, "%d %d %d\n", model.Elements(), 4, 0);
	n = 1;
	for (int i=0; i<model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		int NE = pm->Elements();
		for (int j=0; j<NE; ++j, ++n)
		{
			FEElement& el = pm->Element(j);
			assert(el.Type() == FE_TET4);
			int m[4];
			m[0] = pm->Node(el.m_node[0]).m_ntag;
			m[1] = pm->Node(el.m_node[1]).m_ntag;
			m[2] = pm->Node(el.m_node[2]).m_ntag;
			m[3] = pm->Node(el.m_node[3]).m_ntag;
			fprintf(fp, "%d %d %d %d %d\n", n, m[0], m[1], m[2], m[3]);
		}
	}
	fclose(fp);
	return true;
}
