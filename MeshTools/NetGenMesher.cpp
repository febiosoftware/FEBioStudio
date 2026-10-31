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
#include "NetGenMesher.h"
#include <MeshLib/FSMesh.h>
#include <GeomLib/GObject.h>

#ifdef _MSC_VER
#define NO_PARALLEL_THREADS
#define NOMINMAX
#endif

#ifdef HAS_NETGEN
// nglib.h includes <stddef.h>; include it first so that size_t etc.
// are not declared inside the nglib namespace below.
#include <cstddef>
namespace nglib {
#include <nglib.h>
}
#include <meshing.hpp>

using namespace nglib;
#endif

NetGenMesher::NetGenMesher(GObject& o) : FEMesher(o)
{
	m_meshGranularity = Moderate;
	m_grading = 0.3;
	m_elemPerEdge = 1;
	m_elemPerCurve = 2;
	AddChoiceParam(m_meshGranularity, "Mesh granularity")->SetEnumNames("Very coarse\0Coarse\0Moderate\0Fine\0Very fine\0User-defined\0");
	AddBoolParam(true, "Use local mesh modifiers")->SetEditable(false);
	AddDoubleParam(m_grading, "Grading")->SetFloatRange(0, 1);
	AddDoubleParam(1000.0, "Max element size");
	AddDoubleParam(1.0, "Min element size");
	AddIntParam(3, "Nr. 2D optimization steps");
	AddIntParam(5, "Nr. 3D optimization steps");
	AddBoolParam(false, "Second-order mesh");
	AddDoubleParam(m_elemPerEdge, "Elements per edge");
	AddDoubleParam(m_elemPerCurve, "Elements per curve");
	AddBoolParam(true, "Quad dominant shell mesh");
	AddBoolParam(false, "Surface mesh refinement");
//	AddDoubleParam(0.0, "Surface mesh size");
}

bool NetGenMesher::UpdateData(bool bsave)
{
	if (bsave)
	{
		int gran = GetIntValue(NetGenMesher::GRANULARITY);
		if (gran != m_meshGranularity)
		{
			m_meshGranularity = gran;
			switch (gran) {
			case VeryCoarse:
				m_grading = 0.7;
				m_elemPerEdge = 0.3;
				m_elemPerCurve = 1.0;
				break;
			case Coarse:
				m_grading = 0.5;
				m_elemPerEdge = 0.5;
				m_elemPerCurve = 1.5;
				break;
			case Moderate:
				m_grading = 0.3;
				m_elemPerEdge = 1.0;
				m_elemPerCurve = 2.0;
				break;
			case Fine:
				m_grading = 0.2;
				m_elemPerEdge = 2.0;
				m_elemPerCurve = 3.0;
				break;
			case VeryFine:
				m_grading = 0.1;
				m_elemPerEdge = 3.0;
				m_elemPerCurve = 5.0;
				break;
			case UserDefined:
				break;
			default:
				assert(false);
			}
			SetFloatValue(GRADING, m_grading);
			SetFloatValue(ELEMPEREDGE, m_elemPerEdge);
			SetFloatValue(ELEMPERCURV, m_elemPerCurve);

			return true;
		}
		else if (gran != UserDefined)
		{
			double grading = GetFloatValue(GRADING);
			double elemPerEdge = GetFloatValue(ELEMPEREDGE);
			double elemPerCurve = GetFloatValue(ELEMPERCURV);
			if ((grading != m_grading) || (elemPerCurve != m_elemPerCurve) || (elemPerEdge != m_elemPerEdge))
			{
				m_meshGranularity = UserDefined;
				SetIntValue(NetGenMesher::GRANULARITY, UserDefined);

				m_grading = grading;
				m_elemPerEdge = elemPerEdge;
				m_elemPerCurve = elemPerCurve;
				return true;
			}
		}
	}
	return FEMesher::UpdateData(bsave);
}

FSTaskProgress NetGenMesher::GetProgress()
{
#ifdef HAS_NETGEN
	FSTaskProgress mp;
	mp.percent = netgen::multithread.percent;
	mp.task = netgen::multithread.task;
	mp.valid = true;
	return mp;
#else
	FSTaskProgress mp;
	return mp;
#endif
}

void NetGenMesher::Terminate()
{
#ifdef HAS_NETGEN
	netgen::multithread.terminate = 1;
#endif
}

#ifdef HAS_NETGEN
namespace {

	// Netgen's index types (PointIndex, ElementIndex, ...) no longer convert
	// implicitly to int. Nr0() returns the zero-based number, independent of
	// the index base Netgen was compiled with (NETGEN_POINTINDEX_BASE).
	inline int N0(netgen::PointIndex pi) { return (int)pi.Nr0(); }

	// zero-based geometry edge number of a segment
	inline int SegmentEdgeID(const netgen::Mesh& ngmesh, const netgen::Segment& seg)
	{
		// Since 2026-03-29 Segment::GetIndex() is the edge-descriptor (region) index,
		// and the geometry edge number is stored in the edge descriptor.
		if (ngmesh.HasEdgeDescriptor(seg))
		{
			int edgeNr = ngmesh.GetEdgeDescriptor(seg).EdgeNr(); // 1-based
			if (edgeNr > 0) return edgeNr - 1;
		}
		return (int)seg.GetIndex().Nr0();
	}

	// zero-based surface number of a surface element
	inline int FaceSurfaceID(const netgen::Mesh& ngmesh, const netgen::Element2dRef& sel)
	{
		return ngmesh.GetFaceDescriptor(sel.GetIndex()).SurfNr() - 1;
	}

	void CopyNodes(const netgen::Mesh& ngmesh, FSMesh* mesh)
	{
		for (netgen::PointIndex pi : ngmesh.Points().Range())
		{
			const netgen::MeshPoint& p = ngmesh[pi];
			FSNode& ni = mesh->Node(N0(pi));
			ni.r = vec3d(p(0), p(1), p(2));
		}
	}

	void CopyEdges(const netgen::Mesh& ngmesh, FSMesh* mesh, bool secondOrder)
	{
		for (netgen::SegmentIndex sgi : ngmesh.LineSegments().Range())
		{
			const netgen::Segment& sel = ngmesh[sgi];

			FSEdge& edge = mesh->Edge(sgi.Nr0());

			// get the edge's group ID (which is the geometry edge number)
			edge.m_gid = SegmentEdgeID(ngmesh, sel);

			if ((secondOrder == false) || (sel.GetNP() < 3))
			{
				edge.SetType(FE_EDGE2);
				edge.n[0] = N0(sel[0]);
				edge.n[1] = N0(sel[1]);
				edge.n[2] = edge.n[1];
			}
			else
			{
				edge.SetType(FE_EDGE3);
				edge.n[0] = N0(sel[0]);
				edge.n[1] = N0(sel[1]);
				edge.n[2] = N0(sel[2]);
			}
		}
	}
}
#endif

FSMesh* NetGenMesher::NGMeshToFEMesh(GObject* po, netgen::Mesh* ngmeshPtr, bool secondOrder)
{
#ifdef HAS_NETGEN
	using namespace netgen;

	const Mesh& ngmesh = *ngmeshPtr;

	int nodes = (int)ngmesh.GetNP();
	int elems = (int)ngmesh.GetNE();
	int faces = (int)ngmesh.GetNSE();
	int edges = (int)ngmesh.GetNSeg();

	FSMesh* mesh = new FSMesh();

	// solid mesh
	if (elems) {
		mesh->Create(nodes, elems, faces, edges);

		// copy nodes
		CopyNodes(ngmesh, mesh);

		// we need to find the FE nodes that correspond to the geometry nodes
		// I don't know if there is a better way to do this (i.e. if NetGen stores this information)
		// so we're using a brute-force method
		// (It looks like the first nodes in the NG mesh correspond to the geometry nodes, though not 100% sure)
		if (po)
		{
			for (int i = 0; i < po->Nodes(); ++i)
			{
				GNode& node = *po->Node(i);
				vec3d rn = node.LocalPosition();

				const double eps = 1e-12;
				int minj = -1;
				double D2min = 0.0;
				for (int j = 0; j < mesh->Nodes(); ++j)
				{
					vec3d rj = mesh->Node(j).r;
					double D2 = (rn - rj).norm2();
					if ((minj == -1) || (D2 < D2min))
					{
						minj = j;
						D2min = D2;
						if (D2min < eps) break;
					}
				}
				assert(minj != -1);
				if ((minj != -1) && (D2min < eps))
				{
					mesh->Node(minj).m_gid = i;
				}
			}
		}

		// copy edges
		CopyEdges(ngmesh, mesh, secondOrder);

		// copy facets
		for (SurfaceElementIndex sei : ngmesh.SurfaceElements().Range())
		{
			const Element2dRef sel = ngmesh[sei];

			FSFace& face = mesh->Face(sei.Nr0());
			face.m_gid = FaceSurfaceID(ngmesh, sel);

			if (secondOrder == false)
			{
				face.SetType(FE_FACE_TRI3);
				face.n[0] = N0(sel[0]);
				face.n[1] = N0(sel[1]);
				face.n[2] = N0(sel[2]);
				face.n[3] = face.n[2];
			}
			else
			{
				face.SetType(FE_FACE_TRI6);
				face.n[0] = N0(sel[0]);
				face.n[1] = N0(sel[1]);
				face.n[2] = N0(sel[2]);
				face.n[3] = N0(sel[5]);
				face.n[4] = N0(sel[3]);
				face.n[5] = N0(sel[4]);
			}
		}

		// copy elements
		for (ElementIndex ei : ngmesh.VolumeElements().Range())
		{
			const ElementRef ngel = ngmesh[ei];

			// Note that I need to invert the element
			FSElement& el = mesh->Element(ei.Nr0());

			el.m_gid = ngel.GetIndex().Nr0(); assert(el.m_gid >= 0);
			if (el.m_gid < 0) el.m_gid = 0;

			if (secondOrder == false)
			{
				el.SetType(FE_TET4);
				el.m_node[0] = N0(ngel[2]);
				el.m_node[1] = N0(ngel[1]);
				el.m_node[2] = N0(ngel[0]);
				el.m_node[3] = N0(ngel[3]);
			}
			else
			{
				el.SetType(FE_TET10);
				el.m_node[0] = N0(ngel[0]);
				el.m_node[1] = N0(ngel[2]);
				el.m_node[2] = N0(ngel[1]);
				el.m_node[3] = N0(ngel[3]);
				el.m_node[4] = N0(ngel[5]);
				el.m_node[5] = N0(ngel[7]);
				el.m_node[6] = N0(ngel[4]);
				el.m_node[7] = N0(ngel[6]);
				el.m_node[8] = N0(ngel[9]);
				el.m_node[9] = N0(ngel[8]);
			}
		}
	}
	// shell mesh
	else {
		mesh->Create(nodes, faces, faces, edges);

		// copy nodes
		CopyNodes(ngmesh, mesh);

		// copy edges
		CopyEdges(ngmesh, mesh, secondOrder);

		// copy facets and shell elements
		for (SurfaceElementIndex sei : ngmesh.SurfaceElements().Range())
		{
			const Element2dRef sel = ngmesh[sei];
			int i = sei.Nr0();

			FSFace& face = mesh->Face(i);
			face.m_gid = FaceSurfaceID(ngmesh, sel);

			FSElement& el = mesh->Element(i);
			el.m_gid = 0;

			int np = sel.GetNP();

			if (secondOrder == false)
			{
				switch (np) {
				case 3:
				{
					face.SetType(FE_FACE_TRI3);
					face.n[0] = N0(sel[0]);
					face.n[1] = N0(sel[1]);
					face.n[2] = N0(sel[2]);
					face.n[3] = face.n[2];

					el.SetType(FE_TRI3);
					el.m_node[0] = face.n[0];
					el.m_node[1] = face.n[1];
					el.m_node[2] = face.n[2];
					el.m_node[3] = el.m_node[2];
				}
				break;
				case 4:
				{
					face.SetType(FE_FACE_QUAD4);
					face.n[0] = N0(sel[0]);
					face.n[1] = N0(sel[1]);
					face.n[2] = N0(sel[2]);
					face.n[3] = N0(sel[3]);

					el.SetType(FE_QUAD4);
					for (int k = 0; k < 4; ++k) el.m_node[k] = face.n[k];
				}
				break;
				default:
					break;
				}
			}
			else
			{
				switch (np) {
				case 6:
				{
					face.SetType(FE_FACE_TRI6);
					face.n[0] = N0(sel[0]);
					face.n[1] = N0(sel[1]);
					face.n[2] = N0(sel[2]);
					face.n[3] = N0(sel[5]);
					face.n[4] = N0(sel[3]);
					face.n[5] = N0(sel[4]);

					el.SetType(FE_TRI6);
					for (int k = 0; k < 6; ++k) el.m_node[k] = face.n[k];
				}
				break;
				case 8:
				{
					face.SetType(FE_FACE_QUAD8);
					face.n[0] = N0(sel[0]);
					face.n[1] = N0(sel[1]);
					face.n[2] = N0(sel[2]);
					face.n[3] = N0(sel[3]);
					face.n[4] = N0(sel[4]);
					face.n[5] = N0(sel[7]);
					face.n[6] = N0(sel[5]);
					face.n[7] = N0(sel[6]);

					el.SetType(FE_QUAD8);
					for (int k = 0; k < 8; ++k) el.m_node[k] = face.n[k];
				}
				break;
				default:
					break;
				}
			}
		}
	}

	// update the element neighbours
	mesh->BuildMesh();

	return mesh;
#else
	return nullptr;
#endif
}
