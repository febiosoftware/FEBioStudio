#include "FEViewpointExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>

FEViewpointExport::FEViewpointExport(FEProject& prj) : FEFileExport(prj)
{
}

FEViewpointExport::~FEViewpointExport(void)
{
}

bool FEViewpointExport::Write(const char* szfile)
{
	int m, n, k;
	// the file name should just be the base name of the coordinate
	// and element file. So we make seperate filenames for each
	// of the two files.
	char szfelm[512] = {0}, szfcor[512] = {0};
	sprintf(szfelm, "%s.elm", szfile);	// element file name
	sprintf(szfcor, "%s.cor", szfile);	// coordinate file name

	// write coordinates file
	FILE* fp = fopen(szfcor, "wt");
	if (fp == 0) return false;

	FEModel* ps = &m_prj.GetFEModel();
	GModel& model = ps->GetModel();

	int nn = 1;
	for (m=0; m<model.Objects(); ++m)
	{
		FEMesh* pm = model.Object(m)->GetFEMesh();
		for (n=0; n<pm->Nodes(); ++n, ++nn)
		{
			FENode& node = pm->Node(n);
			node.m_ntag = nn;
			vec3d& r = node.r;

			fprintf(fp, "%d, %lg, %lg, %lg\n", nn, r.x, r.y, r.z);
		}
	}

	fclose(fp);

	// write element connectivity file
	fp = fopen(szfelm, "wt");
	if (fp == 0) return false;

	int ne = 1;
	char szdefault[] = "part";
	for (m=0; m<model.Objects(); ++m)
	{
		GObject* po = model.Object(m);
		FEMesh* pm = po->GetFEMesh();
		const char* sz = po->GetName().c_str();
		if (strlen(sz) == 0) sz = szdefault;
		for (n=0; n<pm->Elements(); ++n, ++ne)
		{
			FEElement& el = pm->Element(n);
			int* l = el.m_node;
			fprintf(fp, "%s", sz);
			for (k=0; k<el.Nodes(); ++k) fprintf(fp, " %d", pm->Node(l[k]).m_ntag);
			fprintf(fp, "\n");
		}
	}

	fclose(fp);

	return true;
}
