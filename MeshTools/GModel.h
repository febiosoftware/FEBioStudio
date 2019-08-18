#pragma once
#include "GeomLib/GObject.h"
#include <FSCore/Serializable.h>
#include "GGroup.h"
#include "GDiscreteObject.h"

#include <vector>
using namespace std;

class FEModel;
class GPartList;
class GFaceList;
class GEdgeList;
class GNodeList;
class GObjectSelection;

//-----------------------------------------------------------------------------
// The GModel class manages all GObjects and GGroups that are created by
// the user.
//
class GModel : public CSerializable
{
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
	int Objects() const { return (int)m_Obj.size(); }

	// return an object
	GObject* Object(int n) { return ((n>=0) && (n<(int) m_Obj.size())? m_Obj[n] : 0); }

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
	int PartLists() const { return (int)m_GPart.size(); }
	GPartList* PartList(int n) { return m_GPart[n]; }

	// --- GFaceList ---
	void AddFaceList(GFaceList* pg);
	void InsertFaceList(int n, GFaceList* pg);
	int RemoveFaceList(GFaceList* pg);
	int FaceLists() const { return (int)m_GFace.size(); }
	GFaceList* FaceList(int n) { return m_GFace[n]; }

	// --- GEdgeList ---
	void AddEdgeList(GEdgeList* pg);
	void InsertEdgeList(int n, GEdgeList* pg);
	int RemoveEdgeList(GEdgeList* pg);
	int EdgeLists() const { return (int)m_GEdge.size(); }
	GEdgeList* EdgeList(int n) { return m_GEdge[n]; }

	// --- GNodeList ---
	void AddNodeList(GNodeList* pg);
	void InsertNodeList(int n, GNodeList* pg);
	int RemoveNodeList(GNodeList* pg);
	int NodeLists() const { return (int)m_GNode.size(); }
	GNodeList* NodeList(int n) { return m_GNode[n]; }

	// --- Discrete Objects ---
	int DiscreteObjects() { return (int)m_Discrete.size(); }
	GDiscreteObject* DiscreteObject(int n);
	void AddDiscreteObject(GDiscreteObject* po) { m_Discrete.push_back(po); }
	int RemoveDiscreteObject(GDiscreteObject* po);
	void InsertDiscreteObject(GDiscreteObject* po, int n);
	int FindDiscreteObjectIndex(GDiscreteObject* po);
	GDiscreteObject* FindDiscreteObject(const std::string& name);

	void RemoveNamedSelections();
	void RemoveEmptySelections();

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
	void ShowPart(GPart* pg, bool bshow = true);

	void ShowAllObjects();
	void ShowAllParts(GObject* po);

	void DeletePart(GPart* pg);

protected:
	vector<GObject*>	m_Obj;	//!< list of objects
	FEModel*			m_ps;	//!< pointer to model
	BOX					m_box;	//!< bounding box

	vector<GPartList*>	m_GPart;	//!< list of GPartGroup
	vector<GFaceList*>	m_GFace;	//!< list of GFaceGroup
	vector<GEdgeList*>	m_GEdge;	//!< list of GEdgeGroup
	vector<GNodeList*>	m_GNode;	//!< list of GNodeGroup

	vector<GDiscreteObject*>	m_Discrete;	//!< list of discrete objects
};

// helper function for generating objects from a type
GObject* BuildObject(int ntype);

//-----------------------------------------------------------------------------
class GNodeIterator
{
public:
	GNodeIterator(GModel& m) : m_mdl(m)
	{
		reset();
	}

	void operator ++ ()
	{
		if (m_node != -1)
		{
			m_node++;
			if (m_node >= m_mdl.Object(m_obj)->Nodes())
			{
				m_obj++;
				m_node = 0;

				if (m_obj >= m_mdl.Objects())
				{
					m_node = -1;
					m_obj = -1;
				}
			}
		}
	}

	GNode* operator -> ()
	{
		if (m_node >= 0)
		{
			return m_mdl.Object(m_obj)->Node(m_node);
		}
		else
			return 0;		
	}

	operator GNode* ()
	{
		if (m_node >= 0)
		{
			return m_mdl.Object(m_obj)->Node(m_node);
		}
		else
			return 0;
	}


	bool isValid() const { return (m_node >= 0); }

	void reset()
	{
		if (m_mdl.Objects() > 0)
		{
			m_node = 0;
			m_obj = 0;
		}
		else m_node = m_obj = -1;
	}

private:
	GModel&	m_mdl;
	int m_node;
	int m_obj;
};
