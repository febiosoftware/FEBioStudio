#include "stdafx.h"
#include "FEMeshOverlap.h"
#include <MeshLib/FEMesh.h>
#include <PostLib/tools.h>
#include <GeomLib/GObject.h>
using namespace MeshTools;
using namespace std;

std::vector<int> MeshTools::FindSurfaceOverlap(FEMesh* mesh, FEMeshBase* trg)
{
	// loop over all of the surface nodes
	mesh->TagAllNodes(0);
	int NN = mesh->Nodes();
	int NF = mesh->Faces();
	vector<vec3d> normalList(NN, vec3d(0, 0, 0));
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = mesh->Face(i);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			normalList[f.n[j]] += f.m_nn[j];
			mesh->Node(f.n[j]).m_ntag = 1;
		}
	}

	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		FENode& node = mesh->Node(i);
		if (node.m_ntag == 1)
		{
			// convert between coordinate systems
			vec3d r_global = mesh->LocalToGlobal(node.r);
			vec3d r = trg->GlobalToLocal(r_global);
			vec3f rf = to_vec3f(r);

			// get the normal at this node
			vec3d N = normalList[i];
			N.Normalize();
			N = mesh->GetGObject()->GetTransform().LocalToGlobalNormal(N);
			N = trg->GetGObject()->GetTransform().GlobalToLocalNormal(N);
			vec3f Nf = to_vec3f(N);

			// find the normal projection onto the target surface
			bool bfound = false;
			float Dmin = 0.f;
			bool backFacing = false;
			vec3f y[FEFace::MAX_NODES], q;
			for (int n = 0; n < trg->Faces(); ++n)
			{
				FEFace& ft = trg->Face(n);

				for (int m = 0; m < ft.Nodes(); ++m) y[m] = to_vec3f(trg->Node(ft.n[m]).r);

				// project r onto the the facet along its normal
				vec3f p;
				if (ProjectToFacet(y, ft.Nodes(), rf, Nf, p, 0.01))
				{
					// return the closest projection
					float D = (p - rf)*(p - rf);
					if ((D < Dmin) || (bfound == false))
					{
						// only consider backfacing intersections
						if (Nf*(p - rf) <= 0.f)
						{
							q = p;
							Dmin = D;
							bfound = true;
							backFacing = (Nf*ft.m_fn < 0.f);
						}
					}
				}
			}

			if (bfound && backFacing)
			{
				node.m_ntag = 2;
			}
		}
	}

	vector<int> faceList;
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = mesh->Face(i);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			if (mesh->Node(f.n[j]).m_ntag == 2)
			{
				faceList.push_back(i);
				break;
			}
		}
	}
	return faceList;
}
