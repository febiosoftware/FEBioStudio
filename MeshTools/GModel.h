#pragma once
#include <FSCore/FSObject.h>
#include <FSCore/box.h>
#include <vector>
using namespace std;

class GObject;
class FEModel;
class FENodeSet;
class FESurface;
class GPart;
class GFace;
class GEdge;
class GNode;
class GPartList;
class GFaceList;
class GEdgeList;
class GNodeList;
class GObjectSelection;
class FEItemListBuilder;
class GDiscreteObject;
class GDiscreteElementSet;
class ObjectMeshList;
class MeshLayer;
class MeshLayerManager;

//-----------------------------------------------------------------------------
// The GModel class manages all GObjects and GGroups that are created by
// the user.
//
class GModel : public FSObject
{
	class Imp;

public:
	//! default constructor
	GModel(FEModel*);

	//! destructor
	~GModel(void);

	//! clear all objects and groups
	void Clear();

	//! clear groups
	void ClearGroups();

	//! clear all discrete objects
	void ClearDiscrete();

	//! reset model
	static void Reset();

	//! Save the model to the archive
	void Save(OArchive& ar);

	//! Load the model from the archive
	void Load(IArchive& ar);

	// return number of objects
	int Objects() const;

	// return an object
	GObject* Object(int n);

	// find an object from its ID
	GObject* FindObject(int id);

	// find an object from its name
	GObject* FindObject(const string& name);

	// find the index of an object
	int FindObjectIndex(GObject* po);

	// replace an object
	void ReplaceObject(int n, GObject* po);

	// replace an object
	void ReplaceObject(GObject* po, GObject* pn);

	// add an object to the model
	void AddObject(GObject* po);

	// remove an object from the model
	int RemoveObject(GObject* po, bool deleteMeshList = false);

	// insert an object before the index
	void InsertObject(GObject* po, int n);

	// --- part functions ---

	// return the total nr of parts
	int Parts();

	// return part n
	GPart* Part(int n);

	// find a part based on its ID
	GPart* FindPart(int nid);

	// --- surface functions ---

	// return the total nr of surfaces
	int Surfaces();

	// return surface n
	GFace* Surface(int n);

	// find a surface based on its ID
	GFace* FindSurface(int nid);

	// --- edge functions ---

	// return total nr of edges
	int Edges();

	// return edge n
	GEdge* Edge(int n);

	// find an edge based on its ID
	GEdge* FindEdge(int nid);

	// find an edge from its name
	GEdge* FindEdgeFromName(const string& name);

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
	FENodeSet* GetNodesetFromID(int id);
	FESurface* GetSurfaceFromID(int id);

	BOX GetBoundingBox();
	void UpdateBoundingBox();

	// count named selections
	int CountNamedSelections() const;
	FEItemListBuilder* FindNamedSelection(int nid);
	FEItemListBuilder* FindNamedSelection(const std::string& name);
	vector<FEItemListBuilder*> AllNamedSelections(int ntype = 0);

	// --- GPartList ---
	void AddPartList(GPartList* pg);
	void InsertPartList(int n, GPartList* pg);
	int RemovePartList(GPartList* pg);
	int PartLists() const;
	GPartList* PartList(int n);

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

	void RemoveNamedSelections();
	void RemoveEmptySelections();

	// if match is true, the list of parts with this material is returend.
	// if match is false, the list of parts that don't have this material is returned.
	list<GPart*> FindPartsFromMaterial(int matId, bool bmatch = true);

public:
	// create a clone
	GObject* CloneObject(GObject *po);

	// clone the object on a grid
	vector<GObject*> CloneGrid(GObject* po, int x0, int x1, int y0, int y1, int z0, int z1, double dx, double dy, double dz);

	// reolve clone the object
	vector<GObject*> CloneRevolve(GObject* po, int count, double range, double spiral, const vec3d& center, const vec3d& axis, bool rotateClones);

	// merge the selected objects
	GObject* MergeSelectedObjects(GObjectSelection* sel, const string& newObjectName, bool weld, double tol);

	// detach the discrete set
	GObject* DetachDiscreteSet(GDiscreteElementSet* set);

	// merge a "discrete" object with the rest
	GObject* MergeDiscreteObject(vector<GObject*> discreteObjects, vector<GObject*>& objList, double tol);

public:
	// show (or hide if bshow==false) a list of objects
	void ShowObjects(const vector<int>& objList, bool bshow = true);
	void ShowObject(GObject* po, bool bshow = true);

	// select a list of objects
	void SelectObjects(const vector<int>& objList);

	// show or hide a list of parts
	void ShowParts(const vector<int>& partList, bool bshow, bool bselect = false);
	void ShowParts(vector<GPart*>& partList, bool bshow);
	void ShowParts(list<GPart*>& partList, bool bshow);
	void ShowPart(GPart* pg, bool bshow = true);

	void ShowAllObjects();
	void ShowAllParts(GObject* po);

	void DeletePart(GPart* pg);

public:
	int MeshLayers() const; 
	int GetActiveMeshLayer() const;
	void SetActiveMeshLayer(int n);
	int FindMeshLayer(const std::string& s);
	const std::string& GetMeshLayerName(int i) const;
	bool AddMeshLayer(const std::string& layerName);
	void DeleteMeshLayer(int n);

	ObjectMeshList* GetObjectMeshList(GObject* po);
	void InsertObjectMeshList(ObjectMeshList* oml);

	MeshLayer* RemoveMeshLayer(int index);
	void InsertMeshLayer(int index, MeshLayer* layer);

	MeshLayerManager* GetMeshLayerManager();

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
