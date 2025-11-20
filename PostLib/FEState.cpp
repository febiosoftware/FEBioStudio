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

#include "stdafx.h"
#include "FEState.h"
#include <MeshLib/FSMesh.h>
#include "FEPostModel.h"
#include "FEMeshData_T.h"

using namespace Post;
using namespace std;

ObjectData::ObjectData()
{
	data = nullptr;
	nsize = 0;
}

ObjectData::~ObjectData()
{
	delete [] data;
	nsize = 0;
}

void ObjectData::append(int n)
{
	off.push_back(nsize);
	if (data == nullptr)
	{
		data = new float[n];
		nsize = n;
	}
	else
	{
		int newSize = nsize + n;
		float* tmp = new float[newSize];
		memcpy(tmp, data, sizeof(float)*nsize);
		delete data;
		data = tmp;
		nsize = newSize;
	}
}

void ObjectData::push_back(float f)
{
	int n = nsize;
	append(1);
	data[n] = f;
}

void ObjectData::push_back(vec3f f)
{
	int n = nsize;
	append(3);
	data[n  ] = f.x;
	data[n+1] = f.y;
	data[n+2] = f.z;
}

void ObjectData::push_back(mat3f f)
{
	int n = nsize;
	append(9);
	data[n    ] = f[0][0]; data[n + 1] = f[0][1]; data[n + 2] = f[0][2];
	data[n + 3] = f[1][0]; data[n + 4] = f[1][1]; data[n + 5] = f[1][2];
	data[n + 6] = f[2][0]; data[n + 7] = f[2][1]; data[n + 8] = f[2][2];
}


FERefState::FERefState(FEPostModel* fem)
{

}

//-----------------------------------------------------------------------------
// Constructor
FEState::FEState(float time, FEPostModel* fem, FSMesh* pmesh) : m_fem(fem), m_mesh(pmesh)
{
	m_id = -1;
	m_ref = nullptr; // will be set by model

	FSMesh& mesh = *m_mesh;

	int nodes = mesh.Nodes();
	int edges = mesh.Edges();
	int elems = mesh.Elements();
	int faces = mesh.Faces();

	// allocate storage
	m_NODE.resize(nodes);
	m_EDGE.resize(edges);
	m_ELEM.resize(elems);
	m_FACE.resize(faces);

	// allocate element data
	for (int i=0; i<elems; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		m_ElemData.append(ne);
	}

	// allocate face data
	for (int i=0; i<faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = face.Nodes();
		m_FaceData.append(nf);
	}

	// allocate edge data
	for (int i = 0; i < edges; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		int ne = edge.Nodes();
		m_EdgeData.append(ne);
	}

	// initialize data
	for (int i=0; i<nodes; ++i) m_NODE[i].m_rt = to_vec3f(mesh.Node(i).r);
	for (int i=0; i<elems; ++i)
	{
		m_ELEM[i].m_state = StatusFlags::VISIBLE;
		m_ELEM[i].m_h[0] = 0.f;
		m_ELEM[i].m_h[1] = 0.f;
		m_ELEM[i].m_h[2] = 0.f;
		m_ELEM[i].m_h[3] = 0.f;
	}

	AddPointObjectData();

	int lnObjs = fem->LineObjects();
	m_objLn.resize(lnObjs);
	for (int i = 0; i < lnObjs; ++i)
	{
		OBJ_LINE_DATA& di = m_objLn[i];
		Post::FEPostModel::LineObject& po = *fem->GetLineObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_r1 = po.m_r1;
		di.m_r2 = po.m_r2;

		int ndata = (int)po.m_data.size();
		di.data = new ObjectData;
		for (int j = 0; j < ndata; ++j)
		{
			Post::PlotObjectData& dj = *po.m_data[j];

			switch (dj.Type())
			{
			case DATA_SCALAR: di.data->push_back(0.f); break;
			case DATA_VEC3: di.data->push_back(vec3f(0.f, 0.f, 0.f)); break;
			default:
				assert(false);
			}
		}
	}

	m_time = time;
	m_nField = -1;
	m_status = 0;

	FEDataManager* pdm = fem->GetDataManager();
	int N = pdm->DataFields();
	FEDataFieldPtr it = pdm->FirstDataField();
	for (int i=0; i<N; ++i, ++it)
	{
		ModelDataField& d = *(*it);
		m_Data.push_back(d.CreateData(this));
	}
}

void FEState::AddPointObjectData()
{
	int ptObjs = m_fem->PointObjects();
	m_objPt.resize(ptObjs);
	for (int i = 0; i < ptObjs; ++i)
	{
		OBJ_POINT_DATA& di = m_objPt[i];
		Post::FEPostModel::PointObject& po = *m_fem->GetPointObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_rt = po.m_rt;

		int ndata = (int)po.m_data.size();
		di.data = new ObjectData;
		for (int j = 0; j < ndata; ++j)
		{
			Post::PlotObjectData& dj = *po.m_data[j];

			switch (dj.Type())
			{
			case DATA_SCALAR: di.data->push_back(0.f); break;
			case DATA_VEC3: di.data->push_back(vec3f(0.f, 0.f, 0.f)); break;
			case DATA_MAT3: di.data->push_back(mat3f(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f)); break;
			default:
				assert(false);
			}
		}
	}
}

vec3f FEState::NodePosition(int node)
{
	Post::FEPostModel* fem = GetFSModel();
	int ndisp = fem->GetDisplacementField();
	FERefState& ref = *m_ref;
	vec3f r = ref.m_Node[node].m_rt;
	if (ndisp >= 0) r += fem->EvaluateNodeVector(node, m_id, ndisp);
	return r;
}

vec3f FEState::NodeRefPosition(int node)
{
	FERefState& ref = *m_ref;
	vec3f r = ref.m_Node[node].m_rt;
	return r;
}

//-----------------------------------------------------------------------------
// helper function for copying data
template <class T> void copyData(Post::FEMeshData* dest, Post::FEMeshData* src)
{ 
	T* pd = dynamic_cast<T*>(dest);
	T* ps = dynamic_cast<T*>(src);
	if (pd && ps) pd->copy(*ps); 
	else 
	{
		// We can get here for implicit data fields (e.g. LagrangeStrain). 
		// which is probably okay.
//		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEState::SetID(int n)
{
	m_id = n;
}

//-----------------------------------------------------------------------------
int FEState::GetID() const 
{ 
	assert(m_id != -1);
	return m_id; 
}

//-----------------------------------------------------------------------------
// Constructor
FEState::FEState(float time, FEPostModel* pfem, FEState* pstate) : m_fem(pfem)
{
	m_id = -1;
	m_time = time;
	m_nField = -1;
	m_status = 0;
	m_mesh = pstate->m_mesh;

	RebuildData();

	// get the data manager
	FEDataManager* pdm = pfem->GetDataManager();

	// Nodal data
	int N = pdm->DataFields();
	FEDataFieldPtr pn = pdm->FirstDataField();
	for (int i = 0; i < N; ++i, ++pn)
	{
		m_Data.push_back((*pn)->CreateData(this));
	}

	// copy data
	pn = pdm->FirstDataField();
	for (int i = 0; i < N; ++i, ++pn)
	{
		ModelDataField& d = *(*pn);
		FEMeshData& md = m_Data[i];
		if (d.DataClass() == NODE_DATA)
		{
			switch (md.GetType())
			{
			case DATA_SCALAR : copyData< Post::FENodeData<FEDataTypeTraits<DATA_SCALAR  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_VEC3  : copyData< Post::FENodeData<FEDataTypeTraits<DATA_VEC3  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3  : copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3S : copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3S >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3SD: copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3SD >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_TENS4S: copyData< Post::FENodeData<FEDataTypeTraits<DATA_TENS4S>::dataType> >(&md, &pstate->m_Data[i]); break;
			default:
				assert(false);
			}
		}
		else if (d.DataClass() == FACE_DATA)
		{
			switch (md.GetType())
			{
			case DATA_SCALAR : copyData< Post::FEFaceData<FEDataTypeTraits<DATA_SCALAR  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_VEC3  : copyData< Post::FEFaceData<FEDataTypeTraits<DATA_VEC3  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3  : copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3S : copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3S >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3SD: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3SD >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_TENS4S: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_TENS4S>::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			default:
				assert(false);
			}
		}
		else if (d.DataClass() == ELEM_DATA)
		{
			switch (md.GetType())
			{
			case DATA_SCALAR : copyData< Post::FEElementData<FEDataTypeTraits<DATA_SCALAR  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_VEC3  : copyData< Post::FEElementData<FEDataTypeTraits<DATA_VEC3  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3  : copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3S : copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3S >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3SD: copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3SD >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_TENS4S: copyData< Post::FEElementData<FEDataTypeTraits<DATA_TENS4S>::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			default:
				assert(false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEState::RebuildData()
{
	FSMesh& mesh = *m_mesh;
	FEPostModel& fem = *m_fem;

	int nodes = mesh.Nodes();
	int edges = mesh.Edges();
	int elems = mesh.Elements();
	int faces = mesh.Faces();

	// allocate storage
	m_NODE.resize(nodes);
	m_EDGE.resize(edges);
	m_ELEM.resize(elems);
	m_FACE.resize(faces);

	// allocate element data
	m_ElemData.clear();
	for (int i = 0; i < elems; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		m_ElemData.append(ne);

		m_ELEM[i].m_state = StatusFlags::VISIBLE;
	}

	// allocate face data
	m_FaceData.clear();
	for (int i = 0; i < faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = face.Nodes();
		m_FaceData.append(nf);
	}

	// allocate edge data
	m_EdgeData.clear();
	for (int i = 0; i < edges; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		int ne = edge.Nodes();
		m_EdgeData.append(ne);
	}

	// initialize data
	for (int i = 0; i < nodes; ++i) m_NODE[i].m_rt = to_vec3f(mesh.Node(i).r);
	for (int i = 0; i < elems; ++i)
	{
		m_ELEM[i].m_h[0] = 0.f;
		m_ELEM[i].m_h[1] = 0.f;
		m_ELEM[i].m_h[2] = 0.f;
		m_ELEM[i].m_h[3] = 0.f;
	}

	int ptObjs = fem.PointObjects();
	m_objPt.resize(ptObjs);
	for (int i = 0; i < ptObjs; ++i)
	{
		OBJ_POINT_DATA& di = m_objPt[i];
		Post::FEPostModel::PointObject& po = *fem.GetPointObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_rt = po.m_rt;
	}

	int lnObjs = fem.LineObjects();
	m_objLn.resize(lnObjs);
	for (int i = 0; i < lnObjs; ++i)
	{
		OBJ_LINE_DATA& di = m_objLn[i];
		Post::FEPostModel::LineObject& po = *fem.GetLineObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_r1 = po.m_r1;
		di.m_r2 = po.m_r2;
	}
}

//-----------------------------------------------------------------------------
OBJECTDATA& FEState::GetObjectData(int n)
{
	if (n < m_objPt.size()) return m_objPt[n];
	else return m_objLn[n - m_objPt.size()];
}

//-----------------------------------------------------------------------------
class OctreeBox
{
public:
	OctreeBox(BOX box, int levels) : m_box(box), m_level(levels)
	{
		if (levels == 0)
		{
			for (int i = 0; i < 8; ++i) m_child[i] = nullptr;
			return;
		}

		double R = box.Radius();

		double x0 = box.x0, x1 = box.x1;
		double y0 = box.y0, y1 = box.y1;
		double z0 = box.z0, z1 = box.z1;
		int n = 0;
		for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
		for (int k = 0; k < 2; ++k)
		{
			double xa = x0 + i * (x1 - x0) * 0.5;
			double ya = y0 + j * (y1 - y0) * 0.5;
			double za = z0 + k * (z1 - z0) * 0.5;
			double xb = x0 + (i+1.0) * (x1 - x0) * 0.5;
			double yb = y0 + (j+1.0) * (y1 - y0) * 0.5;
			double zb = z0 + (k+1.0) * (z1 - z0) * 0.5;
			BOX boxi(xa, ya, za, xb, yb, zb);
			boxi.Inflate(R * 1e-7);
			m_child[n++] = new OctreeBox(boxi, levels - 1);
		}
	}
	~OctreeBox() { for (int i = 0; i < 8; ++i) delete m_child[i]; }

	int addNode(vector<vec3d>& points, const vec3d& r)
	{
		const double eps = 1e-12;
		if (m_level == 0)
		{
			if (m_box.IsInside(r))
			{
				for (int i = 0; i < m_nodes.size(); ++i)
				{
					vec3d& ri = points[m_nodes[i]];
					if ((ri - r).SqrLength() <= eps)
					{
						// node is already in list
						return m_nodes[i];
					}
				}

				// if we get here, the node is in this box, 
				// but not in the points array yet, so add it
				points.push_back(r);
				m_nodes.push_back((int)points.size() - 1);
				return (int)points.size() - 1;
			}
			else return -1;
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				int n = m_child[i]->addNode(points, r);
				if (n >= 0) return n;
			}
			return -1;
		}
	}

private:
	int			m_level;
	BOX			m_box;
	OctreeBox*	m_child[8];
	vector<int>	m_nodes;
};

double Post::IntegrateNodes(FSMesh& mesh, const std::vector<int>& nodeList, Post::FEState* ps)
{
	double res = 0.0;
	int N = (int)nodeList.size();
	for (int i = 0; i < N; ++i)
	{
		FSNode& node = mesh.Node(nodeList[i]);
		if (ps->m_NODE[i].m_ntag > 0)
		{
			res += ps->m_NODE[i].m_val;
		}
	}
	return res;
}

double Post::IntegrateEdges(FSMesh& mesh, const std::vector<int>& edgeList, Post::FEState* ps)
{
	assert(false);
	return 0.0;
}

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double Post::IntegrateFaces(FSMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps)
{
	double res = 0.0;
	float v[FSFace::MAX_NODES];
	vec3f r[FSFace::MAX_NODES];
	int NF = (int)faceList.size();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh.Face(faceList[i]);
		if (f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal values
			for (int j = 0; j < nn; ++j) v[j] = ps->m_NODE[f.n[j]].m_val;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				v[3] = v[2];
				break;
			}

			// get the nodal coordinates
			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// add to integral
			res += IntegrateQuad(r, v);
		}
	}
	return res;
}

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double Post::IntegrateReferenceFaces(FSMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps)
{
	Post::FERefState& ref = *ps->m_ref;
	double res = 0.0;
	float v[FSFace::MAX_NODES];
	vec3f r[FSFace::MAX_NODES];
	int NF = (int)faceList.size();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh.Face(faceList[i]);
		if (f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal values
			for (int j = 0; j < nn; ++j) v[j] = ps->m_NODE[f.n[j]].m_val;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				v[3] = v[2];
				break;
			}

			// get the (reference!) nodal coordinates
			for (int j = 0; j < nn; ++j) r[j] = ref.m_Node[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// add to integral
			res += IntegrateQuad(r, v);
		}
	}
	return res;
}

vec3d Post::IntegrateSurfaceNormal(FSMesh& mesh, Post::FEState* ps)
{
	vec3d res(0, 0, 0);
	float vx[FSFace::MAX_NODES], vy[FSFace::MAX_NODES], vz[FSFace::MAX_NODES];
	vec3f r[FSFace::MAX_NODES];
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& f = mesh.Face(i);
		vec3d N = mesh.FaceNormal(f);

		if (f.IsSelected() && f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal coordinates
			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// get the nodal values
			for (int j = 0; j < nn; ++j)
			{
				double v = ps->m_NODE[f.n[j]].m_val;
				vx[j] = (float)(N.x * v);
				vy[j] = (float)(N.y * v);
				vz[j] = (float)(N.z * v);
			}

			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				vx[3] = vx[2];
				vy[3] = vy[2];
				vz[3] = vz[2];
				break;
			}

			// add to integral
			res.x += IntegrateQuad(r, vx);
			res.y += IntegrateQuad(r, vy);
			res.z += IntegrateQuad(r, vz);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double Post::IntegrateReferenceElems(FSMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps)
{
	Post::FERefState& ref = *ps->m_ref;
	double res = 0.0;
	float v[FSElement::MAX_NODES];
	vec3f r[FSElement::MAX_NODES];
	int NE = (int)elemList.size();
	for (int i = 0; i < NE; ++i)
	{
		FSElement_& e = mesh.ElementRef(elemList[i]);
		if (e.IsSolid() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and (reference!) coordinates
			for (int j = 0; j < nn; ++j) v[j] = ps->m_NODE[e.m_node[j]].m_val;
			for (int j = 0; j < nn; ++j) r[j] = ref.m_Node[e.m_node[j]].m_rt;
			switch (e.Type())
			{
			case FE_PENTA6:
				v[7] = v[5]; r[7] = r[5];
				v[6] = v[5]; r[6] = r[5];
				v[5] = v[4]; r[5] = r[4];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
				v[7] = v[3]; r[7] = r[3];
				v[6] = v[3]; r[6] = r[3];
				v[5] = v[3]; r[5] = r[3];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			}

			// add to integral
			res += IntegrateHex(r, v);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double Post::IntegrateElems(FSMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps)
{
	double res = 0.0;
	float v[FSElement::MAX_NODES] = { 0.f };
	vec3f r[FSElement::MAX_NODES];
	int NE = (int)elemList.size();
	for (int i = 0; i < NE; ++i)
	{
		FSElement_& e = mesh.ElementRef(elemList[i]);
		if (e.IsSolid() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and coordinates
			for (int j = 0; j < nn; ++j) v[j] = ps->m_ElemData.value(i, j);
			//			for (int j = 0; j < nn; ++j) v[j] = ps->m_NODE[e.m_node[j]].m_val;

			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[e.m_node[j]].m_rt;
			switch (e.Type())
			{
			case FE_PENTA6:
				v[7] = v[5]; r[7] = r[5];
				v[6] = v[5]; r[6] = r[5];
				v[5] = v[4]; r[5] = r[4];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
				v[7] = v[3]; r[7] = r[3];
				v[6] = v[3]; r[6] = r[3];
				v[5] = v[3]; r[5] = r[3];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			}

			// add to integral
			res += IntegrateHex(r, v);
		}
		else if (e.IsShell() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and coordinates
			for (int j = 0; j < nn; ++j) v[j] = ps->m_ElemData.value(i, j);
			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[e.m_node[j]].m_rt;

			switch (e.Type())
			{
			case FE_TRI3:
				v[3] = v[2]; r[3] = r[2];
				break;
			}

			res += IntegrateQuad(r, v);
		}

		// TODO: This was done so that discrete element variables can be added, but I don't think that makes sense
		//       for other element types that are considered "beams", e.g. discrete elements. 
		//       I think the solution is to distinguish between "beams" and "discrete" elements. 
		if (e.IsBeam() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			double v0 = ps->m_ElemData.value(i, 0);
			double v1 = ps->m_ElemData.value(i, 1);
			res += 0.5 * (v0 + v1);
		}
	}
	return res;
}
