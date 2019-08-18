#include "stdafx.h"
#include "GLDisplacementMap.h"
#include "GLModel.h"
#include "PostLib/PropertyList.h"
using namespace Post;

//-----------------------------------------------------------------------------
class CGLDisplacementMapProps : public CPropertyList
{
public:
	CGLDisplacementMapProps(CGLDisplacementMap* map) : m_map(map)
	{
		addProperty("Data field", CProperty::DataVec3);
		addProperty("Scale factor", CProperty::Float);
	}

	QVariant GetPropertyValue(int i)
	{
		if (m_map)
		{
			if (i == 0)
			{
				FEModel* pfem = m_map->GetModel()->GetFEModel();
				return pfem->GetDisplacementField();
			}
			if (i == 1) return m_map->m_scl;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		if (i == 0)
		{
			FEModel* pfem = m_map->GetModel()->GetFEModel();
			pfem->SetDisplacementField(v.toInt());
		}
		if (i == 1) m_map->m_scl = v.toFloat();
	}

private:
	CGLDisplacementMap*	m_map;
};

//-----------------------------------------------------------------------------

CGLDisplacementMap::CGLDisplacementMap(CGLModel* po) : CGLDataMap(po)
{
	char szname[128] = { 0 };
	sprintf(szname, "Displacement Map");
	SetName(szname);

	m_scl = 1.f;
}

//-----------------------------------------------------------------------------

void CGLDisplacementMap::Activate(bool b)
{
	CGLObject::Activate(b);

	if (b == false)
	{
		CGLModel* po = GetModel();
		FEMeshBase* pm = po->GetActiveMesh();
		for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_rt = pm->Node(i).m_r0;
		pm->UpdateNormals(po->RenderSmooth());
	}
}

//-----------------------------------------------------------------------------
CPropertyList* CGLDisplacementMap::propertyList()
{
	return new CGLDisplacementMapProps(this);
}

//-----------------------------------------------------------------------------

void CGLDisplacementMap::Update(int ntime, float dt, bool breset)
{
	CGLModel* po = GetModel();
	FEMeshBase* pm = po->GetActiveMesh();
	FEModel* pfem = po->GetFEModel();

	// get the number of states and make sure we have something
	int N = pfem->GetStates();
	if (N == 0) return;

	// get the two bracketing time states
	int n0 = ntime;
	int n1 = (ntime + 1 >= N ? ntime : ntime + 1);
	if (dt == 0.f) n1 = n0;

	if (n0 == n1)
	{
		// update the states
		UpdateState(n0, breset);
		FEState& s1 = *pfem->GetState(n0);

		// set the current nodal positions
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			vec3f du = s1.m_NODE[i].m_rt - node.m_r0;

			// the scaled displacement is stored on the mesh
			node.m_rt = node.m_r0 + du*m_scl;
		}
	}
	else
	{
		FEState& s1 = *pfem->GetState(n0);
		FEState& s2 = *pfem->GetState(n1);

		// update the states
		UpdateState(n0, breset);
		UpdateState(n1, breset);

		// calculate weight factor
		float df = s2.m_time - s1.m_time;
		if (df == 0) df = 1.f;
		float w = dt / df;

		// set the current nodal positions
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);

			// get nodal displacements
			vec3f d1 = s1.m_NODE[i].m_rt - node.m_r0;
			vec3f d2 = s2.m_NODE[i].m_rt - node.m_r0;

			// evaluate current displacement
			vec3f du = d2*w + d1*(1.f - w);

			// the scaled displacement is stored on the mesh
			node.m_rt = node.m_r0 + du*m_scl;
		}
	}

	// update the normals
	pm->UpdateNormals(po->RenderSmooth());
}

//-----------------------------------------------------------------------------

void CGLDisplacementMap::UpdateState(int ntime, bool breset)
{
	CGLModel* po = GetModel();
	FEMeshBase* pm = po->GetActiveMesh();
	FEModel* pfem = po->GetFEModel();

	int N = pfem->GetStates();

	// TODO: This does not look right the correct place for this
	if (breset || (N != m_ntag.size())) m_ntag.assign(N, -1);

	int nfield = pfem->GetDisplacementField();

	if ((nfield >= 0) && (m_ntag[ntime] != nfield))
	{
		m_ntag[ntime] = nfield;

		FEState& s = *pfem->GetState(ntime);

		// set the current nodal positions
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			vec3f dr = pfem->EvaluateNodeVector(i, ntime, nfield);

			// the actual nodal position is stored in the state
			// this is the field that will be used for strain calculations
			s.m_NODE[i].m_rt = node.m_r0 + dr;
		}
	}
}
