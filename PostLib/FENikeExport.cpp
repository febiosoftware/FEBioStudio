#include "stdafx.h"
#include "FENikeExport.h"
#include <stdio.h>

using namespace Post;

FENikeExport::FENikeExport(void)
{
}

FENikeExport::~FENikeExport(void)
{
}

bool FENikeExport::Save(FEModel &fem, const char *szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	int i;

	FEState& s = *fem.GetActiveState();
	FEMeshBase* pm = s.GetFEMesh();

	// export nodes
	for (i=0; i<pm->Nodes(); ++i)
	{
		const vec3f& r = s.m_NODE[i].m_rt;
		fprintf(fp, "%8d%5d%20lg%20lg%20lg%5d\n", i+1, 0, r.x, r.y, r.z, 0);
	}

	// export elements
	for (i=0; i<pm->Elements(); ++i)
	{
		FEElement& e = pm->Element(i);
		int* n = e.m_node;
		fprintf(fp,"%8d%5d%8d%8d%8d%8d%8d%8d%8d%8d\n", i+1, 1,n[0]+1,n[1]+1,n[2]+1,n[3]+1,n[4]+1,n[5]+1,n[6]+1,n[7]+1);
	}

	fclose(fp);

	return true;
}
