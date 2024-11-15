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
#include <MeshLib/FESurfaceMesh.h>
#include <FSCore/FSLogger.h>

// NOTE: Can't build with Netgen in debug config, so just turning it off for now. 
#if defined(WIN32) && defined(_DEBUG)
#undef HAS_NETGEN
#endif

#ifdef HAS_NETGEN

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

#define OCCGEOMETRY

//using namespace std;

namespace nglib {
#include <nglib.h>
#include <nglib_occ.h>
}

#define NO_PARALLEL_THREADS
#include <occgeom.hpp>

#endif

#ifdef HAS_NETGEN
FSMesh* NGMeshToFEMesh(GObject* po, netgen::Mesh* ng, bool secondOrder);
#endif

NetGenMesher::NetGenMesher() : m_occ(nullptr)
{
}

NetGenMesher::NetGenMesher(GOCCObject* po) : m_occ(po)
{
	AddChoiceParam(2, "Mesh granularity")->SetEnumNames("Very coarse\0Coarse\0Moderate\0Fine\0Very fine\0User-defined\0");
	AddBoolParam(true, "Use local mesh modifiers");
	AddDoubleParam(0, "Grading (0 = use default)");
	AddDoubleParam(1000.0, "Max element size");
    AddDoubleParam(1.0, "Min element size");
	AddIntParam(3, "Nr. 2D optimization steps");
	AddIntParam(5, "Nr. 3D optimization steps");
	AddBoolParam(false, "Second-order mesh");
    AddDoubleParam(1.0, "Elements per edge");
    AddDoubleParam(2.0, "Elements per curve");
    AddBoolParam(true, "Quad dominant shell mesh");
    AddBoolParam(false, "Surface mesh refinement");
//    AddDoubleParam(0.0, "Surface mesh size");
}

namespace netgen
{
#ifdef HAS_NETGEN
#endif
}

FSMesh*	NetGenMesher::BuildMesh()
{
#ifdef HAS_NETGEN
    using namespace nglib;
    using namespace netgen;
    //using namespace std;
    
    // Define pointer to OCC Geometry
    Ng_OCC_Geometry *occ_geom;
    
    Ng_Mesh *occ_mesh;
    
    Ng_Meshing_Parameters mp;
    
    TopTools_IndexedMapOfShape FMap;
    
    Ng_OCC_TopTools_IndexedMapOfShape *occ_fmap = (Ng_OCC_TopTools_IndexedMapOfShape*)&FMap;
    
    // Result of Netgen Operations
    Ng_Result ng_res;
    
    // Initialise the Netgen Core library
    FSLogger::Write("Calling Netgen\n");
    Ng_Init();
    
    // Read in the OCC File
    OCCGeometry * occgeo = new OCCGeometry(m_occ->GetShape());
/*
    occgeo->tolerance = 1e-3;
    occgeo->fixsmalledges = true;
    occgeo->fixspotstripfaces = true;
    occgeo->sewfaces = false;
    occgeo->makesolids = false;
    occgeo->splitpartitions = false;

    occgeo->HealGeometry();
*/
    occ_geom = (Ng_OCC_Geometry *)occgeo;
    if (!occ_geom)
    {
        setErrorString("Error converting geometry " + m_occ->GetName());
        Ng_Exit();
        return nullptr;
    }
	FSLogger::Write("Successfully converted geometry %s\n", m_occ->GetName().c_str());

    // check to see if any edge has near-zero length
    // Evaluate the minimum and max edge length to determin min and max element sizes
    double mnedg = 0.0;
    double mxedg = 0.0;
    TopoDS_Shape& occ = m_occ->GetShape();
    TopExp_Explorer anExp (occ, TopAbs_EDGE);
    for (; anExp.More(); anExp.Next()) {
        const TopoDS_Edge& anEdge = TopoDS::Edge (anExp.Current());
        GProp_GProps props;
        BRepGProp::LinearProperties(anEdge, props);
        double length = props.Mass();
        if (mnedg == 0) mnedg = length;
        else {
            mnedg = fmin(mnedg, length);
            mxedg = fmax(mxedg, length);
        }
    }
    FSLogger::Write("Minimum edge length in this object is %g.\n", mnedg);
    FSLogger::Write("Maximum edge length in this object is %g.\n", mxedg);

    multithread.terminate = 0;
    
    occ_mesh = Ng_NewMesh();
    
    ng_res = Ng_OCC_GetFMap(occ_geom,occ_fmap);
    
	FSLogger::Write("ng_res = %d\n", ng_res);
    
    if(!FMap.Extent())
    {
        setErrorString("Error retrieving Face map....");
        Ng_Exit();
        return nullptr;
    }
    
	FSLogger::Write("Successfully extracted the Face Map....: %d\n", FMap.Extent());
    
    for(int i = 1; i <= FMap.Extent(); i++)
    {
        TopoDS_Face OCCface;
        OCCface = TopoDS::Face(FMap.FindKey(i));
        
        GProp_GProps faceProps;
        BRepGProp::SurfaceProperties(OCCface,faceProps);
        

		FSLogger::Write("Index: %d :: Area: %lg\n", i, faceProps.Mass());
    }
    
    int gran = GetIntValue(NetGenMesher::GRANULARITY);
    
    const double resthcloseedgefac_list[] = { 0.5, 1, 2, 3.5, 5 };
    const double resthminedgelen_list[] = {0.002, 0.02, 0.2, 1.0, 2.0};
    
    // set meshing parameters
    // (mp is a global parameter defined in nglib)
    // NOTE: Do this before creating the OCCGeometry
    mp.uselocalh = (GetBoolValue(NetGenMesher::USELOCALH) ? 1 : 0);
    mp.maxh = GetFloatValue(NetGenMesher::MAXELEMSIZE);
    mp.minh = GetFloatValue(NetGenMesher::MINELEMSIZE);
    mp.grading = GetFloatValue(NetGenMesher::GRADING);
    mp.optsteps_2d = GetIntValue(NetGenMesher::NROPT2D);
    mp.optsteps_3d = GetIntValue(NetGenMesher::NROPT3D);
    mp.second_order = (GetBoolValue(NetGenMesher::SECONDORDER) ? 1 : 0);
    mp.check_overlapping_boundary = 0;
    mp.closeedgeenable = 0;
    mp.closeedgefact = 1.0;
    mp.optsurfmeshenable = 1;
    mp.quad_dominated = 0;
    if (m_occ->GetShape().ShapeType() == TopAbs_SHELL) mp.quad_dominated = GetBoolValue(NetGenMesher::QUADMESH) ? 1 : 0;
    
    switch (gran) {
        case 0:
            mp.grading = (mp.grading == 0) ? 0.7 : mp.grading;
            mp.elementsperedge = 0.3;
            mp.elementspercurve = 1.0;
            break;
            
        case 1:
            mp.grading = (mp.grading == 0) ? 0.5 : mp.grading;
            mp.elementsperedge = 0.5;
            mp.elementspercurve = 1.5;
            break;
            
        case 2:
            mp.grading = (mp.grading == 0) ? 0.3 : mp.grading;
            mp.elementsperedge = 1.0;
            mp.elementspercurve = 2.0;
            break;
            
        case 3:
            mp.grading = (mp.grading == 0) ? 0.2 : mp.grading;
            mp.elementsperedge = 2.0;
            mp.elementspercurve = 3.0;
            break;
            
        case 4:
            mp.grading = (mp.grading == 0) ? 0.1 : mp.grading;
            mp.elementsperedge = 3.0;
            mp.elementspercurve = 5.0;
            break;
            
        case 5:
            mp.grading = (mp.grading == 0) ? 0.1 : mp.grading;
            mp.elementsperedge = GetFloatValue(NetGenMesher::ELEMPEREDGE);
            mp.elementspercurve = GetFloatValue(NetGenMesher::ELEMPERCURV);;
            break;
            
        default:
            mp.grading = (mp.grading == 0) ? 0.3 : mp.grading;
            mp.elementsperedge = 1.0;
            mp.elementspercurve = 2.0;
            break;
    }

    // check if user has selected surface mesh refinement option
    if (GetBoolValue(NetGenMesher::SURFREFINE)) {
        
        // Update the face mesh sizes to reflect the global maximum mesh size
        MeshingParameters* mparam = new MeshingParameters();
        mparam->uselocalh = mp.uselocalh;
        
        mparam->maxh = mp.maxh;
        mparam->minh = mp.minh;
        
        mparam->grading = mp.grading;
        mparam->curvaturesafety = mp.elementspercurve;
        mparam->segmentsperedge = mp.elementsperedge;
        
        mparam->secondorder = mp.second_order;
        mparam->quad = mp.quad_dominated;
        
        if (mp.meshsize_filename)
            mparam->meshsizefilename = mp.meshsize_filename;
        else
            mparam->meshsizefilename = "";
        mparam->optsteps2d = mp.optsteps_2d;
        mparam->optsteps3d = mp.optsteps_3d;
        
        mparam->inverttets = mp.invert_tets;
        mparam->inverttrigs = mp.invert_trigs;
        
        mparam->checkoverlap = mp.check_overlap;
        mparam->checkoverlappingboundary = mp.check_overlapping_boundary;
        
        for(int i = 1; i <= occgeo->NrFaces(); i++)
        {
            if(!occgeo->GetFaceMaxhModified(i))
            {
                occgeo->SetFaceMaxH(i, mp.maxh, *mparam);
            }
        }
        
        // set mesh size of selected surface
        for (MeshSize& msize : m_msize) {
            if (msize.meshSize > 0) occgeo->SetFaceMaxH(msize.faceId + 1, msize.meshSize, *mparam);
        }
    }

	FSLogger::Write("Setting Local Mesh size.....\n");
    Ng_OCC_SetLocalMeshSize(occ_geom, occ_mesh, &mp);
	FSLogger::Write("Local Mesh size successfully set.....\n");
    
	FSLogger::Write("Creating Edge Mesh.....\n");
    ng_res = Ng_OCC_GenerateEdgeMesh(occ_geom, occ_mesh, &mp);
    if(ng_res != NG_OK)
    {
        Ng_DeleteMesh(occ_mesh);
        setErrorString("Error creating Edge Mesh.... Aborting!!");
        Ng_Exit();
        return nullptr;
    }
    else
    {
		FSLogger::Write("Edge Mesh successfully created.....\n");
		FSLogger::Write("Number of points =  %d\n", Ng_GetNP(occ_mesh));
    }
    
	FSLogger::Write("Creating Surface Mesh.....\n");
    
    ng_res = Ng_OCC_GenerateSurfaceMesh(occ_geom, occ_mesh, &mp);
    if(ng_res != NG_OK)
    {
        //       Ng_DeleteMesh(occ_mesh);
        setErrorString("Error creating Surface Mesh..... Aborting!!");
        Ng_Exit();
        return nullptr;
    }
    else
    {
		FSLogger::Write("Surface Mesh successfully created.....\n");
		FSLogger::Write("Number of points = %d\n", Ng_GetNP(occ_mesh));
		FSLogger::Write("Number of surface elements = %d\n", Ng_GetNSE(occ_mesh));
    }
    
    if ((m_occ->GetShape().ShapeType() == TopAbs_SOLID) || (m_occ->GetShape().ShapeType() == TopAbs_COMPOUND)) {
		FSLogger::Write("Creating Volume Mesh.....\n");
        
        ng_res = Ng_GenerateVolumeMesh(occ_mesh, &mp);
        
		FSLogger::Write("Volume Mesh successfully created.....\n");
		FSLogger::Write("Number of points = %d\n", Ng_GetNP(occ_mesh));
		FSLogger::Write("Number of volume elements = %d\n", Ng_GetNE(occ_mesh));
    }
    
    if (mp.second_order)
    {
        Ng_OCC_Generate_SecondOrder (occ_geom,occ_mesh);
    }
    FSMesh* mesh = NGMeshToFEMesh(m_occ, (Mesh*)occ_mesh, GetBoolValue(SECONDORDER));
    
    Ng_Exit();
    
    return mesh;
    
#else
	SetErrorMessage("This version of FEBio Studio was not built with NetGen support.");
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
FSMesh* NGMeshToFEMesh(GObject* po, netgen::Mesh* ngmesh, bool secondOrder)
{
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
            face.m_sid = face.m_gid;
            
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
}
#endif

void NetGenMesher::SetMeshSize(int faceId, double v)
{
	for (MeshSize& ms : m_msize)
	{
		if (ms.faceId == faceId)
		{
			ms.meshSize = v;
			return;
		}
	}
	m_msize.push_back({ faceId, v });
}
