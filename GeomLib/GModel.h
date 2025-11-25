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
#include <FSCore/FSObject.h>
#include <FSCore/box.h>
#include <MeshLib/FSItemListBuilder.h>
#include <vector>
#include <functional>

class GObject;
class FSModel;
class FSNodeSet;
class FSSurface;
class GPart;
class GFace;
class GEdge;
class GNode;
class GPartList;
class GFaceList;
class GEdgeList;
class GNodeList;
class GObjectSelection;
class FSItemListBuilder;
class GDiscreteObject;
class GDiscreteElementSet;

//-----------------------------------------------------------------------------
// The GModel class manages all GObjects and GGroups that are created by
// the user.
//
class GModel : public FSObject
{
	class Imp;

public:
	//! default constructor
	GModel(FSModel*);

	//! destructor
	~GModel(void);

	FSModel* GetFSModel();

	//! clear all objects and groups
	void Clear();

	//! clear groups
	void ClearGroups();
	void ClearUnusedGroups();

	//! clear all discrete objects
	void ClearDiscrete();

	//! Save the model to the archive
	void Save(OArchive& ar);

	//! Load the model from the archive
	void Load(IArchive& ar);
    void LoadDiscrete(IArchive& ar);

	// return number of objects
	int Objects() const;

	// return an object
	GObject* Object(int n);

	// find an object from its ID
	GObject* FindObject(int id);

	// find an object from its name
	GObject* FindObject(const std::string& name);

	// get the currently active object
	GObject* GetActiveObject();

	// find the index of an object
	int FindObjectIndex(GObject* po);

	// replace an object
	void ReplaceObject(int n, GObject* po);

	// replace an object
	void ReplaceObject(GObject* po, GObject* pn);

	// add an object to the model
	void AddObject(GObject* po);

	// remove an object from the model
	int RemoveObject(GObject* po);

	// insert an object before the index
	void InsertObject(GObject* po, int n);

	// --- part functions ---

	// return the total nr of parts
	int Parts();

	// return part n
	GPart* Part(int n);

	// find a part based on its ID
	GPart* FindPart(int nid);

	// find a part based on its name
	GPart* FindPart(const std::string& partName);

	// get all the parts (helpful if you want to loop over all parts)
	std::vector<GPart*> AllParts();
	std::vector<GPart*> AllParts(std::function<bool(const GPart* pg)> filter);

	// --- surface functions ---

	// return the total nr of surfaces
	int Surfaces();

	// return surface n
	GFace* Surface(int n);

	// find a surface based on its ID
	GFace* FindSurface(int nid);

	// find a surface from its name
	GFace* FindSurfaceFromName(const std::string& name);

	// --- edge functions ---

	// return total nr of edges
	int Edges();

	// return edge n
	GEdge* Edge(int n);

	// find an edge based on its ID
	GEdge* FindEdge(int nid);

	// find an edge from its name
	GEdge* FindEdgeFromName(const std::string& name);

	// --- node functions ---
	int Nodes();

	GNode* Node(int n);

	GNode* FindNode(int nid);

	// FE counters
	int FENodes ();
	int FEFaces ();
	int Elements();
	int SolidElements();
	int ShellElements();

	// --- group functions ---
	FSNodeSet* GetNodesetFromID(int id);
	FSSurface* GetSurfaceFromID(int id);

	BOX GetBoundingBox();
	void UpdateBoundingBox();

	// count named selections
	int CountNamedSelections() const;
	FSItemListBuilder* FindNamedSelection(int nid);
	FSItemListBuilder* FindNamedSelection(const std::string& name, unsigned int filter = MESH_ITEM_FLAGS::FE_ALL_FLAGS);
	std::vector<FSItemListBuilder*> AllNamedSelections(int ntype = 0);
	void AddNamedSelection(FSItemListBuilder* itemList);

	// --- GPartList ---
	void AddPartList(GPartList* pg);
	void InsertPartList(int n, GPartList* pg);
	int RemovePartList(GPartList* pg);
	int PartLists() const;
	GPartList* PartList(int n);
	GPartList* FindPartList(const std::string& name);

	// --- GFaceList ---
	void AddFaceList(GFaceList* pg);
	void InsertFaceList(int n, GFaceList* pg);
	int RemoveFaceList(GFaceList* pg);
	int FaceLists() const;
	GFaceList* FaceList(int n);

	// --- GEdgeList ---
	void AddEdgeList(GEdgeList* pg);
	void InsertEdgeList(int n, GEdgeList* pg);
	int RemoveEdgeList(GEdgeList* pg);
	int EdgeLists() const;
	GEdgeList* EdgeList(int n);

	// --- GNodeList ---
	void AddNodeList(GNodeList* pg);
	void InsertNodeList(int n, GNodeList* pg);
	int RemoveNodeList(GNodeList* pg);
	int NodeLists() const;
	GNodeList* NodeList(int n);

	// --- Discrete Objects ---
	int DiscreteObjects();
	GDiscreteObject* DiscreteObject(int n);
	void AddDiscreteObject(GDiscreteObject* po);
	int RemoveDiscreteObject(GDiscreteObject* po);
	void InsertDiscreteObject(GDiscreteObject* po, int n);
	int FindDiscreteObjectIndex(GDiscreteObject* po);
	GDiscreteObject* FindDiscreteObject(const std::string& name);

	void RemoveMeshData();
	void RemoveNamedSelections();
	void RemoveEmptySelections();
	void RemoveUnusedSelections();

	// if match is true, the list of parts with this material is returend.
	// if match is false, the list of parts that don't have this material is returned.
	std::list<GPart*> FindPartsFromMaterial(int matId, bool bmatch = true);

public:
	// create a clone
	GObject* CloneObject(GObject *po);

	// clone the object on a grid
	std::vector<GObject*> CloneGrid(GObject* po, int x0, int x1, int y0, int y1, int z0, int z1, double dx, double dy, double dz);

	// reolve clone the object
	std::vector<GObject*> CloneRevolve(GObject* po, int count, double range, double spiral, const vec3d& center, const vec3d& axis, bool rotateClones);

	// merge the selected objects
	GObject* MergeSelectedObjects(GObjectSelection* sel, const std::string& newObjectName, bool weld, double tol);

	// detach the discrete set
	GObject* DetachDiscreteSet(GDiscreteElementSet* set);

	// merge a "discrete" object with the rest
	GObject* MergeDiscreteObject(std::vector<GObject*> discreteObjects, std::vector<GObject*>& objList, double tol);

public:
	// show (or hide if bshow==false) a list of objects
	void ShowObjects(const std::vector<int>& objList, bool bshow = true);
	void ShowObject(GObject* po, bool bshow = true);

	// select a list of objects
	void SelectObjects(const std::vector<int>& objList);

	// show or hide a list of parts
	void ShowParts(const std::vector<int>& partList, bool bshow, bool bselect = false);
	void ShowParts(std::vector<GPart*>& partList, bool bshow);
	void ShowParts(std::list<GPart*>& partList, bool bshow);
	void ShowPart(GPart* pg, bool bshow = true);

	void ShowAllObjects();
	void ShowAllParts(GObject* po);

	bool DeletePart(GPart* pg);
	bool DeleteParts(std::vector<GPart*>& partList);

public:
    void SetLoadOnlyDiscreteFlag(bool flag);

private:
	Imp*	imp;
};

// helper function for generating objects from a type
GObject* BuildObject(int ntype);

//-----------------------------------------------------------------------------
class GNodeIterator
{
public:
	GNodeIterator(GModel& m);

	void operator ++ ();

	GNode* operator -> ();

	operator GNode* ();


	bool isValid() const;

	void reset();

private:
	GModel&	m_mdl;
	int m_node;
	int m_obj;
};

//-----------------------------------------------------------------------------
class GPartIterator
{
public:
	GPartIterator(GModel& m);

	void operator ++ ();

	GPart* operator -> ();

	operator GPart* ();


	bool isValid() const;

	void reset();

private:
	GModel&	m_mdl;
	int m_part;
	int m_obj;
};
