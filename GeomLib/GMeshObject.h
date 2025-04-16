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

//-----------------------------------------------------------------------------
class FSSurfaceMesh;

//-----------------------------------------------------------------------------
// The GMeshObject defines the geometry of a "naked" FE mesh. That is, a mesh
// that does not correspond to a geometry object. There are two ways to create
// GMeshObjects. The first is to import a FE model from a file. Most FE models imported 
// are considered naked. The second is by converting a geometry object that has a mesh
// to a GMeshObject. 
// 
// This object defines the geometry (such as parts, faces, 
// edges, nodes) using this FE geometry. This has one major limitation and that
// is that when the user changes the mesh, it can not be assumed that the geometry
// stays the same and therefore all geometry dependant quantities (e.g. boundary 
// conditions) need to be removed. The GMeshObject is the only object for which
// the user can define their own partitions.

class GMeshObject : public GObject
{
public:
	// Constructor for creating a GMeshObject from a "naked" mesh
	GMeshObject(FSMesh* pm);

	// Constructor for creating a GMeshObject from a "naked" surface mesh 
	// (this creates a shell mesh)
	GMeshObject(FSSurfaceMesh* pm);

	// Constructor for converting a GObject to a GMeshObject
	GMeshObject(GObject* po);

	// update geometry information
	bool Update(bool b = true) override;

	int MakeGNode(int n);

	int AddNode(vec3d r);

	virtual FSMesh* BuildMesh() override;

	// serialization
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	// cloning
	GObject* Clone() override;

	FSMeshBase* GetEditableMesh() override;
	FSLineMesh* GetEditableLineMesh() override;

	// detach an element selection
	GMeshObject* DetachSelection();

public:
	void Attach(GObject* po, bool bweld, double tol);

	bool DeletePart(GPart* pg) override;
	bool DeleteParts(std::vector<GPart*> pg);

protected:
	void BuildGMesh() override;

protected:
	void UpdateParts();
	void UpdateSurfaces();
	void UpdateEdges();
	void UpdateNodes();
	void UpdateFaceNodes();
	virtual void UpdateSections(); // TODO: made this virtual so I can override it in PostObject. Probably need to find better solution.
};

GMeshObject* ExtractSelection(GObject* po);

GMeshObject* ConvertToEditableMesh(GObject* po);
