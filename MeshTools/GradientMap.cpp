#include "stdafx.h"
#include "GradientMap.h"
#include <MeshLib/FEMesh.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshTools/FENodeData.h>

GradientMap::GradientMap()
{
}

void GradientMap::Apply(const FENodeData& data, vector<vec3d>& out, int niter)
{
	FEMesh* pm = data.GetMesh();
	if (pm==0) return;
	FEMesh& mesh = *pm;

	int NE = mesh.Elements();
	out.assign(NE, vec3d(0,0,0));

	double v[FEElement::MAX_NODES];
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);

		int ne = el.Nodes();
		for (int j = 0; j<ne; ++j) v[j] = data.get(el.m_node[j]);

		vec3d g(0, 0, 0);
		for (int j = 0; j<ne; ++j) g += FEMeshMetrics::Gradient(*pm, el, j, v);
		g.Normalize();

		out[i] = g;
	}	

	// do smoothess iterations
	for (int n=0; n<niter; ++n)
	{
		for (int i=0; i<NE; ++i)
		{
			FEElement& el = mesh.Element(i);

			vec3d avg(0,0,0);
			int nf = el.Faces(), nnb = 0;
			for (int j=0; j<nf; ++j)
			{
				if (el.m_nbr[j] >= 0)
				{
					avg += out[el.m_nbr[j]];
					nnb++;
				}
			}
			if (nnb > 0) avg /= (double) nnb;

				out[i] = avg;
		}
	}
}
