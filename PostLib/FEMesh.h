#pragma once

#include <MeshLib/FENode.h>
#include <MeshLib/FEElement.h>
#include <MeshLib/FECoreMesh.h>
#include "FEGroup.h"
#include <MeshLib/FENodeElementList.h>
#include "FENodeFaceList.h"
#include <FSCore/box.h>
#include <utility>
#include <vector>
using namespace std;

namespace Post {

//-----------------------------------------------------------------------------
class FEMeshBase : public FECoreMesh
{
public:
	// --- M E M O R Y   M A N A G M E N T ---
	//! constructor
	FEMeshBase();

	//! destructor
	virtual ~FEMeshBase();

	//! allocate storage for mesh
	void Create(int nodes, int elems, int faces = 0, int edges = 0) override;

	//! Clean up all data
	void CleanUp();

	//! clean mesh and all data
	void ClearAll();

	// from FELineMesh
	void UpdateMeshData() override;

	const vector<NodeElemRef>& NodeElemList(int n) const { return m_NEL.ElementList(n); }
	vector<NodeFaceRef>& NodeFaceList(int n) { return m_NFL.FaceList(n); }

	// --- G E O M E T R Y ---

	virtual void ClearElements() = 0;

	//! return domains
	int Domains() const { return (int) m_Dom.size(); }

	//! return a domain
	FEDomain& Domain(int i) { return *m_Dom[i]; }

	//! nr of parts
	int Parts() const { return (int) m_Part.size(); }

	//! add a part
	void AddPart(FEPart* pg) { m_Part.push_back(pg); }

	//! return a part
	FEPart& Part(int n) { return *m_Part[n]; }

	// number of surfaces
	int Surfaces() const { return (int) m_Surf.size(); }

	// return a surface
	FESurface& Surface(int n) { return *m_Surf[n]; }

	// Add a surface
	void AddSurface(FESurface* ps) { m_Surf.push_back(ps); }

	//! number of node sets
	int NodeSets() const { return (int) m_NSet.size(); }

	//! return a node set
	FENodeSet& NodeSet(int i) { return *m_NSet[i]; }

	//! Add a node set
	void AddNodeSet(FENodeSet* ps) { m_NSet.push_back(ps); }

	// --- D A T A   U P D A T E ---

	//! update mesh data
	void Update();

protected:
	virtual void CreateElements(int elems) = 0;

protected:
	void BuildFaces();
	void FindFaceNeighbors();
	void FindNeighbours();
	void UpdateNodes();
	void BuildEdges();
	void UpdateDomains();

	void ClearDomains();
	void ClearParts();
	void ClearSurfaces();
	void ClearNodeSets();

protected:
	// --- G E O M E T R Y ---
	vector<FEDomain*>	m_Dom;	// domains

	// user-defined partitions
	vector<FEPart*>		m_Part;	// parts
	vector<FESurface*>	m_Surf;	// surfaces
	vector<FENodeSet*>	m_NSet;	// node sets

	FENodeElementList	m_NEL;
	FENodeFaceList		m_NFL;
};

template <class T> class FEMesh_ : public FEMeshBase
{
public:
	FEMesh_(){}

public:
	// number of elements
	int Elements() const { return (int) m_Elem.size(); }

	//! return an element
	FEElement_& ElementRef(int i) override { return m_Elem[i]; }
	const FEElement_& ElementRef(int i) const override { return m_Elem[i]; }

	void ClearElements() override { m_Elem.clear(); }

protected:
	void CreateElements(int elems) override { m_Elem.resize(elems); }

protected:
	vector<T>	m_Elem;	// element array
};

typedef FEMesh_<FETri3> FETriMesh;
typedef FEMesh_<FEQuad4> FEQuadMesh;
typedef FEMesh_<FETet4> FEMeshTet4;
typedef FEMesh_<FEHex8> FEMeshHex8;
typedef FEMesh_<FEElement> FEMesh;
typedef FEMesh_<FELinearElement> FELinearMesh;

// find the element and the iso-parametric coordinates of a point inside the mesh
bool FindElementRef(FEMeshBase& m, const vec3f& x, int& nelem, double r[3]);

// find the element and the iso-parametric coordinates of a point inside the mesh
// the x coordinates is assumed to be in reference frame
bool FindElementInReferenceFrame(FEMeshBase& m, const vec3f& x, int& nelem, double r[3]);

// project the point p in the reference frame of element el. This returns the iso-parametric coordinates in r.
// The return value is true or false depending if the point is actually inside the element
bool ProjectInsideReferenceElement(FEMeshBase& m, FEElement_& el, const vec3f& p, double r[3]);

// project the point p in the current frame of element el. This returns the iso-parametric coordinates in r.
// The return value is true or false depending if the point is actually inside the element
bool ProjectInsideElement(FEMeshBase& m, FEElement_& el, const vec3f& p, double r[3]);

class FEFindElement
{
public:
	class OCTREE_BOX
	{
	public:
		BOX					m_box;
		vector<OCTREE_BOX*>	m_child;
		int					m_elem;
		int					m_level;

	public:
		OCTREE_BOX();
		~OCTREE_BOX();

		void split(int levels);

		OCTREE_BOX* Find(const vec3f& r);

		bool IsInside(const vec3f& r) const { return m_box.IsInside(r); }

		void Add(BOX& b, int nelem);
	};

public:
	FEFindElement(FEMeshBase& mesh);

	void Init(int nframe = 0);
	void Init(vector<bool>& flags, int nframe = 0);

	bool FindElement(const vec3f& x, int& nelem, double r[3]);

	BOX BoundingBox() const { return m_bound.m_box; }

private:
	void InitReferenceFrame(vector<bool>& flags);
	void InitCurrentFrame(vector<bool>& flags);

	bool FindInReferenceFrame(const vec3f& x, int& nelem, double r[3]);
	bool FindInCurrentFrame(const vec3f& x, int& nelem, double r[3]);

private:
	OCTREE_BOX* FindBox(const vec3f& r);

private:
	OCTREE_BOX	m_bound;
	FEMeshBase&	m_mesh;
	int			m_nframe;	// = 0 reference, 1 = current
};

inline bool FEFindElement::FindElement(const vec3f& x, int& nelem, double r[3])
{
	return (m_nframe == 0 ? FindInReferenceFrame(x, nelem, r) : FindInCurrentFrame(x, nelem, r));
}
}
