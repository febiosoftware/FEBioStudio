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
#include "GBaseObject.h"
#include <FSCore/box.h>
#include <vector>
//using namespace std;

//-----------------------------------------------------------------------------
// forward declarations
class FSElemSet;
class FSSurface;
class FSEdgeSet;
class FSNodeSet;
class FSPartSet;
class FECurveMesh;
class FSSurfaceMesh;
class FEMesher;
class FSMesh;
class FSMeshBase;
class FSLineMesh;
class FSNode;
class FSGroup;
class GMesh;
class GObject;

//-----------------------------------------------------------------------------
enum ObjectSaveFlags {
	SAVE_MESH = 1,
	ALL_FLAGS = 0xFF
};

//-----------------------------------------------------------------------------
// some exceptions that may be thrown in case of throuble
class GObjectException
{
public:
	GObjectException(GObject* po, const char* szerr) : m_po(po), m_szerr(szerr){}
	const char* ErrorMsg() { return m_szerr; }

	GObject* GetGObject() const { return m_po; }

protected:
	const char* m_szerr;
	GObject* m_po;
};

//-----------------------------------------------------------------------------
// GObject is the base class for all geometry objects
//
class GObject : public GBaseObject
{
	class Imp;

public:
	// --- C R E A T I O N ---

	// con/de-structor
	GObject(int ntype);
	virtual ~GObject(void);

	// create the object
	virtual void Create() {}

	// create a clone of this object
	virtual GObject* Clone() { return 0; }

	// Copy the geometry from another object
	// This does not copy the mesh from the passed object.
	// If a mesh exists then this mesh is deleted.
	virtual void Copy(GObject* po);

	// return type of Object
	int GetType() const;

	// get/set object color
	GLColor GetColor() const;
	void SetColor(const GLColor& c);

	// collapse the transformation
	void CollapseTransform();

	// serialization
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// set save flag options
	void SetSaveFlags(unsigned int flags);

	// --- M E S H I N G ---

	// set the mesher
	void SetFEMesher(FEMesher* pmesher);

	// retrieve the mesher
	virtual FEMesher* GetFEMesher();

	// create a default mesher
	virtual FEMesher* CreateDefaultMesher();

	// retrieve the FE mesh
	FSMesh* GetFEMesh();
	const FSMesh* GetFEMesh() const;

	// get the editable mesh
	virtual FSMeshBase* GetEditableMesh() { return 0; }

	// get the editable line mesh
	virtual FSLineMesh* GetEditableLineMesh() { return 0; }

	// replace the current mesh
	void ReplaceFEMesh(FSMesh* pm, bool bup = false, bool bdel = false);

	// set the FE mesh
	void SetFEMesh(FSMesh* pm);

	// replace the current surface mesh
	virtual void ReplaceSurfaceMesh(FSSurfaceMesh* newMesh);

	// retrieve an FE nodes from a GNode
	FSNode* GetFENode(int gid);

	// build the FSMesh
	virtual FSMesh* BuildMesh();

	// delete the mesh
	void DeleteFEMesh();

	// update data
	bool Update(bool b =  true);

	// update GNodes (only GMeshObject uses this).
	void UpdateGNodes();

	// update FE mesh
	virtual void UpdateMesh() {}

	// get the mesh of an edge curve
	virtual FECurveMesh* GetFECurveMesh(int edgeId);

	// --- G E O M E T R Y ---

	// assign a material to all parts of this object
	void AssignMaterial(int matid);

	// assign a material to a part
	virtual void AssignMaterial(int partid, int matid);

	// render the geometry of the object (not the FE mesh)
//	virtual void Render(GLCanvas* pc);

	// build the render mesh
	virtual void BuildGMesh();

	// get the render mesh
	GMesh*	GetRenderMesh();

	// get the local bounding box 
	BOX GetLocalBox() const;

	// get the global bounding box
	BOX GetGlobalBox() const;

public:
	// show the object
	void Show();

	// hide the object
	void Hide();

	// show (or hide) a part by index
	void ShowPart(int n, bool bshow);

	// show (or hide) a part by reference
	void ShowPart(GPart& part, bool bshow);

	// show all parts
	void ShowAllParts();

	// update the visibility of items (i.e. surfaces, edges, nodes, and mesh items)
	void UpdateItemVisibility();

	// is called whenever the selection has changed (default does nothing)
	virtual void UpdateSelection();

	virtual bool IsValid() const;

	// check if the object has any dependencies.
	virtual bool CanDelete() const;
	virtual bool CanDeleteMesh() const;

public:
	bool IsFaceVisible(const GFace* pf) const;

protected:
	// set the render mesh
	void SetRenderMesh(GMesh* mesh);

	void SetValidFlag(bool b);

public:
	// --- G R O U P S ---
	int FEPartSets() const;
	int FEElemSets() const;
	int FESurfaces() const;
	int FEEdgeSets() const;
	int FENodeSets() const;

	void AddFEElemSet(FSElemSet* pg);
	void AddFESurface(FSSurface* pg);
	void AddFEEdgeSet(FSEdgeSet* pg);
	void AddFENodeSet(FSNodeSet* pg);
	void AddFEPartSet(FSPartSet* pg);

	FSElemSet* GetFEElemSet(int n);
	FSSurface* GetFESurface(int n);
	FSEdgeSet* GetFEEdgeSet(int n);
	FSNodeSet* GetFENodeSet(int n);
	FSPartSet* GetFEPartSet(int n);

	int RemoveFEElemSet(FSElemSet* pg);
	int RemoveFESurface(FSSurface* pg);
	int RemoveFEEdgeSet(FSEdgeSet* pg);
	int RemoveFENodeSet(FSNodeSet* pg);
	int RemoveFEPartSet(FSPartSet* pg);

	void InsertFEElemSet(int n, FSElemSet* pg);
	void InsertFESurface(int n, FSSurface* pg);
	void InsertFEEdgeSet(int n, FSEdgeSet* pg);
	void InsertFENodeSet(int n, FSNodeSet* pg);
	void InsertFEPartSet(int n, FSPartSet* pg);

	FSGroup* FindFEGroup(int nid);

	FSSurface* FindFESurface(const string& szname);
	FSNodeSet* FindFENodeSet(const string& szname);

	void ClearFEPartSets();
	void ClearFEElementSets();
	void ClearFESurfaces();
	void ClearFEEdgeSets();
	void ClearFENodeSets();
	void ClearFEGroups();

	void RemoveEmptyFEGroups();
	void RemoveUnusedFEGroups();

	GNode* FindNodeFromTag(int ntag);

	void Reindex();

public:
	static void SetActiveObject(GObject* po);
	static GObject* GetActiveObject();
	bool IsActiveObject() const;

private:
	Imp*	imp;

	static GObject*		m_activeObject;
};

// helper function to see if two faces are identical
bool IsSameFace(int n[4], int m[4]);
