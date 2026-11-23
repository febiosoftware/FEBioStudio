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
#include "NetGenFacetMesher.h"
#include <MeshLib/FSMesh.h>
#include <FSCore/FSLogger.h>
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#define NO_PARALLEL_THREADS
#define NOMINMAX
#endif

#ifdef HAS_NETGEN
#include <cstddef>
namespace nglib {
#include <nglib.h>
}
#include <meshing.hpp>

using namespace nglib;
using namespace netgen;
#endif

NetGenFacetMesher::NetGenFacetMesher(GObject& o) : NetGenMesher(o)
{
}

// Fallback: convert the patch's own triangulation into a quad-dominant shell mesh by greedily merging
// pairs of adjacent triangles into well-shaped convex quads. No element size control (the patch's
// triangulation is reused), but it always works, also for open surfaces.
static FSMesh* PairTrianglesToQuads(const FacePatch& P, bool quad)
{
	const int NT = P.Triangles();
	std::vector<int> partner(NT, -1);		// triangle --> partner triangle
	std::vector<int> quadNodes(NT * 4, -1);	// quad node ordering stored with the first triangle

	if (quad)
	{
		// directed edge (a,b) --> triangle
		std::map<std::pair<int, int>, int> dedge;
		for (int t = 0; t < NT; ++t)
			for (int k = 0; k < 3; ++k)
				dedge[{ P.tri[3 * t + k], P.tri[3 * t + (k + 1) % 3] }] = t;

		struct Cand { double score; int t1, t2; int q[4]; };
		std::vector<Cand> cands;
		const double dmax = 50.0 * 3.14159265358979 / 180.0;	// max deviation of a corner from 90 degrees
		for (int t1 = 0; t1 < NT; ++t1)
		{
			for (int k = 0; k < 3; ++k)
			{
				int u = P.tri[3 * t1 + k], v = P.tri[3 * t1 + (k + 1) % 3], w1 = P.tri[3 * t1 + (k + 2) % 3];
				auto it = dedge.find({ v, u });
				if (it == dedge.end()) continue;
				int t2 = it->second;
				if (t2 <= t1) continue;
				int w2 = -1;
				for (int j = 0; j < 3; ++j)
				{
					int a = P.tri[3 * t2 + j];
					if ((a != u) && (a != v)) w2 = a;
				}
				if (w2 < 0) continue;

				int q[4] = { u, w2, v, w1 };
				vec3d n = (P.pts[v] - P.pts[u]) ^ (P.pts[w1] - P.pts[u]);
				double nl = n.norm();
				if (nl == 0) continue;
				n /= nl;

				bool ok = true;
				double worst = 0;
				for (int c = 0; c < 4 && ok; ++c)
				{
					vec3d a = P.pts[q[(c + 3) % 4]] - P.pts[q[c]];
					vec3d b = P.pts[q[(c + 1) % 4]] - P.pts[q[c]];
					double la = a.norm(), lb = b.norm();
					if ((la == 0) || (lb == 0)) { ok = false; break; }
					// interior angle at corner c (convexity: b x a must point along n)
					double sn = ((b ^ a) * n) / (la * lb);
					double cs = (a * b) / (la * lb);
					if (sn <= 0) { ok = false; break; }
					double ang = std::atan2(sn, cs);
					worst = std::max(worst, std::fabs(ang - 0.5 * 3.14159265358979));
				}
				if (!ok || (worst > dmax)) continue;

				Cand cd; cd.score = worst; cd.t1 = t1; cd.t2 = t2;
				for (int c = 0; c < 4; ++c) cd.q[c] = q[c];
				cands.push_back(cd);
			}
		}
		std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) { return a.score < b.score; });
		for (const Cand& cd : cands)
		{
			if ((partner[cd.t1] != -1) || (partner[cd.t2] != -1)) continue;
			partner[cd.t1] = cd.t2; partner[cd.t2] = cd.t1;
			for (int c = 0; c < 4; ++c) quadNodes[cd.t1 * 4 + c] = cd.q[c];
		}
	}

	// count elements
	int ne = 0;
	for (int t = 0; t < NT; ++t)
	{
		if (partner[t] == -1) ne++;
		else if (quadNodes[t * 4] != -1) ne++;
	}

	FSMesh* mesh = new FSMesh();
	mesh->Create((int)P.pts.size(), ne, ne, 0);
	for (size_t i = 0; i < P.pts.size(); ++i) mesh->Node((int)i).r = P.pts[i];

	int e = 0;
	for (int t = 0; t < NT; ++t)
	{
		bool isQuad = (partner[t] != -1);
		if (isQuad && (quadNodes[t * 4] == -1)) continue;	// the second triangle of a pair

		FSFace& face = mesh->Face(e);
		FSElement& el = mesh->Element(e);
		face.m_gid = 0;
		el.m_gid = 0;
		if (isQuad)
		{
			face.SetType(FE_FACE_QUAD4);
			el.SetType(FE_QUAD4);
			for (int k = 0; k < 4; ++k) face.n[k] = el.m_node[k] = quadNodes[t * 4 + k];
		}
		else
		{
			face.SetType(FE_FACE_TRI3);
			el.SetType(FE_TRI3);
			for (int k = 0; k < 3; ++k) face.n[k] = el.m_node[k] = P.tri[3 * t + k];
			face.n[3] = el.m_node[3] = face.n[2];
		}
		e++;
	}
	mesh->BuildMesh();
	return mesh;
}

FSMesh* NetGenFacetMesher::BuildMesh()
{
#ifdef HAS_NETGEN
	if (m_patch.empty())
	{
		setErrorString("No face data to mesh.");
		return nullptr;
	}

	FSMesh* result = BuildWithNetGen();
	if (result) return result;

	// NetGen could not mesh this patch (it typically has trouble with open surfaces), so
	// convert the patch's triangulation to a quad-dominant mesh instead.
	FSLogger::Write("NetGen failed on this surface (%s). Falling back to merging the existing triangulation into quads (no element size control).\n", GetErrorString().c_str());
	setErrorString("");
	return PairTrianglesToQuads(m_patch, GetBoolValue(NetGenMesher::QUADMESH));
#else
	setErrorString("This version of FEBio Studio was not built with NetGen support.");
	return PairTrianglesToQuads(m_patch, GetBoolValue(NetGenMesher::QUADMESH));
#endif
}

#ifdef HAS_NETGEN
FSMesh* NetGenFacetMesher::BuildWithNetGen()
{
	Ng_Init();

	// build the STL-type geometry
	Ng_STL_Geometry* geom = Ng_STL_NewGeometry();
	for (int i = 0; i < m_patch.Triangles(); ++i)
	{
		double p[3][3];
		for (int k = 0; k < 3; ++k)
		{
			const vec3d& r = m_patch.pts[m_patch.tri[3 * i + k]];
			p[k][0] = r.x; p[k][1] = r.y; p[k][2] = r.z;
		}
		Ng_STL_AddTriangle(geom, p[0], p[1], p[2]);
	}

	Ng_Mesh* ngmesh = Ng_NewMesh();

	Ng_Meshing_Parameters mp;
	mp.uselocalh = (GetBoolValue(NetGenMesher::USELOCALH) ? 1 : 0);
	mp.maxh = GetFloatValue(NetGenMesher::MAXELEMSIZE);
	mp.minh = GetFloatValue(NetGenMesher::MINELEMSIZE);
	mp.grading = GetFloatValue(NetGenMesher::GRADING);
	mp.elementsperedge = GetFloatValue(NetGenMesher::ELEMPEREDGE);
	mp.elementspercurve = GetFloatValue(NetGenMesher::ELEMPERCURV);
	mp.optsteps_2d = GetIntValue(NetGenMesher::NROPT2D);
	mp.optsteps_3d = GetIntValue(NetGenMesher::NROPT3D);
	mp.second_order = (GetBoolValue(NetGenMesher::SECONDORDER) ? 1 : 0);
	mp.check_overlapping_boundary = 0;
	mp.closeedgeenable = 0;
	mp.closeedgefact = 1.0;
	mp.optsurfmeshenable = 1;
	mp.quad_dominated = (GetBoolValue(NetGenMesher::QUADMESH) ? 1 : 0);

	FSMesh* femesh = nullptr;
	try {
		Ng_Result ret = Ng_STL_InitSTLGeometry(geom);
		FSLogger::Write("Ng_STL_InitSTLGeometry returned %d\n", (int)ret);
		if (ret == NG_OK) { ret = Ng_STL_MakeEdges(geom, ngmesh, &mp); FSLogger::Write("Ng_STL_MakeEdges returned %d\n", (int)ret); }
		if (ret == NG_OK) { ret = Ng_STL_GenerateSurfaceMesh(geom, ngmesh, &mp); FSLogger::Write("Ng_STL_GenerateSurfaceMesh returned %d (maxh = %g, minh = %g)\n", (int)ret, mp.maxh, mp.minh); }

		if ((ret == NG_OK) && (Ng_GetNSE(ngmesh) > 0))
		{
			FSLogger::Write("Number of points = %d\n", Ng_GetNP(ngmesh));
			FSLogger::Write("Number of surface elements = %d\n", Ng_GetNSE(ngmesh));
			femesh = NGMeshToFEMesh(nullptr, (Mesh*)ngmesh, GetBoolValue(SECONDORDER));
		}
		else setErrorString("NetGen failed to generate the surface mesh.");
	}
	catch (...)
	{
		setErrorString("NetGen raised an exception while meshing the faces.");
	}

	Ng_DeleteMesh(ngmesh);
	Ng_Exit();
	return femesh;
}
#endif
