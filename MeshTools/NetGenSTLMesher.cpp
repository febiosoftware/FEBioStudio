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
#include "NetGenSTLMesher.h"
#include <GeomLib/GSurfaceMeshObject.h>

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
using namespace netgen;
#endif

#include <FSCore/ClassDescriptor.h>

REGISTER_CLASS3(NetGenSTLMesher, CLASS_MESHER, NetGen_STL_Mesher, "ng_stl_mesher", 0, 0);

NetGenSTLMesher::NetGenSTLMesher() 
{
	m_pso = nullptr;
	SetType(NetGen_STL_Mesher);
}

FSMesh* NetGenSTLMesher::BuildMesh(GObject* po)
{
#ifdef HAS_NETGEN
	m_pso = dynamic_cast<GSurfaceMeshObject*>(po);
	if (m_pso == nullptr) return nullptr;

	FSSurfaceMesh* surfMesh = m_pso->GetSurfaceMesh();
	int NN = surfMesh->Nodes();
	if (NN == 0) return nullptr;
	int NF = surfMesh->Faces();
	if (NF == 0) return nullptr;

	Ng_Init();

	// create the NG mesh
	Ng_Mesh* ngmesh = Ng_NewMesh();

	// Add all the point to the mesh
	double v[3];
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = surfMesh->Node(i);
		vec3d r = node.r;
		v[0] = r.x; v[1] = r.y; v[2] = r.z;
		Ng_AddPoint(ngmesh, v);
	}

	// Add face descriptors.
	// Note that the mesh will already have one face descriptor added by default,
	// so we only add additional ones if necessary. 
	// Also note that surface numbers are one-based
	netgen::Mesh& mesh = *((netgen::Mesh*)ngmesh);
	for (int i = 1; i < m_pso->Faces(); ++i)
	{
		FaceDescriptor fd;
		fd.SetSurfNr(i + 1);
		fd.SetDomainIn(1);
		mesh.AddFaceDescriptor(fd);
	}

	// Add all the surface elements
	int p[3];
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = surfMesh->Face(i);
		p[0] = face.n[0] + 1;
		p[1] = face.n[1] + 1;
		p[2] = face.n[2] + 1;
		Ng_AddSurfaceElement(ngmesh, NG_TRIG, p);
	}

	// Set the surface element indices to the correct (one-based) surface number
	for (SurfaceElementIndex sei = 0; sei < NF; ++sei)
	{
		FSFace& face = surfMesh->Face(sei);
		Element2d& sel = mesh[sei];
		sel.SetIndex(face.m_gid + 1);
	}

	// set meshing parameters
	// set meshing parameters
	// (mp is a global parameter defined in nglib)
	// NOTE: Do this before creating the OCCGeometry
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
	mp.quad_dominated = 0;

	// Generate volume mesh
	Ng_Result ret = Ng_GenerateVolumeMesh(ngmesh, &mp);
	if (ret != NG_OK) {
		Ng_DeleteMesh(ngmesh);
		Ng_Exit();
		return nullptr;
	}

	FSMesh* femesh = NGMeshToFEMesh(m_pso, (Mesh*)ngmesh, GetBoolValue(SECONDORDER));

	Ng_DeleteMesh(ngmesh);
	Ng_Exit();

	return femesh;
#else
	SetErrorMessage("This version of FEBio Studio was not built with NetGen support.");
	return nullptr;
#endif
}
