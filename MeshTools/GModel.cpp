#include "stdafx.h"
#include "GModel.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GOCCObject.h>
#include "GModifiedObject.h"

//-----------------------------------------------------------------------------
GModel::GModel(FEModel* ps)
{
	SetName("Model");
	m_ps = ps;
}

//-----------------------------------------------------------------------------
GModel::~GModel(void)
{
	Clear();
}

//-----------------------------------------------------------------------------
void GModel::Clear()
{
	int i;

	// cleanup all objects
	for (i=0; i<(int) m_Obj.size(); ++i) delete m_Obj[i];
	m_Obj.clear();

	// cleanup all groups
	for (i=0; i<(int) m_GPart.size(); ++i) delete m_GPart[i]; m_GPart.clear();
	for (i=0; i<(int) m_GFace.size(); ++i) delete m_GFace[i]; m_GFace.clear();
	for (i=0; i<(int) m_GEdge.size(); ++i) delete m_GEdge[i]; m_GEdge.clear();
	for (i=0; i<(int) m_GNode.size(); ++i) delete m_GNode[i]; m_GNode.clear();

	// cleanup discrete objects
	for (i=0; i<(int) m_Discrete.size(); ++i) delete m_Discrete[i]; m_Discrete.clear();
}

//-----------------------------------------------------------------------------
void GModel::ClearGroups()
{
	int i;

	// cleanup all groups
	for (i=0; i<(int) m_GPart.size(); ++i) delete m_GPart[i]; m_GPart.clear();
	for (i=0; i<(int) m_GFace.size(); ++i) delete m_GFace[i]; m_GFace.clear();
	for (i=0; i<(int) m_GEdge.size(); ++i) delete m_GEdge[i]; m_GEdge.clear();
	for (i=0; i<(int) m_GNode.size(); ++i) delete m_GNode[i]; m_GNode.clear();

}

//-----------------------------------------------------------------------------
void GModel::ClearDiscrete()
{
	// cleanup discrete objects
	for (int i = 0; i<(int)m_Discrete.size(); ++i) delete m_Discrete[i]; m_Discrete.clear();
}

//-----------------------------------------------------------------------------

void GModel::Reset()
{
	// reset counters
	GObject::ResetCounter();
	GPart::ResetCounter();
	GFace::ResetCounter();
	GEdge::ResetCounter();
	GNode::ResetCounter();
	FEStep::ResetCounter();
}

//-----------------------------------------------------------------------------
int GModel::RemoveObject(GObject* po)
{
	int i;

	// find where this object is located
	vector<GObject*>::iterator io = m_Obj.begin();
	for (i=0; i<Objects(); ++i, ++io) if ((*io) == po) break;

	assert(io != m_Obj.end());
	
	// remove the mesh from the list
	if (io != m_Obj.end()) m_Obj.erase(io);

	// update the bounding box
	UpdateBoundingBox();

	// note that we don't delete the actual object here.
	// we let the function that called this function deal with that
	return i;
}

//-----------------------------------------------------------------------------

void GModel::InsertObject(GObject* po, int n)
{
	assert( (n>=0) && (n<=Objects()) );

	vector<GObject*>::iterator it = m_Obj.begin();
	for (int i=0; i<n; ++i, ++it) if (i == n) break;

	// insert the mesh to the list
	m_Obj.insert(it, po);

	// update bounding box
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------
GDiscreteObject* GModel::DiscreteObject(int n)
{ 
	if (n < m_Discrete.size())
		return m_Discrete[n]; 
	else
	{
		int nid = n - (int)m_Discrete.size();
		for (int i=0; i<(int) m_Discrete.size(); ++i)
		{
			GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(m_Discrete[i]);
			if (ps)
			{
				int index = ps->FindElement(nid);
				if (index >= 0)
				{
					return &ps->element(index);
				}
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::RemoveDiscreteObject(GDiscreteObject* po)
{
	// find where this object is located
	vector<GDiscreteObject*>::iterator io = m_Discrete.begin();
	for (int i=0; i<DiscreteObjects(); ++i, ++io) 
	{
		GDiscreteObject* pdi = (*io);
		if (pdi == po)
		{
			// remove the mesh from the list
			m_Discrete.erase(io);
			return i;
		}
		else if (dynamic_cast<GDiscreteElementSet*>(pdi))
		{
			// The discrete object can also be a part of a set
			GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(pdi);

			// see if this belongs to the set
			int NE = pds->size();
			for (int j=0; j<NE; ++j)
			{
				GDiscreteObject* pj = &pds->element(j);
				if (pj == po)
				{
					pds->RemoveElement(j);
					return -1;
				}
			}
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------

void GModel::InsertDiscreteObject(GDiscreteObject* po, int n)
{
	assert( (n>=0) && (n<=DiscreteObjects()) );

	vector<GDiscreteObject*>::iterator it = m_Discrete.begin();
	for (int i=0; i<n; ++i, ++it) if (i == n) break;

	// insert the mesh to the list
	m_Discrete.insert(it, po);
}

//-----------------------------------------------------------------------------
int GModel::FindDiscreteObjectIndex(GDiscreteObject* po)
{
	if (po == 0) return -1;

	int N = DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pi = DiscreteObject(i);
		if (pi == po) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
GDiscreteObject* GModel::FindDiscreteObject(const std::string& name)
{
	int N = DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pi = DiscreteObject(i);
		if (pi->GetName() == name) return pi;
	}
	return 0;
}

//-----------------------------------------------------------------------------

GObject* GModel::FindObject(int id)
{
	int N = Objects();
	for (int i=0; i<N; ++i) 
	{
		GObject* po = Object(i);
		if (po->GetID() == id) return po;
	}

	return 0;
}

//-----------------------------------------------------------------------------

GObject* GModel::FindObject(const string& name)
{
	int N = Objects();
	for (int i=0; i<N; ++i) 
	{
		GObject* po = m_Obj[i];
		if (name == po->GetName()) return po;
	}

	return 0;
}

//-----------------------------------------------------------------------------

int GModel::FindObjectIndex(GObject* po)
{
	int N = Objects();
	for (int i=0; i<N; ++i)
	{
		if (m_Obj[i] == po) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------

void GModel::ReplaceObject(int n, GObject* po)
{
	assert((n>=0) && (n<(int) m_Obj.size()));
	RemoveObject(Object(n));
	InsertObject(po, n);
}

//-----------------------------------------------------------------------------

void GModel::ReplaceObject(GObject* po, GObject* pn)
{
	int n = FindObjectIndex(po);
	ReplaceObject(n, pn);
}

//-----------------------------------------------------------------------------
void GModel::AddObject(GObject* po)
{ 
	m_Obj.push_back(po); 
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------

int GModel::Parts()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Parts();
	return n;
}

//-----------------------------------------------------------------------------

GPart* GModel::Part(int n)
{
	int m = 0;
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		for (int j=0; j<po->Parts(); ++j, ++m)
			if (n == m) return po->Part(j);
	}
	return 0;
}

//-----------------------------------------------------------------------------

GPart* GModel::FindPart(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GPart* pg = po->FindPart(nid);
		if (pg) return pg;
	}
	return 0;
}


//-----------------------------------------------------------------------------

int GModel::Surfaces()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Faces();
	return n;
}

//-----------------------------------------------------------------------------

GFace* GModel::Surface(int n)
{
	int nobj = Objects();
	for (int i=0; i<nobj; ++i)
	{
		GObject* po = Object(i);
		int nf = po->Faces();
		if (nf <= n) n -= nf;
		else return po->Face(n); 
	}
	return 0;
}

//-----------------------------------------------------------------------------

GFace* GModel::FindSurface(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GFace* pg = po->FindFace(nid);
		if (pg) return pg;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::Edges()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Edges();
	return n;
}

//-----------------------------------------------------------------------------

GEdge* GModel::Edge(int n)
{
	int m = 0;
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		int ne = po->Edges();
		if (ne <= n) n -= ne;
		else return po->Edge(n);
	}
	return 0;
}

//-----------------------------------------------------------------------------

GEdge* GModel::FindEdge(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GEdge* pg = po->FindEdge(nid);
		if (pg) return pg;
	}
	return 0;
}

//-----------------------------------------------------------------------------

GEdge* GModel::FindEdgeFromName(const string& name)
{
	for (int i = 0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		int NE = po->Edges();
		for (int j=0; j<NE; ++j)
		{
			GEdge* pg = po->Edge(j);
			if (pg->GetName() == name) return pg;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::Nodes()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Nodes();
	return n;
}

//-----------------------------------------------------------------------------

GNode* GModel::Node(int n)
{
	int m = 0;
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		int nn = po->Nodes();
		if (nn <= n) n -= nn;
		else return po->Node(n);
	}
	return 0;
}

//-----------------------------------------------------------------------------

GNode* GModel::FindNode(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GNode* pg = po->FindNode(nid);
		if (pg) return pg;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::FENodes()
{
	int nodes = 0;
	for (int i=0; i<(int) m_Obj.size(); ++i) 
	{
		FEMesh* pm = m_Obj[i]->GetFEMesh();
		if (pm) nodes += pm->Nodes();
	}
	return nodes;
}

//-----------------------------------------------------------------------------

int GModel::FEFaces()
{
	int faces = 0;
	for (int i=0; i<(int) m_Obj.size(); ++i) 
	{
		FEMesh* pm = m_Obj[i]->GetFEMesh();
		if (pm) faces += pm->Faces();
	}
	return faces;
}

//-----------------------------------------------------------------------------

int GModel::Elements()
{
	int elems = 0;
	for (int i=0; i<(int) m_Obj.size(); ++i)
	{
		FEMesh* pm = m_Obj[i]->GetFEMesh();
		if (pm) elems += pm->Elements();
	}
	return elems;
}

//-----------------------------------------------------------------------------

int GModel::SolidElements()
{
	int elems = 0;
	for (int i=0; i<(int) m_Obj.size(); ++i)
	{
		FEMesh* pm = m_Obj[i]->GetFEMesh();
		for (int j=0; j<pm->Elements(); ++j)
		{
			FEElement& el = pm->Element(j);
			if (el.IsSolid()) ++elems;
		}
	}
	return elems;
}

//-----------------------------------------------------------------------------

int GModel::ShellElements()
{
	int elems = 0;
	for (int i=0; i<(int) m_Obj.size(); ++i)
	{
		FEMesh* pm = m_Obj[i]->GetFEMesh();
		for (int j=0; j<pm->Elements(); ++j)
		{
			FEElement& el = pm->Element(j);
			if (el.IsShell()) 
			{
				++elems;
			}
		}
	}
	return elems;
}


//-----------------------------------------------------------------------------

FENodeSet* GModel::GetNodesetFromID(int id)
{
	int i, j;
	for (i=0; i<(int) m_Obj.size(); ++i)
	{
		GObject* po = m_Obj[i];
		for (j=0; j<po->FENodeSets(); ++j)
		{
			FENodeSet* pn = po->GetFENodeSet(j);
			if (pn->GetID() == id) return pn;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

FESurface* GModel::GetSurfaceFromID(int id)
{
	int i, j;
	for (i=0; i<(int) m_Obj.size(); ++i)
	{
		GObject* po = m_Obj[i];
		for (j=0; j<po->FESurfaces(); ++j)
		{
			FESurface* ps = po->GetFESurface(j);
			if (ps->GetID() == id) return ps;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

BOX GModel::GetBoundingBox()
{
	return m_box;
}

//-----------------------------------------------------------------------------
void GModel::UpdateBoundingBox()
{
	if (m_Obj.size() == 0) 
	{
		m_box = BOX(-1, -1, -1, 1, 1, 1);
	}
	else
	{
		m_box = m_Obj[0]->GetGlobalBox();
		for (int i = 1; i<(int)m_Obj.size(); ++i) m_box += m_Obj[i]->GetGlobalBox();
	}
}

//-----------------------------------------------------------------------------
// count named selections
int GModel::CountNamedSelections() const
{
	int nsel = 0;
	nsel += PartLists();
	nsel += FaceLists();
	nsel += EdgeLists();
	nsel += NodeLists();

	for (int i=0; i<Objects(); ++i)
	{
		const GObject* obj = m_Obj[i];
		nsel += obj->FENodeSets();
		nsel += obj->FESurfaces();
		nsel += obj->FEEdgeSets();
		nsel += obj->FEParts();
	}

	return nsel;
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GModel::FindNamedSelection(int nid)
{
	// don't bother looking if the ID is invalid
	if (nid < 0) return 0;

	int i, N;

	FEItemListBuilder* pg = 0;

	// search the GGroups
	N = PartLists();
	for (i = 0; i<N; ++i)
	{
		pg = PartList(i);
		if (pg->GetID() == nid) return pg;
	}

	N = FaceLists();
	for (i = 0; i<N; ++i)
	{
		pg = FaceList(i);
		if (pg->GetID() == nid) return pg;
	}

	N = EdgeLists();
	for (i = 0; i<N; ++i)
	{
		pg = EdgeList(i);
		if (pg->GetID() == nid) return pg;
	}

	N = NodeLists();
	for (i = 0; i<N; ++i)
	{
		pg = NodeList(i);
		if (pg->GetID() == nid) return pg;
	}

	// search all objects
	for (int n = 0; n<Objects(); ++n)
	{
		GObject* po = Object(n);
		FEMesh* pm = po->GetFEMesh();

		N = po->FEParts();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFEPart(i);
			if (pg->GetID() == nid) return pg;
		}

		N = po->FESurfaces();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFESurface(i);
			if (pg->GetID() == nid) return pg;
		}

		N = po->FENodeSets();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFENodeSet(i);
			if (pg->GetID() == nid) return pg;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GModel::FindNamedSelection(const std::string& name)
{
	int i, N;
	FEItemListBuilder* pg = 0;

	// search the GGroups
	N = PartLists();
	for (i = 0; i<N; ++i)
	{
		pg = PartList(i);
		if (pg->GetName() == name) return pg;
	}

	N = FaceLists();
	for (i = 0; i<N; ++i)
	{
		pg = FaceList(i);
		if (pg->GetName() == name) return pg;
	}

	N = EdgeLists();
	for (i = 0; i<N; ++i)
	{
		pg = EdgeList(i);
		if (pg->GetName() == name) return pg;
	}

	N = NodeLists();
	for (i = 0; i<N; ++i)
	{
		pg = NodeList(i);
		if (pg->GetName() == name) return pg;
	}

	// search all objects
	for (int n = 0; n<Objects(); ++n)
	{
		GObject* po = Object(n);
		FEMesh* pm = po->GetFEMesh();

		N = po->FEParts();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFEPart(i);
			if (pg->GetName() == name) return pg;
		}

		N = po->FESurfaces();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFESurface(i);
			if (pg->GetName() == name) return pg;
		}

		N = po->FENodeSets();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFENodeSet(i);
			if (pg->GetName() == name) return pg;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
vector<FEItemListBuilder*> GModel::AllNamedSelections(int ntype)
{
	vector<FEItemListBuilder*> list;

	if (ntype == DOMAIN_PART)
	{
		for (int i = 0; i<PartLists(); ++i)
		{
			FEItemListBuilder* pg = PartList(i);
			list.push_back(pg);
		}
	}

	if (ntype == DOMAIN_SURFACE)
	{
		for (int i = 0; i<FaceLists(); ++i)
		{
			FEItemListBuilder* pg = FaceList(i);
			list.push_back(pg);
		}
	}

	if (ntype == DOMAIN_EDGE)
	{
		for (int i = 0; i<EdgeLists(); ++i)
		{
			FEItemListBuilder* pg = EdgeList(i);
			list.push_back(pg);
		}
	}

	if (ntype == DOMAIN_NODESET)
	{
		for (int i = 0; i<NodeLists(); ++i)
		{
			FEItemListBuilder* pg = NodeList(i);
			list.push_back(pg);
		}
	}

	// search all objects
	for (int n = 0; n<Objects(); ++n)
	{
		GObject* po = Object(n);
		FEMesh* pm = po->GetFEMesh();

		if (ntype == DOMAIN_PART)
		{
			for (int i = 0; i<po->FEParts(); ++i)
			{
				FEItemListBuilder* pg = po->GetFEPart(i);
				list.push_back(pg);
			}
		}

		if (ntype == DOMAIN_SURFACE)
		{
			for (int i = 0; i<po->FESurfaces(); ++i)
			{
				FEItemListBuilder*pg = po->GetFESurface(i);
				list.push_back(pg);
			}
		}

		if (ntype == DOMAIN_NODESET)
		{
			for (int i = 0; i<po->FENodeSets(); ++i)
			{
				FEItemListBuilder* pg = po->GetFENodeSet(i);
				list.push_back(pg);
			}
		}
	}

	return list;
}

//-----------------------------------------------------------------------------
void GModel::Save(OArchive &ar)
{
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// save the objects
	ar.BeginChunk(CID_OBJ_GOBJECTS);
	{
		for (int i=0; i<(int) m_Obj.size(); ++i)
		{
			GObject* po = m_Obj[i];
			int ntype = po->GetType();
			ar.BeginChunk(ntype);
			{
				po->Save(ar);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the parts
	for (int i=0; i<(int) m_GPart.size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GPARTGROUP);
		{
			m_GPart[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the surfaces
	for (int i=0; i<(int) m_GFace.size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GFACEGROUP);
		{
			m_GFace[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the edges
	for (int i=0; i<(int) m_GEdge.size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GEDGEGROUP);
		{
			m_GEdge[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the nodes
	for (int i=0; i<(int) m_GNode.size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GNODEGROUP);
		{
			m_GNode[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the discrete objects
	if (m_Discrete.size() > 0)
	{
		ar.BeginChunk(CID_DISCRETE_OBJECT);
		{
			for (int i=0; i<(int) m_Discrete.size(); ++i)
			{
				int ntype = m_Discrete[i]->GetType();
				ar.BeginChunk(ntype);
				{
					m_Discrete[i]->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
GObject* BuildObject(int ntype)
{
	GObject* po = 0;
	switch (ntype)
	{
	case GOBJECT			: po = new GObject(GOBJECT); break;
	case GBOX               : po = new GBox           (); break;
	case GCYLINDER          : po = new GCylinder      (); break;
	case GCYLINDER2         : po = new GCylinder2     (); break;
	case GSPHERE            : po = new GSphere        (); break;
//	case GSPRING            : po = new GSpring		   (); break;
	case GTORUS             : po = new GTorus         (); break;
	case GTUBE              : po = new GTube          (); break;
	case GTUBE2             : po = new GTube2         (); break;
	case GCONE              : po = new GCone          (); break;
	case GHOLLOW_SPHERE     : po = new GHollowSphere  (); break;
	case GDISC	            : po = new GDisc          (); break;
	case GPATCH	            : po = new GPatch         (); break;
	case GRING	            : po = new GRing          (); break;
//	case GSHELL_SPHERE      : po = new GShellSphere   (); break;
//	case GSHELL_TORUS       : po = new GShellTorus    (); break;
	case GSHELL_TUBE        : po = new GThinTube      (); break;
//	case GCURVE             : po = new GCurve         (); break;
//	case GCURVE_CIRCLE      : po = new GCircle        (); break;
	case GMESH_OBJECT       : po = new GMeshObject    ((FEMesh*)0); break;
	case GMODIFIED_OBJECT   : po = new GModifiedObject(0); break;
	case GSLICE		        : po = new GSlice         (); break;
	case GTRUNC_ELLIPSOID   : po = new GTruncatedEllipsoid(); break;
	case GQUART_DOG_BONE    : po = new GQuartDogBone(); break;
	case GSOLIDARC          : po = new GSolidArc    (); break;
	case GCYLINDER_IN_BOX   : po = new GCylinderInBox(); break;
	case GSPHERE_IN_BOX     : po = new GSphereInBox  (); break;
	case GSURFACEMESH_OBJECT: po = new GSurfaceMeshObject(); break;
	case GCURVEMESH_OBJECT  : po = new GCurveMeshObject(); break;
	case GOCCOBJECT         : po = new GOCCObject(); break;
	case GOCC_BOTTLE        : po = new GOCCBottle(); break;
	case GOCC_BOX           : po = new GOCCBox(); break;
	}

	assert(po);
	return po;
}

//-----------------------------------------------------------------------------
void GModel::Load(IArchive &ar)
{
	TRACE("GModel::Load");

	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case CID_FEOBJ_INFO:
			{
				string info;
				ar.read(info);
				SetInfo(info);
			}
		break;
		case CID_OBJ_GOBJECTS:
			{
				while (IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					// create a new geometry object
					GObject* po = BuildObject(ntype);
					if (po == 0) throw ReadError("error parsing CID_OBJ_GOBJECTS in GModel::Load");

					// add object to the model
					AddObject(po);

					// load the object data
					po->Load(ar);

					ar.CloseChunk();
				}
			}
			break;
		case CID_OBJ_GPARTGROUP:
			{
				GPartList* pg = new GPartList(m_ps);
				pg->Load(ar);
				AddPartList(pg);
			}
			break;
		case CID_OBJ_GFACEGROUP:
			{
				GFaceList* pg = new GFaceList(m_ps);
				pg->Load(ar);
				AddFaceList(pg);
			}
			break;
		case CID_OBJ_GEDGEGROUP:
			{
				GEdgeList* pg = new GEdgeList(m_ps);
				pg->Load(ar);
				AddEdgeList(pg);
			}
			break;
		case CID_OBJ_GNODEGROUP:
			{
				GNodeList* pg = new GNodeList(m_ps);
				pg->Load(ar);
				AddNodeList(pg);
			}
			break;
		case CID_DISCRETE_OBJECT:
			{
				while (IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					// create a new geometry object
					GDiscreteObject* po = 0;
					switch (ntype)
					{
					case FE_DISCRETE_SPRING     : po = new GLinearSpring(); break;
					case FE_GENERAL_SPRING      : po = new GGeneralSpring(); break;
					case FE_LINEAR_SPRING_SET   : po = new GLinearSpringSet(); break;
					case FE_NONLINEAR_SPRING_SET: po = new GNonlinearSpringSet(); break;
					default:
						throw ReadError("error parsing CID_DISCRETE_OBJECT (GModel::Load)");
					}

					// load the object data
					po->Load(ar);

					// add object to the model
					AddDiscreteObject(po);

					ar.CloseChunk();
				}
			}
			break;
		}

		ar.CloseChunk();
	}

	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------

void GModel::AddPartList(GPartList *pg)
{
	m_GPart.push_back(pg);
}

//-----------------------------------------------------------------------------

void GModel::InsertPartList(int n, GPartList* pg)
{
	vector<GPartList*>::iterator it = m_GPart.begin();
	for (int i=0; i<n; ++i) ++it;
	m_GPart.insert(it, pg);
}

//-----------------------------------------------------------------------------

int GModel::RemovePartList(GPartList *pg)
{
	vector<GPartList*>::iterator it = m_GPart.begin();
	for (int i=0; i<(int) m_GPart.size(); ++i, ++it) if ((*it) == pg) { m_GPart.erase(it); return i; }
	return -1;
}

//-----------------------------------------------------------------------------

void GModel::AddFaceList(GFaceList *pg)
{
	m_GFace.push_back(pg);
}

//-----------------------------------------------------------------------------

void GModel::InsertFaceList(int n, GFaceList* pg)
{
	vector<GFaceList*>::iterator it = m_GFace.begin();
	for (int i=0; i<n; ++i) ++it;
	m_GFace.insert(it, pg);
}

//-----------------------------------------------------------------------------

int GModel::RemoveFaceList(GFaceList *pg)
{
	vector<GFaceList*>::iterator it = m_GFace.begin();
	for (int i=0; i<(int) m_GFace.size(); ++i, ++it) if ((*it) == pg) { m_GFace.erase(it); return i; }
	return -1;
}

//-----------------------------------------------------------------------------

void GModel::AddEdgeList(GEdgeList *pg)
{
	m_GEdge.push_back(pg);
}


//-----------------------------------------------------------------------------

void GModel::InsertEdgeList(int n, GEdgeList* pg)
{
	vector<GEdgeList*>::iterator it = m_GEdge.begin();
	for (int i=0; i<n; ++i) ++it;
	m_GEdge.insert(it, pg);
}

//-----------------------------------------------------------------------------

int GModel::RemoveEdgeList(GEdgeList *pg)
{
	vector<GEdgeList*>::iterator it = m_GEdge.begin();
	for (int i=0; i<(int) m_GEdge.size(); ++i, ++it) if ((*it) == pg) { m_GEdge.erase(it); return i; }
	return -1;
}

//-----------------------------------------------------------------------------

void GModel::AddNodeList(GNodeList *pg)
{
	m_GNode.push_back(pg);
}

//-----------------------------------------------------------------------------

void GModel::InsertNodeList(int n, GNodeList* pg)
{
	vector<GNodeList*>::iterator it = m_GNode.begin();
	for (int i=0; i<n; ++i) ++it;
	m_GNode.insert(it, pg);
}

//-----------------------------------------------------------------------------

int GModel::RemoveNodeList(GNodeList *pg)
{
	vector<GNodeList*>::iterator it = m_GNode.begin();
	for (int i=0; i<(int) m_GNode.size(); ++i, ++it) if ((*it) == pg) { m_GNode.erase(it); return i; }
	return -1;
}

//-----------------------------------------------------------------------------
void GModel::RemoveNamedSelections()
{
	// remove all FE selections
	for (int i=0; i<(int)m_Obj.size(); ++i)
	{
		GObject& obj = *m_Obj[i];
		obj.ClearFEGroups();
	}

	// remove all geometry selections
	ClearGroups();
}

//-----------------------------------------------------------------------------

template <class T> void clearList(vector<T*>& l)
{
	if (l.empty()) return;

	typename vector<T*>::iterator it = l.begin();
	while (it != l.end())
	{
		if ((*it)->size() == 0)
		{
			it = l.erase(it);
		}
		else ++it;
	}
}

void GModel::RemoveEmptySelections()
{
	clearList(m_GPart);
	clearList(m_GFace);
	clearList(m_GEdge);
	clearList(m_GNode);

	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = m_Obj[i];
		po->RemoveEmptyFEGroups();
	}
}

//-----------------------------------------------------------------------------
void GModel::ShowObjects(const vector<int>& objList, bool bshow)
{
	if (bshow)
        for (int i : objList) Object(i)->Show();
	else
        for (int i : objList) Object(i)->Hide();
}

void GModel::ShowObject(GObject* po, bool bshow)
{
	if (bshow)
		po->Show();
	else
		po->Hide();
}

//-----------------------------------------------------------------------------
void GModel::SelectObjects(const vector<int>& objList)
{
    for (int i : objList)
	{
		if (Object(i)->IsVisible())
		{
			Object(i)->Select();
		}
		else
		{
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------
// show or hide a list of parts
void GModel::ShowParts(const vector<int>& partList, bool bshow, bool bselect)
{
	int N = (int)partList.size();
	for (int i = 0; i<N; ++i)
	{
		GPart* pg = FindPart(partList[i]);
		GObject* po = dynamic_cast<GObject*>(pg->Object()); assert(po);
		if (po) 
		{
			if (bshow) 
			{
				po->ShowPart(*pg, bshow);
				if (bselect) pg->Select();
			}
			else po->ShowPart(*pg, false);
		}
	}
}

//-----------------------------------------------------------------------------
// show or hide a list of parts
void GModel::ShowParts(vector<GPart*>& partList, bool bshow)
{
	for (int i=0; i<Objects(); ++i) Object(i)->m_ntag = 0;
	for (GPart* part : partList)
	{
		if (bshow) part->ShowItem(); else part->HideItem();
		GBaseObject* po = part->Object(); assert(po);
		if (po) po->m_ntag = 1;
	}

	for (int i=0; i<Objects(); ++i) 
	{
		GObject* po = Object(i);
		if (po->m_ntag == 1) po->UpdateItemVisibility();
	}
}

//-----------------------------------------------------------------------------
void GModel::ShowPart(GPart* pg, bool bshow)
{
	if (pg == 0) return;

	GObject* po = dynamic_cast<GObject*>(pg->Object()); assert(po);
	if (po)
	{
		po->ShowPart(*pg, bshow);
	}
}

//-----------------------------------------------------------------------------
void GModel::DeletePart(GPart* pg)
{
	if (pg == 0) return;
	GMeshObject* obj = dynamic_cast<GMeshObject*>(pg->Object());
	if (obj)
	{
		obj->DeletePart(pg);
	}
}

//-----------------------------------------------------------------------------
void GModel::ShowAllObjects()
{
	for (int i = 0; i<Objects(); ++i) ShowObject(Object(i));
}

//-----------------------------------------------------------------------------
void GModel::ShowAllParts(GObject* po)
{
	if (po) po->ShowAllParts();
}

//-----------------------------------------------------------------------------
GObject* GModel::CloneObject(GObject *po)
{
	// clone counter
	static int n = 1;

	// clone the object
	GObject* pco = po->Clone();
	if (pco == 0) return 0;

	// set a new name
	char sz[256];
	sprintf(sz, "Clone%02d", n++);
	pco->SetName(sz);

	// copy material assignments
	assert(pco->Parts() == po->Parts());
	if (pco->Parts() == po->Parts())
	{
		int NP = po->Parts();
		for (int i = 0; i<NP; ++i)
		{
			GPart* srcPart = po->Part(i);
			GPart* dstPart = pco->Part(i);

			dstPart->SetMaterialID(srcPart->GetMaterialID());
		}
	}

	return pco;
}

vector<GObject*> GModel::CloneGrid(GObject* po, int x0, int x1, int y0, int y1, int z0, int z1, double dx, double dy, double dz)
{
	// clone counter
	static int n = 1;

	// sanity checks
	if (x1 < x0) { int tmp = x0; x0 = x1; x1 = tmp; }
	if (y1 < y0) { int tmp = y0; y0 = y1; y1 = tmp; }
	if (z1 < z0) { int tmp = z0; z0 = z1; z1 = tmp; }

	// list of cloned objects
	vector<GObject*> newObjects;

	for (int i = x0; i <= x1; ++i)
		for (int j = y0; j <= y1; ++j)
			for (int k = z0; k <= z1; ++k)
				if ((i != 0) || (j != 0) || (k != 0))
				{
					// clone the object
					GObject* pco = po->Clone();
					if (pco == 0) return newObjects;

					// set a new name
					char sz[256];
					sprintf(sz, "GridClone%02d", n++);
					pco->SetName(sz);

					// copy material assignments
					assert(pco->Parts() == po->Parts());
					if (pco->Parts() == po->Parts())
					{
						int NP = po->Parts();
						for (int i = 0; i<NP; ++i)
						{
							GPart* srcPart = po->Part(i);
							GPart* dstPart = pco->Part(i);

							dstPart->SetMaterialID(srcPart->GetMaterialID());
						}
					}

					// apply transform
					pco->Transform().Translate(vec3d(i*dx, j*dy, k*dz));

					newObjects.push_back(pco);
				}

	return newObjects;
}

//-----------------------------------------------------------------------------
vector<GObject*> GModel::CloneRevolve(GObject* po, int count, double range, double spiral, const vec3d& center, const vec3d& axis, bool rotateClones)
{
	// clone counter
	static int n = 1;

	vector<GObject*> newObjects;

	// make sure there is work to do
	if (count <= 0) return newObjects;

	// get the source object's position
	vec3d r = po->Transform().GetPosition();

	vec3d normAxis = axis.Normalized();

	// create clones
	for (int i = 0; i<count; ++i)
	{
		double w = DEG2RAD*((i + 1) * range / (count + 1));

		double d = (i + 1)*spiral / count;

		quatd Q = quatd(w, axis);

		// clone the object
		GObject* pco = po->Clone();
		if (pco == 0) return newObjects;

		// set a new name
		char sz[256];
		sprintf(sz, "RevolveClone%02d", n++);
		pco->SetName(sz);

		// calculate new positions
		if (rotateClones) pco->Transform().Rotate(Q, center);
		else
		{
			vec3d pos = center + Q*(r - center);
			pco->Transform().SetPosition(pos);
		}
		pco->Transform().Translate(normAxis * d);

		// add the object
		newObjects.push_back(pco);
	}

	// all done
	return newObjects;
}

//-----------------------------------------------------------------------------
GObject* GModel::MergeSelectedObjects(GObjectSelection* sel, const string& newObjectName, bool weld, double tol)
{
	// make sure we have more than one object
	if (sel->Count() == 1) return 0;

	// check for some special cases first.
	// See if one of the objects is a "discrete" object, an object that has only nodes and springs are attached to them
	// This is currently the case if the object's type is GOBJECT
	for (int i = 0; i<sel->Count(); ++i)
	{
		GObject* poi = sel->Object(i);
		if (poi->GetType() == GOBJECT)
		{
			// okay, we assume this is the case, so merge this object with the rest.
			vector<GObject*> discList;
			vector<GObject*> objList;
			for (int j=0; j<sel->Count(); ++j)
			{
				GObject* poj = sel->Object(j);
				if (poj->GetType() == GOBJECT) discList.push_back(poj);
				else objList.push_back(poj);
			}
			GObject* newObj = MergeDiscreteObject(discList, objList, tol); 
			if (newObj) newObj->SetName(newObjectName);
			return newObj;
		}
	}

	// If all objects are surface meshes, we'll create a new surface mesh object
	bool allSurfaces = true;
	for (int i = 0; i<sel->Count(); ++i)
	{
		GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(sel->Object(i));
		if (po == 0)
		{
			allSurfaces = false;
			break;
		}
		else
		{
			// make sure this object has a surface mesh
			if (po->GetEditableMesh() == 0)
			{
				return nullptr;
			}
		}
	}

	if (allSurfaces == false)
	{
		// make sure all objects have meshes
		for (int i = 0; i<sel->Count(); ++i)
		{
			GObject* po = sel->Object(i);
			FEMesh* pm = po->GetFEMesh();
			if (pm == 0)
			{
				return nullptr;
			}
		}
	}

	// this will be the new object
	GObject* newObject = 0;

	if (allSurfaces)
	{
		// create a new object by copying the first selected object
		GSurfaceMeshObject* poa = dynamic_cast<GSurfaceMeshObject*>(sel->Object(0)); assert(poa);
		GSurfaceMeshObject* ponew = new GSurfaceMeshObject(poa); newObject = ponew;
		ponew->SetName(newObjectName.c_str());
		ponew->CopyTransform(poa);

		for (int i = 1; i<sel->Count(); ++i)
		{
			// get the next object
			GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(sel->Object(i));

			// attach it
			ponew->Attach(po, weld, tol);
		}
	}
	else
	{
		// create a new object by copying the first selected object
		GObject* poa = sel->Object(0); assert(poa);
		GMeshObject* ponew = new GMeshObject(poa); newObject = ponew;
		ponew->SetName(newObjectName.c_str());
		ponew->CopyTransform(poa);

		for (int i = 1; i<sel->Count(); ++i)
		{
			// get the next object
			GObject* po = sel->Object(i);

			// attach it
			ponew->Attach(po, weld, tol);
		}
	}
	assert(newObject);

	// the new object must have the same materials as the old objects
	// it is assumed that the number of partitions and their order did not change.
	int np = 0;
	for (int i = 0; i <sel->Count(); ++i)
	{
		GObject* po = sel->Object(i);
		for (int j = 0; j<po->Parts(); ++j, ++np)
		{
			assert(np < newObject->Parts());
			newObject->Part(np)->SetMaterialID(po->Part(j)->GetMaterialID());
		}
	}

	// TODO: Figure out a way to retain the original smoothing groups during merge.
	newObject->GetEditableMesh()->SmoothByPartition();

	return newObject;
}

//-----------------------------------------------------------------------------
GObject* GModel::MergeDiscreteObject(vector<GObject*> discreteObjects, vector<GObject*>& objList, double tol)
{
	// sanity checks
	if (discreteObjects.size() == 0) return 0;
	if (objList.size() == 0) return 0;

	// we're going to assume that we all the objects in the objList are GMeshObjects
	// create a new GMeshObject
	GMeshObject* newObj = new GMeshObject(objList[0]);
	newObj->CopyTransform(objList[0]);
	for (int i = 1; i<objList.size(); ++i)
	{
		// get the next object
		GObject* po = objList[i];
		// attach it
		newObj->Attach(po, false, 0.0);
	}

	// we'll need the mesh of the new object
	FEMesh* pm = newObj->GetFEMesh();

	// now, insert the nodes of the "discrete" objects
	for (int n=0; n<discreteObjects.size(); ++n)
	{
		GObject* po = discreteObjects[n];
		int N = po->Nodes();
		for (int i=0; i<N; ++i)
		{
			po->Node(i)->m_ntag = -1;

			// get the nodal position in the local coordinates of the new object
			vec3d r = po->Node(i)->Position();
			r = newObj->Transform().GlobalToLocal(r);

			// find the closest FE node
			int closestNode = -1;
			double Lmin = 0.0;
			int NN = pm->Nodes();
			for (int j=0; j<NN; ++j)
			{
				vec3d rj = pm->Node(j).r;
				double L = (r - rj).SqrLength();
				if ((closestNode == -1) || (L < Lmin))
				{
					closestNode = j;
					Lmin = L;
				}
			}

			// make sure it falls within the tolerance
			if ((tol == 0.0) || (Lmin < tol))
			{
				FENode& node = pm->Node(closestNode);
				int nodeID = node.m_gid;

				// partition this node
				if (nodeID == -1)
				{
					nodeID = newObj->MakeGNode(closestNode);
				}
				else nodeID = newObj->Node(nodeID)->GetID();

				// store the new ID
				po->Node(i)->m_ntag = nodeID;
			}
		}
	}

	// Okay, now we need to update the springs to reference the new nodes
	int ND = DiscreteObjects();
	for (int i=0; i<ND; ++i)
	{
		GDiscreteElementSet* set = dynamic_cast<GDiscreteElementSet*>(DiscreteObject(i));
		if (set)
		{
			int NE = set->size();
			for (int j=0; j<NE; ++j)
			{
				GDiscreteElement& de = set->element(j);
				int nd[2] = {0};
				nd[0] = de.Node(0);
				nd[1] = de.Node(1);

				GNode* node[2] = {0};
				for (int k=0; k<2; ++k)
				{
					for (int n=0; n<discreteObjects.size(); ++n)
					{
						GObject* po = discreteObjects[n];
						GNode* nk = po->FindNode(nd[k]);
						if (nk)
						{
							node[k] = nk;
							break;
						}						
					}
					if (node[k]) nd[k] = node[k]->m_ntag;
				}
				de.SetNodes(nd[0], nd[1]);
			}
		}
	}

	// all done!
	return newObj;
}

//-----------------------------------------------------------------------------
// detach the discrete set
GObject* GModel::DetachDiscreteSet(GDiscreteElementSet* set)
{
	// make sure we have something to do
	assert(set);
	if ((set == 0) || (set->size() == 0)) return 0;

	GNodeIterator it(*this);
	while (it.isValid()) { it->m_ntag = -1; ++it; }

	// Identify the nodes that need to be copied
	int NE = set->size();
	for (int i=0; i<NE; ++i)
	{
		GDiscreteElement& de = set->element(i);
		int n0 = de.Node(0);
		int n1 = de.Node(1);

		GNode* node0 = FindNode(n0); assert(node0); if (node0 == 0) return 0;
		GNode* node1 = FindNode(n1); assert(node1); if (node1 == 0) return 0;

		node0->m_ntag = 1;
		node1->m_ntag = 1;
	}

	// fill a list
	it.reset();
	vector<GNode*> nodes;
	while (it.isValid())
	{
		if (it->m_ntag == 1) nodes.push_back(it);
		++it;
	}

	// Create a new generic object
	GObject* po = new GObject(GOBJECT);

	// add all the nodes
	for (int i=0; i<(int)nodes.size(); ++i)
	{
		GNode* ni = nodes[i];
		int lid = po->AddNode(ni->Position(), NODE_VERTEX, true);
		ni->m_ntag = po->Node(lid)->GetID();
	}
	po->Update();

	// attach the springs to this new object
	for (int i=0; i<NE; ++i)
	{
		GDiscreteElement& de = set->element(i);
		int n0 = de.Node(0);
		int n1 = de.Node(1);

		GNode* node0 = FindNode(n0); assert(node0); if (node0 == 0) return 0;
		GNode* node1 = FindNode(n1); assert(node1); if (node1 == 0) return 0;

		de.SetNodes(node0->m_ntag, node1->m_ntag);
	}

	return po;
}
