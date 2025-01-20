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
#include "NetGenOCCMesher.h"
#include <GeomLib/GOCCObject.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSSurfaceMesh.h>
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

#include <FSCore/ClassDescriptor.h>

REGISTER_CLASS3(NetGenOCCMesher, CLASS_MESHER, NetGen_OCC_Mesher, "ng_occ_mesher", 0, 0);


NetGenOCCMesher::NetGenOCCMesher() : m_occ(nullptr)
{
	SetType(NetGen_OCC_Mesher);
}

NetGenOCCMesher::NetGenOCCMesher(GOCCObject* po) : m_occ(po)
{
	SetType(NetGen_OCC_Mesher);
}

namespace netgen
{
#ifdef HAS_NETGEN
#endif
}

#ifdef HAS_NETGEN
using namespace nglib;
using namespace netgen;

// helper class for wrapping Ng_Init and Ng_Exit
class NGHelper
{
public:
	NGHelper() { Ng_Init(); }
	~NGHelper() { Ng_Exit(); }
};
#endif

FSMesh*	NetGenOCCMesher::BuildMesh()
{
#ifdef HAS_NETGEN
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
	NGHelper ng;
    
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
    
    const double resthcloseedgefac_list[] = { 0.5, 1, 2, 3.5, 5 };
    const double resthminedgelen_list[] = {0.002, 0.02, 0.2, 1.0, 2.0};
    
    // set meshing parameters
    // (mp is a global parameter defined in nglib)
    // NOTE: Do this before creating the OCCGeometry
    mp.uselocalh = (GetBoolValue(NetGenOCCMesher::USELOCALH) ? 1 : 0);
    mp.maxh = GetFloatValue(NetGenOCCMesher::MAXELEMSIZE);
    mp.minh = GetFloatValue(NetGenOCCMesher::MINELEMSIZE);
    mp.grading = GetFloatValue(NetGenOCCMesher::GRADING);
	mp.elementsperedge = GetFloatValue(NetGenOCCMesher::ELEMPEREDGE);
	mp.elementspercurve = GetFloatValue(NetGenOCCMesher::ELEMPERCURV);
    mp.optsteps_2d = GetIntValue(NetGenOCCMesher::NROPT2D);
    mp.optsteps_3d = GetIntValue(NetGenOCCMesher::NROPT3D);
    mp.second_order = (GetBoolValue(NetGenOCCMesher::SECONDORDER) ? 1 : 0);
    mp.check_overlapping_boundary = 0;
    mp.closeedgeenable = 0;
    mp.closeedgefact = 1.0;
    mp.optsurfmeshenable = 1;
    mp.quad_dominated = 0;
    if (m_occ->GetShape().ShapeType() == TopAbs_SHELL) mp.quad_dominated = GetBoolValue(NetGenOCCMesher::QUADMESH) ? 1 : 0;

	// some validation
	if ((mp.grading <= 0) || (mp.grading >= 1))
	{
		SetErrorMessage("Grading parameter should be between 0 and 1.");
		return nullptr;
	}

    
    // check if user has selected surface mesh refinement option
    if (GetBoolValue(NetGenOCCMesher::SURFREFINE)) {
        
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
        
		try {
			Ng_GenerateVolumeMesh(occ_mesh, &mp);
		}
		catch (...)
		{
			setErrorString("Error creating volume mesh..... aborting!!");
			return nullptr;
		}
        
		FSLogger::Write("Volume Mesh successfully created.....\n");
		FSLogger::Write("Number of points = %d\n", Ng_GetNP(occ_mesh));
		FSLogger::Write("Number of volume elements = %d\n", Ng_GetNE(occ_mesh));
    }
    
    if (mp.second_order)
    {
		try {
			Ng_OCC_Generate_SecondOrder(occ_geom, occ_mesh);
		}
		catch (...)
		{
			return nullptr;
		}
    }

	FSMesh* mesh = NGMeshToFEMesh(m_occ, (Mesh*)occ_mesh, GetBoolValue(SECONDORDER));
    return mesh;
    
#else
	SetErrorMessage("This version of FEBio Studio was not built with NetGen support.");
    return nullptr;
#endif // HAS_NETGEN
}

void NetGenOCCMesher::SetMeshSize(int faceId, double v)
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
