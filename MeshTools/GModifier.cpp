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
#include "GModifier.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>
#include <MeshLib/GMesh.h>

//-----------------------------------------------------------------------------
GModifier::GModifier(void)
{
}

GModifier::~GModifier(void)
{
}

//-----------------------------------------------------------------------------
GModifierStack::GModifierStack(GObject *po)
{
	m_po = po;
	m_pmesh = 0;
}

//-----------------------------------------------------------------------------
GModifierStack::~GModifierStack()
{
	Clear();
}

//-----------------------------------------------------------------------------
void GModifierStack::Clear()
{
	list<GModifier*>::iterator pi = m_Mod.begin();
	for (int i=0; i<(int) m_Mod.size(); ++i, ++pi) delete (*pi);
	m_Mod.clear();
	ClearMesh();
}

//-----------------------------------------------------------------------------
void GModifierStack::ClearMesh()
{
	if (m_pmesh) delete m_pmesh; 
	m_pmesh = 0;
}

//-----------------------------------------------------------------------------
void GModifierStack::Apply()
{
	// get the original mesh
	FSMesh* pm = m_po->GetFEMesh();

	// if no mesh exists, we create a copy of the current mesh
	// else we restore the original mesh before we apply the modifiers
	if (m_pmesh == 0) { m_pmesh = new FSMesh; m_pmesh->ShallowCopy(pm); }
	else pm->ShallowCopy(m_pmesh);

	list<GModifier*>::iterator pi = m_Mod.begin();
	for (int i=0; i<(int) m_Mod.size(); ++i, ++pi) (*pi)->Apply(m_po);
}

//-----------------------------------------------------------------------------
// Note that this function removes the modifier from the stack but it 
// does not delete the modifier. It is assumed that the calling object
// takes care of this.
//
void GModifierStack::Remove(GModifier* pm)
{
	list<GModifier*>::iterator pi = m_Mod.begin();
	for (int i=0; i<(int) m_Mod.size(); ++i, ++pi)
	{
		if (*pi == pm) 
		{
			m_Mod.erase(pi);
			return;
		}
	}
	assert(false);
}

//-----------------------------------------------------------------------------
void GModifierStack::Save(OArchive &ar)
{
	if (m_pmesh)
	{
		ar.BeginChunk(CID_MODIFIER_MESH);
		{
			m_pmesh->Save(ar);
		}
		ar.EndChunk();
	}

	ar.BeginChunk(CID_MODIFIERS);
	{
		list<GModifier*>::iterator pi = m_Mod.begin();
		for (int i=0; i<(int) m_Mod.size(); ++i, ++pi)
		{
			GModifier* pm = (*pi);
			int ntype = 0;
			if      (dynamic_cast<GTwistModifier*>(pm)) ntype = GMOD_TWIST;
			else if (dynamic_cast<GBendModifier*> (pm)) ntype = GMOD_BEND;
			else if (dynamic_cast<GSkewModifier*> (pm)) ntype = GMOD_SKEW;
			else if (dynamic_cast<GWrapModifier*> (pm)) ntype = GMOD_WRAP;
			else if (dynamic_cast<GPinchModifier*>(pm)) ntype = GMOD_PINCH;
			assert(ntype > 0);

			ar.BeginChunk(ntype);
			{
				pm->Save(ar);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();
}

//-----------------------------------------------------------------------------
void GModifierStack::Load(IArchive &ar)
{
	TRACE("GModifierStack::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MODIFIER_MESH:
			m_pmesh = new FSMesh;
			m_pmesh->SetGObject(m_po);
			m_pmesh->Load(ar);
			break;
		case CID_MODIFIERS:
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					GModifier* pm = 0;
					switch (nid)
					{
					case GMOD_TWIST   : pm = new GTwistModifier; break;
					case GMOD_BEND    : pm = new GBendModifier; break;
					case GMOD_SKEW    : pm = new GSkewModifier; break;
					case GMOD_WRAP	  : pm = new GWrapModifier; break;
					case GMOD_PINCH   : pm = new GPinchModifier; break;
					default:
						throw ReadError("error when reading CID_MODIFIERS in GModifierStack::Load");
					}
					assert(pm);
					pm->Load(ar);
					m_Mod.push_back(pm);
					ar.CloseChunk();
				}
			}
			break;
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
void GModifierStack::Copy(GModifierStack* ps)
{
	Clear();
	int N = ps->Size();
	for (int i=0; i<N; ++i)
	{
		GModifier* pm = ps->Modifier(i);
		GModifier* pmnew = 0;
		if      (dynamic_cast<GTwistModifier*>(pm)) pmnew = new GTwistModifier;
		else if (dynamic_cast<GBendModifier*> (pm)) pmnew = new GBendModifier;
		else if (dynamic_cast<GSkewModifier*> (pm)) pmnew = new GSkewModifier;
		else if (dynamic_cast<GWrapModifier*> (pm)) pmnew = new GWrapModifier;
		assert(pmnew);
		if (pmnew)
		{
			pmnew->GetParamBlock() = pm->GetParamBlock();
			Add(pmnew);
		}
	}
}

//=============================================================================
// GTwistModifier
//-----------------------------------------------------------------------------
GTwistModifier::GTwistModifier(FSModel* ps)
{
	AddIntParam(2, "orientation", "orientation")->SetEnumNames("X\0Y\0Z\0");
	AddDoubleParam(0, "twist", "twist");
	AddDoubleParam(1.0, "scale", "scale");
	AddDoubleParam(0, "min", "min");
	AddDoubleParam(1, "max", "max");
}

//-----------------------------------------------------------------------------
void GTwistModifier::Apply(GObject* po)
{
	int m = GetIntValue(ORIENT);
	double w = GetFloatValue(TWIST);
	double h = GetFloatValue(SCALE);
	double smin = GetFloatValue(SMIN);
	double smax = GetFloatValue(SMAX);

	if (h == 0) h = 1;
	if (m < 0) m = 0;
	if (m > 2) m = 2;

	double ca, sa, a, t;
	int N = po->Nodes();

	BOX box = po->GetLocalBox();
	double dx = box.Width(); if (dx == 0) dx = 1;
	double dy = box.Height(); if (dy == 0) dy = 1;
	double dz = box.Depth(); if (dz == 0) dz = 1;

	vec3d rc = box.Center();

	switch (m)
	{
	case 0: // X
		{
			for (int i=0; i<N; ++i)
			{
				GNode& n = *po->Node(i);
				t = (n.LocalPosition().x - box.x0) / dx;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dx/h;

				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = n.LocalPosition() - rc;

				n.LocalPosition().y = r.y*ca + r.z*sa + rc.y;
				n.LocalPosition().z = -r.y*sa + r.z*ca + rc.z;
			}
		}
		break;
	case 1: // Y
		{
			for (int i=0; i<N; ++i)
			{
				GNode& n = *po->Node(i);
				t = (n.LocalPosition().y - box.y0) / dy;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dy/h;

				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = n.LocalPosition() - rc;

				n.LocalPosition().x = r.x*ca + r.z*sa + rc.x;
				n.LocalPosition().z = -r.x*sa + r.z*ca + rc.z;
			}
		}
		break;
	case 2:	// Z
		{
			for (int i=0; i<N; ++i)
			{
				GNode& n = *po->Node(i);
				t = (n.LocalPosition().z - box.z0) / dz;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dz/h;
				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = n.LocalPosition() - rc;

				n.LocalPosition().x = r.x*ca + r.y*sa + rc.x;
				n.LocalPosition().y = -r.x*sa + r.y*ca + rc.y;
			}
		}
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
GMesh* GTwistModifier::BuildGMesh(GObject* po)
{
	GMesh* pm = po->GetRenderMesh();

	int m = GetIntValue(ORIENT);
	double w = GetFloatValue(TWIST);
	double h = GetFloatValue(SCALE);
	double smin = GetFloatValue(SMIN);
	double smax = GetFloatValue(SMAX);

	if (h == 0) h = 1;
	if (m < 0) m = 0;
	if (m > 2) m = 2;

	double ca, sa, a, t;
	int N = pm->Nodes();

	BOX box = pm->GetBoundingBox();
	double dx = box.Width(); if (dx == 0) dx = 1;
	double dy = box.Height(); if (dy == 0) dy = 1;
	double dz = box.Depth(); if (dz == 0) dz = 1;

	vec3d rc = box.Center();

	switch (m)
	{
	case 0: // X
		{
			for (int i=0; i<N; ++i)
			{
				GMesh::NODE& n = pm->Node(i);
				t = (n.r.x - box.x0) / dx;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dx/h;

				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = to_vec3d(n.r) - rc;

				n.r.y =  r.y*ca + r.z*sa + rc.y;
				n.r.z = -r.y*sa + r.z*ca + rc.z;
			}
		}
		break;
	case 1: // Y
		{
			for (int i=0; i<N; ++i)
			{
				GMesh::NODE& n = pm->Node(i);
				t = (n.r.y - box.y0) / dy;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dy/h;

				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = to_vec3d(n.r) - rc;

				n.r.x =  r.x*ca + r.z*sa + rc.x;
				n.r.z = -r.x*sa + r.z*ca + rc.z;
			}
		}
		break;
	case 2:	// Z
		{
			for (int i=0; i<N; ++i)
			{
				GMesh::NODE& n = pm->Node(i);
				t = (n.r.z - box.z0) / dz;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dz/h;
				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = to_vec3d(n.r) - rc;

				n.r.x =  r.x*ca + r.y*sa + rc.x;
				n.r.y = -r.x*sa + r.y*ca + rc.y;
			}
		}
		break;
	default:
		assert(false);
	}

	pm->Update();

	return 0;
}

//-----------------------------------------------------------------------------
FSMesh* GTwistModifier::BuildFEMesh(GObject* po)
{
	FSMesh* pm = po->GetFEMesh();

	int m = GetIntValue(ORIENT);
	double w = GetFloatValue(TWIST);
	double h = GetFloatValue(SCALE);
	double smin = GetFloatValue(SMIN);
	double smax = GetFloatValue(SMAX);

	if (h == 0) h = 1;
	if (m < 0) m = 0;
	if (m > 2) m = 2;

	double ca, sa, a, t;
	int N = pm->Nodes();

	BOX box = pm->GetBoundingBox();
	double dx = box.Width(); if (dx == 0) dx = 1;
	double dy = box.Height(); if (dy == 0) dy = 1;
	double dz = box.Depth(); if (dz == 0) dz = 1;

	vec3d rc = box.Center();

	switch (m)
	{
	case 0: // X
		{
			for (int i=0; i<N; ++i)
			{
				FSNode& n = pm->Node(i);
				t = (n.r.x - box.x0) / dx;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dx/h;

				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = n.r - rc;

				n.r.y =  r.y*ca + r.z*sa + rc.y;
				n.r.z = -r.y*sa + r.z*ca + rc.z;
			}
		}
		break;
	case 1: // Y
		{
			for (int i=0; i<N; ++i)
			{
				FSNode& n = pm->Node(i);
				t = (n.r.y - box.y0) / dy;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dy/h;

				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = n.r - rc;

				n.r.x =  r.x*ca + r.z*sa + rc.x;
				n.r.z = -r.x*sa + r.z*ca + rc.z;
			}
		}
		break;
	case 2:	// Z
		{
			for (int i=0; i<N; ++i)
			{
				FSNode& n = pm->Node(i);
				t = (n.r.z - box.z0) / dz;
				t = (t < smin ? 0 : (t > smax ? smax - smin : t - smin));
				a = w*t*dz/h;
				ca = cos(a*2.0*PI);
				sa = sin(a*2.0*PI);

				vec3d r = n.r - rc;

				n.r.x =  r.x*ca + r.y*sa + rc.x;
				n.r.y = -r.x*sa + r.y*ca + rc.y;
			}
		}
		break;
	default:
		assert(false);
	}

	pm->UpdateMesh();

	return 0;
}


//=============================================================================
// GPinchModifier
//-----------------------------------------------------------------------------
GPinchModifier::GPinchModifier(FSModel* ps)
{
	AddDoubleParam(1.0, "scale", "scale");
	AddIntParam(0, "orientation", "orientation")->SetEnumNames("X\0Y\0Z\0");
}

//-----------------------------------------------------------------------------
void GPinchModifier::Apply(GObject* po)
{
	BOX box = po->GetLocalBox();
	vec3d c = box.Center();
	double W = 0.5*box.Width();

	double s = GetFloatValue(SCALE);
	int m = GetIntValue(ORIENT);

	double vec3d::*pa = 0;
	double vec3d::*pb = 0;

	switch (m)
	{
	case 0: pa = &vec3d::y; pb = &vec3d::x; break;
	case 1: pa = &vec3d::x; pb = &vec3d::y; break;
	case 2: pa = &vec3d::x; pb = &vec3d::z; break;
	}

	for (int i=0; i<po->Nodes(); ++i)
	{
		GNode& node = *po->Node(i);
		vec3d r = node.LocalPosition() - c;
		double w = fabs(r.*pa)/W;
		if (w>1) w = 1;
		w = w*w;
		r.*pb *= w + (1-w)*s;
		node.LocalPosition() = c + r;
	}
}

//-----------------------------------------------------------------------------
GMesh* GPinchModifier::BuildGMesh(GObject* po)
{
	GMesh* pm = po->GetRenderMesh();

	BOX box = pm->GetBoundingBox();
	vec3d c = box.Center();
	double W = 0.5*box.Width();

	double s = GetFloatValue(SCALE);
	int m = GetIntValue(ORIENT);

	double vec3d::*pa = 0;
	double vec3d::*pb = 0;

	switch (m)
	{
	case 0: pa = &vec3d::y; pb = &vec3d::x; break;
	case 1: pa = &vec3d::x; pb = &vec3d::y; break;
	case 2: pa = &vec3d::x; pb = &vec3d::z; break;
	}

	for (int i=0; i<pm->Nodes(); ++i)
	{
		GMesh::NODE& node = pm->Node(i);
		vec3d r = to_vec3d(node.r) - c;
		double w = fabs(r.*pa)/W;
		if (w>1) w = 1;
		w = w*w;
		r.*pb *= w + (1-w)*s;
		node.r = to_vec3f(c + r);
	}

	pm->Update();

	return 0;
}

//-----------------------------------------------------------------------------
FSMesh* GPinchModifier::BuildFEMesh(GObject* po)
{
	FSMesh* pm = po->GetFEMesh();

	BOX box = pm->GetBoundingBox();
	vec3d c = box.Center();
	double W = 0.5*box.Width();

	double s = GetFloatValue(SCALE);
	int m = GetIntValue(ORIENT);

	double vec3d::*pa = 0;
	double vec3d::*pb = 0;

	switch (m)
	{
	case 0: pa = &vec3d::y; pb = &vec3d::x; break;
	case 1: pa = &vec3d::x; pb = &vec3d::y; break;
	case 2: pa = &vec3d::x; pb = &vec3d::z; break;
	}

	for (int i=0; i<pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		vec3d r = node.r - c;
		double w = fabs(r.*pa)/W;
		if (w>1) w = 1;
		w = w*w;
		r.*pb *= w + (1-w)*s;
		node.r = c + r;
	}

	pm->UpdateMesh();

	return 0;
}


//=============================================================================
// GBendModifier
//-----------------------------------------------------------------------------
GBendModifier::GBendModifier()
{
	SetName("Bend");

	AddIntParam(2, "orientation", "orientation")->SetEnumNames("X\0Y\0Z\0");
	AddDoubleParam(0, "angle", "angle");
	AddDoubleParam(0, "min", "min");
	AddDoubleParam(1, "max", "max");
	AddDoubleParam(0, "x-pivot", "x-pivot");
	AddDoubleParam(0, "y-pivot", "y-pivot");
	AddDoubleParam(0, "z-pivot", "z-pivot");
}

//-----------------------------------------------------------------------------
void GBendModifier::UpdateParams()
{
	// get parameter values
	int m = GetIntValue(ORIENT);
	m_a = GetFloatValue(ANGLE);
	m_smin = GetFloatValue(SMIN) - 0.5;
	m_smax = GetFloatValue(SMAX) - 0.5;
	double px = GetFloatValue(XPIVOT);
	double py = GetFloatValue(YPIVOT);
	double pz = GetFloatValue(ZPIVOT);
	m_pvt = vec3d(px, py, pz);

	// check parameters
	if (m < 0) m = 0;
	if (m > 2) m = 2;

	// get the bounding box and its dimensions
	double dx = m_box.Width (); if (dx == 0) dx = 1;
	double dy = m_box.Height(); if (dy == 0) dy = 1;
	double dz = m_box.Depth (); if (dz == 0) dz = 1;

	m_rc = m_box.Center();

	// convert the angle to radians
	m_a *= DEG2RAD;
	if (fabs(m_a) < 1e-7) return;

	m_L = 0;
	switch (m)
	{
	case 0: 
		{
			m_q = quatd(vec3d(1,0,0), vec3d(0,0,1));
			if (dz >= dy) { m_L = dz; }
			else { m_L = dy; m_q *= quatd(-0.5*PI, vec3d(1,0,0)); }
		}
		break;
	case 1: 
		{
			m_q = quatd(vec3d(0,1,0), vec3d(0,0,-1)); 
			if (dx >= dz) { m_L = dx; }
			else { m_L = dz; m_q *= quatd(-0.5*PI, vec3d(0,1,0)); }
		}
		break; 
	case 2: 
		if (dx >= dy) { m_L = dx; m_q = quatd(      0, vec3d(0,0,1)); }
		else          { m_L = dy; m_q = quatd( PI*0.5, vec3d(0,0,1)); }
		break; 
	}

	// determine the radius of curvature
	m_R0 = m_L / m_a;
}

//-----------------------------------------------------------------------------
void GBendModifier::Apply(GObject *po)
{
	// set the bounding box
	m_box = po->GetLocalBox();

	// update parameters
	UpdateParams();

	// apply modifier to GObject
	int N = po->Nodes();
	vec3d r;
	for (int i=0; i<N; ++i)
	{
		GNode& node = *po->Node(i);
		r = node.LocalPosition() - m_box.Center();
		Apply(r);
		node.LocalPosition() = r + m_box.Center();
	}
	vec3d dr = m_box.Center() - m_rc;
	for (int i = 0; i<N; ++i) po->Node(i)->LocalPosition() -= dr;
}

//-----------------------------------------------------------------------------
GMesh* GBendModifier::BuildGMesh(GObject *po)
{
	GMesh* pm = po->GetRenderMesh();

	// set the bounding box
	m_box = pm->GetBoundingBox();

	// update parameters
	UpdateParams();

	// apply modifier to GObject
	int N = pm->Nodes();
	for (int i=0; i<N; ++i)
	{
		GMesh::NODE& node = pm->Node(i);
		vec3d r = to_vec3d(node.r) - m_box.Center();
		Apply(r);
		node.r = to_vec3f(r + m_box.Center());
	}
	vec3d dr = m_box.Center() - m_rc;
	for (int i=0; i<N; ++i) pm->Node(i).r -= to_vec3f(dr);
	pm->Update();

	return 0;
}

//-----------------------------------------------------------------------------
FSMesh* GBendModifier::BuildFEMesh(GObject* po)
{
	FSMesh* pm = po->GetFEMesh();

	// set the bounding box
	m_box = pm->GetBoundingBox();

	// update parameters
	UpdateParams();

	// apply modifier to FSMesh
	int N = pm->Nodes();
	vec3d r;
	for (int i=0; i<N; ++i)
	{
		FSNode& node = pm->Node(i);
		r = node.r - m_box.Center();
		Apply(r);
		node.r = r + m_box.Center();
	}
	vec3d dr = m_box.Center() - m_rc;
	for (int i=0; i<N; ++i) pm->Node(i).r -= dr;

	pm->UpdateMesh();

	return 0;
}

//-----------------------------------------------------------------------------
void GBendModifier::Apply(vec3d& r)
{
	if (fabs(m_a) < 1e-7) return;

	m_q.RotateVector(r);

	r -= m_pvt;

	double t = r.x/ m_L;
	double dt = 0;
	if (t < m_smin) dt = m_smin - t;
	if (t > m_smax) 
		{
			dt = m_smax - t;
	}
//	t += dt;

	double R1 = m_R0 - r.y;

	double w = m_a*t;
	double cw = cos(w);
	double sw = sin(w);

	r.x = R1*sw - m_L*dt*cw;
	r.y -= R1*cw + m_L*dt*sw - R1;

	r += m_pvt;

	m_q.Inverse().RotateVector(r);
}

//=============================================================================
// GSkewModifier
//-----------------------------------------------------------------------------
GSkewModifier::GSkewModifier(FSModel* ps)
{
	AddIntParam(0, "orientation", "Orientation")->SetEnumNames("X\0Y\0Z\0");
	AddDoubleParam(0, "distance", "Skew distance");
}

//-----------------------------------------------------------------------------
void GSkewModifier::Apply(GObject* po)
{
	// get parameters
	int m = GetIntValue(ORIENT);
	double a = GetFloatValue(SKEW);

	// get the bounding box and its dimensions
	BOX box = po->GetLocalBox();
	double dx = box.Width(); if (dx == 0) dx = 1;
	double dy = box.Height(); if (dy == 0) dy = 1;
	double dz = box.Depth(); if (dz == 0) dz = 1;

	vec3d rc = box.Center();

	vec3d r;
	double (vec3d::*pl) = 0;
	double (vec3d::*pr) = 0;
	double d = 0;
	
	switch (m)
	{
	case 0: pl = &vec3d::x; pr = &vec3d::y; d = dx; break;
	case 1: pl = &vec3d::y; pr = &vec3d::z; d = dy; break;
	case 2: pl = &vec3d::x; pr = &vec3d::z; d = dx; break;
	}

	for (int i=0; i<po->Nodes(); ++i)
	{
		GNode& node = *po->Node(i);
		r = node.LocalPosition() - rc;
		r.*pl += a*(r.*pr)*d;
		node.LocalPosition() = r + rc;
	}
}

//-----------------------------------------------------------------------------
GMesh* GSkewModifier::BuildGMesh(GObject* po)
{
	GMesh* pm = po->GetRenderMesh();

	// get parameters
	int m = GetIntValue(ORIENT);
	double a = GetFloatValue(SKEW);

	// get the bounding box and its dimensions
	BOX box = pm->GetBoundingBox();
	double dx = box.Width(); if (dx == 0) dx = 1;
	double dy = box.Height(); if (dy == 0) dy = 1;
	double dz = box.Depth(); if (dz == 0) dz = 1;

	vec3d rc = box.Center();

	double (vec3d::*pl) = 0;
	double (vec3d::*pr) = 0;
	double d = 0;
	
	switch (m)
	{
	case 0: pl = &vec3d::x; pr = &vec3d::y; d = dx; break;
	case 1: pl = &vec3d::y; pr = &vec3d::z; d = dy; break;
	case 2: pl = &vec3d::x; pr = &vec3d::z; d = dx; break;
	}

	for (int i=0; i<pm->Nodes(); ++i)
	{
		GMesh::NODE& node = pm->Node(i);
		vec3d r = to_vec3d(node.r) - rc;
		r.*pl += a*(r.*pr)*d;
		node.r = to_vec3f(r + rc);
	}

	pm->Update();

	return 0;
}

//-----------------------------------------------------------------------------
FSMesh* GSkewModifier::BuildFEMesh(GObject* po)
{
	FSMesh* pm = po->GetFEMesh();

	// get parameters
	int m = GetIntValue(ORIENT);
	double a = GetFloatValue(SKEW);

	// get the bounding box and its dimensions
	BOX box = pm->GetBoundingBox();
	double dx = box.Width(); if (dx == 0) dx = 1;
	double dy = box.Height(); if (dy == 0) dy = 1;
	double dz = box.Depth(); if (dz == 0) dz = 1;

	vec3d rc = box.Center();

	vec3d r;

	double (vec3d::*pl) = 0;
	double (vec3d::*pr) = 0;
	double d = 0;
	
	switch (m)
	{
	case 0: pl = &vec3d::x; pr = &vec3d::y; d = dx; break;
	case 1: pl = &vec3d::y; pr = &vec3d::z; d = dy; break;
	case 2: pl = &vec3d::x; pr = &vec3d::z; d = dx; break;
	}

	for (int i=0; i<pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		r = node.r - rc;
		r.*pl += a*(r.*pr)*d;
		node.r = r + rc;
	}

	pm->UpdateMesh();

	return 0;
}
