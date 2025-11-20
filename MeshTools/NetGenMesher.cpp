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

FSMesh* NetGenMesher::NGMeshToFEMesh(GObject* po, netgen::Mesh* ngmesh, bool secondOrder)
{
#ifdef HAS_NETGEN
	using namespace netgen;

	int nodes = ngmesh->GetNP();
	int elems = ngmesh->GetNE();
	int faces = ngmesh->GetNSE();

	FSMesh* mesh = new FSMesh();

	// solid mesh
	if (elems) {
		mesh->Create(nodes, elems, faces);

		// copy nodes
		PointIndex pi;
		int i = 0;
		for (pi = PointIndex::BASE;
			pi < ngmesh->GetNP() + PointIndex::BASE; pi++, ++i)
		{
			double x = (*ngmesh)[pi](0);
			double y = (*ngmesh)[pi](1);
			double z = (*ngmesh)[pi](2);

			FSNode& ni = mesh->Node(i);
			ni.r = vec3d(x, y, z);
		}

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

		// copy facets
		SurfaceElementIndex sei;
		i = 0;
		for (sei = 0; sei < faces; sei++, ++i)
		{
			Element2d sel = (*ngmesh)[sei];
			//		if (invertsurf) sel.Invert();

			FSFace& face = mesh->Face(i);
			face.m_gid = ngmesh->GetFaceDescriptor(sel.GetIndex()).SurfNr() - 1;

			if (secondOrder == false)
			{
				face.SetType(FE_FACE_TRI3);
				face.n[0] = sel[0] - 1;
				face.n[1] = sel[1] - 1;
				face.n[2] = sel[2] - 1;
				face.n[3] = face.n[2];
			}
			else
			{
				int np = sel.GetNP();
				face.SetType(FE_FACE_TRI6);
				face.n[0] = sel[0] - 1;
				face.n[1] = sel[1] - 1;
				face.n[2] = sel[2] - 1;
				face.n[3] = sel[5] - 1;
				face.n[4] = sel[3] - 1;
				face.n[5] = sel[4] - 1;
			}
		}

		// copy elements
		i = 0;
		for (ElementIndex ei = 0; ei < ngmesh->GetNE(); ei++, ++i)
		{
			Element ngel = (*ngmesh)[ei];

			// Note that I need to invert the element
			FSElement& el = mesh->Element(i);
			el.m_gid = 0;

			if (secondOrder == false)
			{
				el.SetType(FE_TET4);
				el.m_node[0] = ngel[2] - 1;
				el.m_node[1] = ngel[1] - 1;
				el.m_node[2] = ngel[0] - 1;
				el.m_node[3] = ngel[3] - 1;
			}
			else
			{
				int np = ngel.GetNP();
				el.SetType(FE_TET10);
				el.m_node[0] = ngel[0] - 1;
				el.m_node[1] = ngel[2] - 1;
				el.m_node[2] = ngel[1] - 1;
				el.m_node[3] = ngel[3] - 1;
				el.m_node[4] = ngel[5] - 1;
				el.m_node[5] = ngel[7] - 1;
				el.m_node[6] = ngel[4] - 1;
				el.m_node[7] = ngel[6] - 1;
				el.m_node[8] = ngel[9] - 1;
				el.m_node[9] = ngel[8] - 1;
			}
		}
	}
	// shell mesh
	else {
		mesh->Create(nodes, faces, faces);

		// copy nodes
		PointIndex pi;
		int i = 0;
		for (pi = PointIndex::BASE;
			pi < ngmesh->GetNP() + PointIndex::BASE; pi++, ++i)
		{
			double x = (*ngmesh)[pi](0);
			double y = (*ngmesh)[pi](1);
			double z = (*ngmesh)[pi](2);

			FSNode& ni = mesh->Node(i);
			ni.r = vec3d(x, y, z);
		}

		// copy facets
		SurfaceElementIndex sei;
		i = 0;
		for (sei = 0; sei < faces; sei++, ++i)
		{
			Element2d sel = (*ngmesh)[sei];
			//        if (invertsurf) sel.Invert();

			FSFace& face = mesh->Face(i);
			face.m_gid = ngmesh->GetFaceDescriptor(sel.GetIndex()).SurfNr() - 1;

			int np = sel.GetNP();

			if (secondOrder == false)
			{
				switch (np) {
				case 3:
				{
					face.SetType(FE_FACE_TRI3);
					face.n[0] = sel[0] - 1;
					face.n[1] = sel[1] - 1;
					face.n[2] = sel[2] - 1;
					face.n[3] = face.n[2];
				}
				break;
				case 4:
				{
					face.SetType(FE_FACE_QUAD4);
					face.n[0] = sel[0] - 1;
					face.n[1] = sel[1] - 1;
					face.n[2] = sel[2] - 1;
					face.n[3] = sel[3] - 1;
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
					face.n[0] = sel[0] - 1;
					face.n[1] = sel[1] - 1;
					face.n[2] = sel[2] - 1;
					face.n[3] = sel[5] - 1;
					face.n[4] = sel[3] - 1;
					face.n[5] = sel[4] - 1;
				}
				break;
				case 8:
				{
					face.SetType(FE_FACE_QUAD8);
					face.n[0] = sel[0] - 1;
					face.n[1] = sel[1] - 1;
					face.n[2] = sel[2] - 1;
					face.n[3] = sel[3] - 1;
					face.n[4] = sel[4] - 1;
					face.n[5] = sel[7] - 1;
					face.n[6] = sel[5] - 1;
					face.n[7] = sel[6] - 1;
				}
				break;
				default:
					break;
				}
			}
		}

		// copy elements
		i = 0;
		for (sei = 0; sei < faces; sei++, ++i)
		{
			Element2d sel = (*ngmesh)[sei];

			// Note that I need to invert the element
			FSElement& el = mesh->Element(i);
			el.m_gid = 0;

			int np = sel.GetNP();

			if (secondOrder == false)
			{
				switch (np) {
				case 3:
				{
					el.SetType(FE_TRI3);
					el.m_node[0] = sel[0] - 1;
					el.m_node[1] = sel[1] - 1;
					el.m_node[2] = sel[2] - 1;
					el.m_node[3] = el.m_node[2];
				}
				break;
				case 4:
				{
					el.SetType(FE_QUAD4);
					el.m_node[0] = sel[0] - 1;
					el.m_node[1] = sel[1] - 1;
					el.m_node[2] = sel[2] - 1;
					el.m_node[3] = sel[3] - 1;
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
					el.SetType(FE_TRI6);
					el.m_node[0] = sel[0] - 1;
					el.m_node[1] = sel[1] - 1;
					el.m_node[2] = sel[2] - 1;
					el.m_node[3] = sel[5] - 1;
					el.m_node[4] = sel[3] - 1;
					el.m_node[5] = sel[4] - 1;
				}
				break;
				case 8:
				{
					el.SetType(FE_QUAD8);
					el.m_node[0] = sel[0] - 1;
					el.m_node[1] = sel[1] - 1;
					el.m_node[2] = sel[2] - 1;
					el.m_node[3] = sel[3] - 1;
					el.m_node[4] = sel[4] - 1;
					el.m_node[5] = sel[7] - 1;
					el.m_node[6] = sel[5] - 1;
					el.m_node[7] = sel[6] - 1;
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

	// associate the FE nodes with the GNodes
/*	double R2 = mesh->GetBoundingBox().GetMaxExtent();
	if (R2 == 0) R2 = 1.0; else R2 *= R2;
	for (i = 0; i<mesh->Nodes(); ++i)
	{
		FSNode& node = mesh->Node(i);
		vec3d& ri = node.r;
		node.m_gid = -1;
		for (j = 0; j<m_occ->Nodes(); ++j)
		{
			GNode& gn = *m_po->Node(j);
			vec3d& rj = gn.LocalPosition();
			double L2 = (ri - rj).SqrLength();
			if (L2 / R2 < 1e-6)
			{
				node.m_gid = j;
				break;
			}
		}
	}
*/
	return mesh;
#else
	return nullptr;
#endif
}
