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
#include "FEModifier.h"
#include "FEExtrudeFaces.h"
#include "FEDiscardMesh.h"
#include "FEFillHole.h"
#include "FEMMGRemesh.h"
#include "FETetGenModifier.h"
#include "FEAutoPartition.h"
#include <MeshLib/FEMeshBuilder.h>
#include <MeshLib/FENodeNodeList.h>
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/MeshTools.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
using namespace std;

FEInflateMesh::FEInflateMesh() : FEModifier("Inflate")
{
	AddDoubleParam(1, "distance");
	AddIntParam(1, "segments");
	AddDoubleParam(1, "mesh bias");
	AddBoolParam(false, "symmetric mesh bias");
    AddDoubleParam(1e-6, "weld tolerance");
    AddDoubleParam(70, "crease angle");
}

FSMesh* FEInflateMesh::Apply(FSMesh* pm)
{
    // Keep track of the selected surface
    FSMesh* hollow = pm->ExtractFaces(true);
	if (hollow == nullptr) {
		SetError("Failed to extract surface mesh."); return nullptr;
	}

    int nnode = hollow->Nodes();
    for (int i=0; i < hollow->Faces(); ++i) hollow->Face(i).Select();
    
    // Apply extrusion to create biased mesh
    double d = -fabs(GetFloatValue(0));
    int nseg = GetIntValue(1);
    double rbias = GetFloatValue(2);
    bool symmBias = GetBoolValue(3);
    double tol = GetFloatValue(4);
    double angle = GetFloatValue(5);
    FEExtrudeFaces extrude;
    extrude.SetExtrusionDistance(d);
    extrude.SetUseNormalLocal(true);
    extrude.SetSegments(nseg);
    extrude.SetMeshBiasFactor(rbias);
    extrude.SetSymmetricBias(symmBias);
    FSMesh* biasedMesh = extrude.Apply(hollow);

    // invert the solid mesh (since it was extruded inward)
    FEInvertMesh invert;
    biasedMesh = invert.Apply(biasedMesh);
    GMeshObject gbm(biasedMesh);
    // delete original shell elements
    gbm.DeletePart(gbm.Part(0));

    // extract the inner surface of this hollow solid
    FSMesh* inner = biasedMesh->ExtractFaces(true);
    inner = invert.Apply(inner);
    FSSurfaceMesh* innerSurf = inner->ExtractFacesAsSurface(false);
    // select all the faces of this mesh
    for (int i=0; i<innerSurf->Faces(); ++i) innerSurf->Face(i).Select();
    // set the element size
    double h = fabs(d);

    // check if this surface is closed
    bool isClosed = MeshTools::IsMeshClosed(*innerSurf);
    
    // if not closed, close it a remesh the faces
    if (!isClosed) {
        FEFillHole fill;
        fill.FillAllHoles(innerSurf);
        // select the newly formed faces
        for (int i=0; i<innerSurf->Faces(); ++i) {
            if (innerSurf->Face(i).IsSelected())
                innerSurf->Face(i).Unselect();
            else
                innerSurf->Face(i).Select();
        }
        // MMG remesh the selected (new) faces
        MMGSurfaceRemesh remesh;
        remesh.SetElementSize(h);
        remesh.SetElementSize(h);
        remesh.SetHausdorf(h/10.);
        remesh.SetGradation(1.3);
        innerSurf = remesh.Apply(innerSurf);
    }
    
    // Use Tetgen to mesh this surface
    FETetGenModifier mytg;
    mytg.SetElementSize(h);
    mytg.SetSplitFaces(false);
    FSMesh* lumen = MeshTools::ConvertSurfaceToMesh(innerSurf);
	if (lumen == nullptr) { SetError("Failed to convert surface to mesh."); return nullptr; }
    lumen = mytg.Apply(lumen);
	if (lumen == nullptr) { SetError("Failed to create tetmesh."); return nullptr; }

    // combine the two domains together into one
    GMeshObject obj(lumen);
    gbm.Attach(&obj, true, tol);
    FSMesh* mymesh = gbm.GetFEMesh();
	if (mymesh == nullptr) { SetError("Object has no mesh."); return nullptr; }
    FERebuildMesh rebuild;
    rebuild.SetRepartition(true);
    rebuild.SetCreaseAngle(angle);
    mymesh = rebuild.Apply(mymesh);
    
    return mymesh;
}
