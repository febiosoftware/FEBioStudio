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

// forward declarations
class FSElemSet;
class FSSurface;
class FSEdgeSet;
class FSNodeSet;
class FSPartSet;
class FSCurveMesh;
class FSSurfaceMesh;
class FEMesher;
class FSMesh;
class FSMeshBase;
class FSLineMesh;
class FSNode;
class FSGroup;
class GLMesh;
class GObject;
class GObjectManipulator;

//! GObject is the base class for all geometry objects
class GObject : public GBaseObject
{
public:
	//! Internal implementation class
	class Imp;

public:
	// --- C R E A T I O N ---

	//! Constructor
	GObject(int ntype);
	//! Destructor
	virtual ~GObject(void);

	//! Create the object
	virtual void Create() {}

	//! Create a clone of this object
	virtual GObject* Clone() { return 0; }

	//! Copy the geometry from another object. This does not copy the mesh from the passed object. If a mesh exists then this mesh is deleted.
	virtual void Copy(GObject* po);

	//! Return type of Object
	int GetType() const;

	//! Get object color
	GLColor GetColor() const;
	//! Set object color
	void SetColor(const GLColor& c);

	//! Collapse the transformation
	void CollapseTransform();

	//! Save object to archive
	void Save(OArchive& ar) override;
	//! Load object from archive
	void Load(IArchive& ar) override;

	// --- M E S H I N G ---

	//! Set the mesher
	void SetFEMesher(FEMesher* pmesher);

	//! Retrieve the mesher
	virtual FEMesher* GetFEMesher();

	//! Create a default mesher
	virtual FEMesher* CreateDefaultMesher();

	//! Retrieve the FE mesh
	FSMesh* GetFEMesh();
	//! Retrieve the FE mesh (const version)
	const FSMesh* GetFEMesh() const;

	//! Get the editable mesh
	virtual FSMeshBase* GetEditableMesh() { return 0; }

	//! Get the editable line mesh
	virtual FSLineMesh* GetEditableLineMesh() { return 0; }

	//! Replace the current mesh (returns old mesh)
	FSMesh* ReplaceFEMesh(FSMesh* pm);

	//! Set the FE mesh
	void SetFEMesh(FSMesh* pm);

	//! Replace the current surface mesh
	virtual void ReplaceSurfaceMesh(FSSurfaceMesh* newMesh);

	//! Retrieve an FE nodes from a GNode
	FSNode* GetFENode(int gid);

	//! Build the FSMesh
	virtual FSMesh* BuildMesh();

	//! Delete the mesh
	void DeleteFEMesh();

	//! Update data
	bool Update(bool b =  true);

	//! Update GNodes (only GMeshObject uses this)
	void UpdateGNodes();

	//! Update FE mesh
	virtual void UpdateMesh() {}

	//! Get the mesh of an edge curve
	virtual FSCurveMesh* GetFECurveMesh(int edgeId);

	// --- G E O M E T R Y ---

	//! Build the render mesh
	virtual void BuildGMesh();

	//! Get the render mesh
	GLMesh*	GetRenderMesh();

	//! Get the mesh for rendering the FE mesh
	GLMesh* GetFERenderMesh();

	//! Get the local bounding box 
	virtual BOX GetLocalBox() const;

	//! Get the global bounding box
	BOX GetGlobalBox() const;

public:
	//! Show elements in the provided list
	void ShowElements(std::vector<int>& elemList, bool show);

public:
	//! Show the object
	void Show();

	//! Hide the object
	void Hide();

	//! Show (or hide) a part by index
	void ShowPart(int n, bool bshow);

	//! Show (or hide) a part by reference
	void ShowPart(GPart& part, bool bshow);

	//! Show all parts
	void ShowAllParts();

	//! Update the visibility of items (i.e. surfaces, edges, nodes, and mesh items)
	void UpdateItemVisibility();

	//! Check if the object is valid
	virtual bool IsValid() const;

	//! Check if the object has any dependencies
	virtual bool CanDelete() const;
	//! Check if the mesh can be deleted
	virtual bool CanDeleteMesh() const;

	//! Update the element material IDs
	void UpdateFEElementMatIDs();

public:
	//! Check if a face is visible
	bool IsFaceVisible(const GFace* pf) const;

	//! Build the FE render mesh
	virtual void BuildFERenderMesh();
	//! Update the FE render mesh
	virtual void UpdateFERenderMesh();

protected:
	//! Set the render mesh
	void SetRenderMesh(GLMesh* mesh);
	//! Set the FE render mesh
	void SetFERenderMesh(GLMesh* mesh);

	//! Set the valid flag
	void SetValidFlag(bool b);

public:
	// --- G R O U P S ---
	//! Get number of FE part sets
	int FEPartSets() const;
	//! Get number of FE element sets
	int FEElemSets() const;
	//! Get number of FE surfaces
	int FESurfaces() const;
	//! Get number of FE edge sets
	int FEEdgeSets() const;
	//! Get number of FE node sets
	int FENodeSets() const;

	//! Add an FE element set
	void AddFEElemSet(FSElemSet* pg);
	//! Add an FE surface
	void AddFESurface(FSSurface* pg);
	//! Add an FE edge set
	void AddFEEdgeSet(FSEdgeSet* pg);
	//! Add an FE node set
	void AddFENodeSet(FSNodeSet* pg);
	//! Add an FE part set
	void AddFEPartSet(FSPartSet* pg);

	//! Get an FE element set by index
	FSElemSet* GetFEElemSet(int n);
	//! Get an FE surface by index
	FSSurface* GetFESurface(int n);
	//! Get an FE edge set by index
	FSEdgeSet* GetFEEdgeSet(int n);
	//! Get an FE node set by index
	FSNodeSet* GetFENodeSet(int n);
	//! Get an FE part set by index
	FSPartSet* GetFEPartSet(int n);

	//! Remove an FE element set
	int RemoveFEElemSet(FSElemSet* pg);
	//! Remove an FE surface
	int RemoveFESurface(FSSurface* pg);
	//! Remove an FE edge set
	int RemoveFEEdgeSet(FSEdgeSet* pg);
	//! Remove an FE node set
	int RemoveFENodeSet(FSNodeSet* pg);
	//! Remove an FE part set
	int RemoveFEPartSet(FSPartSet* pg);

	//! Insert an FE element set at specified index
	void InsertFEElemSet(int n, FSElemSet* pg);
	//! Insert an FE surface at specified index
	void InsertFESurface(int n, FSSurface* pg);
	//! Insert an FE edge set at specified index
	void InsertFEEdgeSet(int n, FSEdgeSet* pg);
	//! Insert an FE node set at specified index
	void InsertFENodeSet(int n, FSNodeSet* pg);
	//! Insert an FE part set at specified index
	void InsertFEPartSet(int n, FSPartSet* pg);

	//! Find an FE group by ID
	FSGroup* FindFEGroup(int nid);
	//! Find an FE part set by name
	FSPartSet* FindFEPartSet(const std::string& name);
	//! Find an FE surface by name
	FSSurface* FindFESurface(const std::string& szname);
	//! Find an FE edge set by name
	FSEdgeSet* FindFEEdgeSet(const std::string& szname);
	//! Find an FE node set by name
	FSNodeSet* FindFENodeSet(const std::string& szname);

	//! Clear all FE groups
	void ClearFEGroups();

	//! Remove empty FE groups
	void RemoveEmptyFEGroups();
	//! Remove unused FE groups
	void RemoveUnusedFEGroups();

	//! Find a node from its tag
	GNode* FindNodeFromTag(int ntag);

	//! Reindex the object
	void Reindex();

	//! Get the object manipulator
	GObjectManipulator* GetManipulator();

	//! Set the object manipulator
	void SetManipulator(GObjectManipulator* om);

public:
	//! Set the active object
	static void SetActiveObject(GObject* po);
	//! Get the active object
	static GObject* GetActiveObject();
	//! Check if this is the active object
	bool IsActiveObject() const;

private:
	//! Internal implementation pointer
	Imp*	imp;

	//! Static pointer to the active object
	static GObject*		m_activeObject;
};

//! Helper function to see if two faces are identical
bool IsSameFace(int n[4], int m[4]);

//! Object manipulator class for handling object transformations
class GObjectManipulator
{
public:
	//! Constructor
	GObjectManipulator(GObject* po);
	//! Destructor
	virtual ~GObjectManipulator();

	//! Get the associated object
	GObject* GetObject();

	//! Transform a node with the given transformation
	virtual void TransformNode(GNode* pn, const Transform& T) {}

private:
	//! Pointer to the associated object
	GObject* m_po;
};