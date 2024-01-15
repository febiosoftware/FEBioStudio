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

#include "stdafx.h"
#include "NetGenMesher.h"
#include <GeomLib/GOCCObject.h>
#include <MeshLib/FEMesh.h>

// NOTE: Can't build with Netgen in debug config, so just turning it off for now. 
#if defined(WIN32) && defined(_DEBUG)
#undef HAS_NETGEN
#endif

#ifdef HAS_NETGEN

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

#define OCCGEOMETRY

//using namespace std;

namespace nglib {
#include <nglib.h>
}

#define NO_PARALLEL_THREADS
#include <occgeom.hpp>

#endif

#ifdef HAS_NETGEN
FSMesh* NGMeshToFEMesh(netgen::Mesh* ng, bool secondOrder);
#endif

NetGenMesher::NetGenMesher() : m_occ(nullptr)
{
}

NetGenMesher::NetGenMesher(GOCCObject* po) : m_occ(po)
{
	AddChoiceParam(2, "Mesh granularity")->SetEnumNames("Very coarse\0Coarse\0Moderate\0Fine\0");
	AddBoolParam(true, "Use local mesh modifiers");
	AddDoubleParam(0, "Grading (0 = use default)");
	AddDoubleParam(1000.0, "Max element size");
	AddIntParam(3, "Nr. 2D optimization steps");
	AddIntParam(5, "Nr. 3D optimization steps");
	AddBoolParam(false, "Second-order tets");
}

namespace netgen
{
#ifdef HAS_NETGEN
	DLL_HEADER extern MeshingParameters mparam;
#endif
}

FSMesh*	NetGenMesher::BuildMesh()
{
#ifdef HAS_NETGEN
	using namespace nglib;
	using namespace netgen;
	//using namespace std;

	// Initialise the Netgen Core library
	cout << "Calling Netgen" << std::endl;
	Ng_Init();

	multithread.terminate = 0;

	int gran = GetIntValue(NetGenMesher::GRANULARITY);

	const double resthcloseedgefac_list[] = { 0.5, 1, 2, 3.5, 5 };
	const double resthminedgelen_list[] = {0.002, 0.02, 0.2, 1.0, 2.0};

	// set meshing parameters
	// (mparam is a global parameter defined in nglib)
	// NOTE: Do this before creating the OCCGeometry
	mparam.uselocalh = (GetBoolValue(NetGenMesher::USELOCALH) ? 1 : 0);
	mparam.maxh = GetFloatValue(NetGenMesher::MAXELEMSIZE);
	mparam.grading = GetFloatValue(NetGenMesher::GRADING);
	mparam.optsteps2d = GetIntValue(NetGenMesher::NROPT2D);
	mparam.optsteps3d = GetIntValue(NetGenMesher::NROPT3D);
	mparam.secondorder = (GetBoolValue(NetGenMesher::SECONDORDER) ? 1 : 0);
	mparam.checkoverlappingboundary = 0;
    mparam.perfstepsstart = 0;
	mparam.giveuptol = 25;
	mparam.sloppy = 5;

	// make sure grading is not zero
//	if (mparam.grading == 0.0) mparam.grading = 0.001;
    
    switch (gran) {
        case 0:
            mparam.grading = (mparam.grading == 0) ? 0.7 : mparam.grading;
            mparam.segmentsperedge = 0.3;
            mparam.curvaturesafety = 1.0;
            break;
            
        case 1:
            mparam.grading = (mparam.grading == 0) ? 0.5 : mparam.grading;
            mparam.segmentsperedge = 0.5;
            mparam.curvaturesafety = 1.5;
            break;
            
        case 2:
            mparam.grading = (mparam.grading == 0) ? 0.3 : mparam.grading;
            mparam.segmentsperedge = 1.0;
            mparam.curvaturesafety = 2.0;
            break;
            
        case 3:
            mparam.grading = (mparam.grading == 0) ? 0.2 : mparam.grading;
            mparam.segmentsperedge = 2.0;
            mparam.curvaturesafety = 3.0;
            break;
            
        default:
            mparam.grading = (mparam.grading == 0) ? 0.3 : mparam.grading;
            mparam.segmentsperedge = 1.0;
            mparam.curvaturesafety = 2.0;
            break;
    }

    // create new occ geometry object
    shared_ptr<OCCGeometry> occgeo = make_shared<OCCGeometry>();

#if defined(WIN32) || defined(__APPLE__)
    // set OCC parameters
	//

	occparam.resthcloseedgefac = resthcloseedgefac_list[gran];
	occparam.resthcloseedgeenable = 1;
	occparam.resthminedgelen = resthminedgelen_list[gran];
	occparam.resthminedgelenenable = 1;
#else
    // set OCC parameters
	//
    OCCParameters par;
    mparam.closeedgefac = resthcloseedgefac_list[gran];
    par.resthminedgelen = resthminedgelen_list[gran];
    par.resthminedgelenenable = 1;

    occgeo->SetOCCParameters(par);
#endif

	occgeo->shape = m_occ->GetShape();
	occgeo->changed = 1;
	occgeo->BuildFMap();
	occgeo->CalcBoundingBox();

	// Generate the mesh
	shared_ptr<Mesh> ngmesh;
	try {
		occgeo->GenerateMesh(ngmesh, mparam);
	}
	catch (...)
	{
		Ng_Exit();
		return 0;
	}

	if (mparam.secondorder)
	{
		occgeo->GetRefinement().MakeSecondOrder(*ngmesh.get());
	}

	// Build the FE mesh
	FSMesh* mesh = NGMeshToFEMesh(ngmesh.get(), GetBoolValue(SECONDORDER));

	Ng_Exit();

	return mesh;

#else
	return nullptr;
#endif // HAS_NETGEN
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
FSMesh* NGMeshToFEMesh(netgen::Mesh* ngmesh, bool secondOrder)
{
	using namespace netgen;

	int nodes = ngmesh->GetNP();
	int elems = ngmesh->GetNE();
	int faces = ngmesh->GetNSE();

	FSMesh* mesh = new FSMesh();
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

	// copy facets
	SurfaceElementIndex sei;
	i = 0;
	for (sei = 0; sei < faces; sei++, ++i)
	{
		Element2d sel = (*ngmesh)[sei];
//		if (invertsurf) sel.Invert();

		FSFace& face = mesh->Face(i);
		face.m_gid = ngmesh->GetFaceDescriptor(sel.GetIndex()).SurfNr() - 1;
		face.m_sid = face.m_gid;

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
}
#endif
