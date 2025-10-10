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

#pragma once
#include "GObject.h"
#include <MeshLib/FSSurfaceMesh.h>

class tetgenio;
class GOCCObject;

class GSurfaceMeshObject : public GObject
{
public:
	// create a new surface mesh object
	GSurfaceMeshObject(FSSurfaceMesh* pm = 0);

	// create a new surface mesh object from a (meshed) object
	// this extracts the surface from the object
	GSurfaceMeshObject(GObject* po);

	// build the mesh
	FSMesh* BuildMesh() override;

	// update mesh for rendering
	void BuildGMesh() override;

	// update data structures
	void Update();

	// clone function
	GObject* Clone() override;

	// default mesher
	FEMesher* CreateDefaultMesher() override;

	// return the surface mesh
	FSSurfaceMesh* GetSurfaceMesh();
	const FSSurfaceMesh* GetSurfaceMesh() const;

	FSMeshBase* GetEditableMesh() override { return GetSurfaceMesh(); }
	FSLineMesh* GetEditableLineMesh() override { return GetSurfaceMesh(); }

	// get the mesh of an edge curve
	FSCurveMesh* GetFECurveMesh(int edgeId) override;

	void ReplaceSurfaceMesh(FSSurfaceMesh* newMesh) override;

	// serialization
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	// attach another surface mesh object
	void Attach(const GSurfaceMeshObject* po, bool weld, double weldTolerance);

	void UpdateSurfaceMeshData();

	BOX GetLocalBox() const override;

private:
	// Move this elsewhere or refactor
	bool build_tetgen_plc(FSMesh* pm, tetgenio& in);

private:
	void UpdateEdges();
	void UpdateNodes();
	void UpdateSurfaces();

private:
	FSSurfaceMesh*	m_surfmesh;
};

// Helper function for converting an object to an editable surface.
// This assumes that the object has a mesh.
GSurfaceMeshObject* ConvertToEditableSurface(GObject* po);
