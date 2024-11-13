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

// FEModifier.cpp: implementation of the FEModifier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEModifier.h"
#include "FENNQuery.h"
#include <MeshLib/FENodeNodeList.h>
#include <MeshLib/FENodeElementList.h>
#include <MeshLib/FEFaceEdgeList.h>
#include "FELinearToQuadratic.h"
#include "FESplitModifier.h"
#include <GeomLib/GObject.h>
#include <stdarg.h>
#include <FECore/units.h>
#include <MeshLib/FEMeshBuilder.h>
#include <GeomLib/GGroup.h>

std::string FEModifier::m_error;

FEModifier::FEModifier(const char* sz) { SetName(sz); }
FEModifier::~FEModifier() {}

FSMesh* FEModifier::Apply(FSMesh* pm) { return nullptr; }
FSMesh* FEModifier::Apply(FSGroup* pg) { return Apply(pg->GetMesh()); }
FSMesh* FEModifier::Apply(GObject* po, FESelection* sel)
{ 
	if ((po == nullptr) || (sel == nullptr)) return nullptr;
	FSMesh* oldMesh = po->GetFEMesh();
	if (oldMesh == nullptr) return nullptr;

	FSMesh* newMesh = nullptr;
	FEItemListBuilder* list(sel->CreateItemList());
	FSGroup* pg = dynamic_cast<FSGroup*>(list);
	if (pg && (pg->GetMesh() == oldMesh))
	{
		newMesh = Apply(pg);
	}
	delete list;
	return newMesh; 
}

FSMeshBase* FEModifier::ApplyModifier(FSMeshBase* pm) { return nullptr; }

bool FEModifier::SetError(const char* szerr, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char sz[256] = {0};
	va_start(args, szerr);
	vsprintf(sz, szerr, args);
	va_end(args);

	m_error = std::string(sz);
	return false;
}

std::string FEModifier::GetErrorString() 
{ 
	return m_error; 
}
	
//=============================================================================
// FEPartitionSelection
//-----------------------------------------------------------------------------

FEPartitionSelection::FEPartitionSelection() : FEModifier("Partition")
{
	AddBoolParam(true, "Create new Partition");
	AddIntParam(1, "Assign to partition")->SetState(Param_VISIBLE | Param_PERSISTENT);
}

bool FEPartitionSelection::UpdateData(bool bsave)
{
	if (bsave)
	{
		bool newPartition = GetBoolValue(0);
		if (newPartition)
			GetParam(1).SetState(Param_VISIBLE | Param_PERSISTENT);
		else
			GetParam(1).SetState(Param_VISIBLE | Param_EDITABLE | Param_PERSISTENT);
		return true;
	}
	return false;
}

FSMesh* FEPartitionSelection::Apply(FSMesh* pm)
{
	int gid = -1;
	bool newPartition = GetBoolValue(0);
	if (newPartition == false)
	{
		GObject* po = pm->GetGObject();
		int pid = GetIntValue(1);
		GPart* pg = po->FindPart(pid);
		if (pg == nullptr) return nullptr;
		gid = pg->GetLocalID();
	}

	FSMesh* newMesh = new FSMesh(*pm);
	FEMeshBuilder meshBuilder(*newMesh);
	meshBuilder.PartitionElementSelection(gid);

	return newMesh;
}

FSMesh* FEPartitionSelection::Apply(FSGroup* pg)
{
	FSMesh* oldMesh = pg->GetMesh();
	if (oldMesh == 0) return 0;

	int gid = -1;
	bool newPartition = GetBoolValue(0);
	if (newPartition == false)
	{
		GObject* po = oldMesh->GetGObject();
		int pid = GetIntValue(1);
		GPart* pg = po->FindPart(pid);
		if (pg == nullptr) return nullptr;
		gid = pg->GetLocalID();
	}

	FSMesh* newMesh = new FSMesh(*oldMesh);
	FEMeshBuilder meshBuilder(*newMesh);

	FSSurface* s = dynamic_cast<FSSurface*>(pg);
	if (s)
	{
		meshBuilder.PartitionFaceSelection(gid);
	}

	FSElemSet* p = dynamic_cast<FSElemSet*>(pg);
	if (p)
	{
		meshBuilder.PartitionElementSelection(gid);
	}

	FSNodeSet* n = dynamic_cast<FSNodeSet*>(pg);
	if (n)
	{
		meshBuilder.PartitionNodeSet(n);
	}

	FSEdgeSet* e = dynamic_cast<FSEdgeSet*>(pg);
	if (e)
	{
		meshBuilder.PartitionEdgeSelection(gid);
	}

	return newMesh;
}

//=============================================================================
// FERemoveDuplicateElements
//-----------------------------------------------------------------------------

FSMesh* FERemoveDuplicateElements::Apply(FSMesh* pm)
{
	int i, j, k;

	FSMesh* pnm = new FSMesh(*pm);
	FSMesh& m = *pnm;

	FSNodeElementList NEL; NEL.Build(&m);
	int NE = m.Elements();
	int NN = m.Nodes();
	for (i=0; i<NE; ++i) m.Element(i).m_ntag = i;

	// tag all duplicate elements
	for (i=0; i<NN; ++i)
	{
		int ne = NEL.Valence(i);
		for (j=0; j<ne; ++j)
		{
			FEElement_& ej = *NEL.Element(i, j);
			if (ej.m_ntag != -1)
			{
				for (k=j+1; k<ne; ++k)
				{
					FEElement_& ek = *NEL.Element(i, k);
					if ((ek.m_ntag!=-1) && (ej.is_equal(ek))) 
					{
						ej.m_ntag = -1;
						ek.m_ntag = -1;
					}
				}
			}
		}
	}

	// delete tagged elements
	FEMeshBuilder meshBuilder(m);
	meshBuilder.DeleteTaggedElements(-1);

	return pnm;
}

//////////////////////////////////////////////////////////////////////
// FEFlattenFaces
//////////////////////////////////////////////////////////////////////

FSMesh* FEFlattenFaces::Apply(FSMesh *pm)
{
	int i, j, ntag;

	// create a new mesh
	FSMesh* pnm = new FSMesh(*pm);
	FSMesh& m = *pnm;

	// tag all nodes
	for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;

	for (i=0; i<m.Faces(); ++i)
	{
		FSFace& f = m.Face(i);
		if (f.IsSelected())
		{
			for (j=0; j<f.Nodes(); ++j) m.Node(f.n[j]).m_ntag = 1;
		}
	}

	// calculate the average normal
	vec3d na;
	if (!m_bun)
	{
		for (i=0; i<m.Faces(); ++i)
		{
			FSFace& f = m.Face(i);
			if (f.IsSelected()) na += to_vec3d(f.m_fn);
		}
	}
	else na = m_na;

	// make sure our vector is normal
	na.Normalize();

	BOX box = m.GetBoundingBox();
	double R = box.GetMaxExtent();

	// find the lowest point
	vec3d p = box.Center() + na*R*2;
	double d0 = na*p, d;
	for (i=0, ntag = 0; i<m.Nodes(); ++i)
	{
		FSNode& n = m.Node(i);
		if (n.m_ntag)
		{
			d = na*n.r;
			if (d < d0) 
			{
				p = n.r;
				d0 = d;
			}
			++ntag;
		}
	}

	std::vector<int> tag(ntag);
	std::vector<double>	wgt(m.Nodes());
	for (i=0, ntag = 0; i<m.Nodes(); ++i)
	{
		if (m.Node(i).m_ntag) tag[ntag++] = i;
	}

	// find the distance to project for the tagged nodes
	for (i=0; i<m.Nodes(); ++i)
	{
		FSNode& n = m.Node(i);
		if (n.m_ntag) wgt[i] = na*(n.r - p);
	}

	// find the distance to project for the non-tagged nodes
	for (i=0; i<m.Nodes(); ++i)
	{
		FSNode& n = m.Node(i);
		if (n.m_ntag == 0)
		{
			// find the closest tagged node
			vec3d& r0 = n.r;
			double dmin = 1e300;
			int jmin = -1;
			for (j=0; j<ntag; ++j)
			{
				vec3d& r1 = m.Node(tag[j]).r;
				d = (r1-r0)*(r1-r0);
				if (d < dmin) { dmin = d; jmin = tag[j]; }
			}

			assert(jmin != -1);

			double f = 1 - sqrt(dmin)/m_rad;

			wgt[i] = wgt[jmin]*f;
			if (wgt[i] < 0) wgt[i] = 0;
		}
	}

	// project all nodes
	for (i=0; i<m.Nodes(); ++i)
	{
		FSNode& node = m.Node(i);
		if (wgt[i] > 0.0) node.r -= na*wgt[i];
	}

	// update geometry
	m.UpdateMesh();

	return pnm;
}

//=============================================================================
// FEAlignNodes
//=============================================================================

FEAlignNodes::FEAlignNodes() : FEModifier("Align")
{
	AddChoiceParam(0, "align", "align")->SetEnumNames("+X\0-X\0+Y\0-Y\0+Z\0-Z\0");
}

FSMesh* FEAlignNodes::Apply(FSMesh* pm)
{
	int nalign = GetIntValue(0);

	FSMesh* pnm = new FSMesh(*pm);

	vec3d rc;
	int iref = -1;
	for (int i=0; i<pnm->Nodes(); ++i)
	{
		FSNode& node = pnm->Node(i);
		vec3d ri = node.pos();
		if (node.IsSelected())
		{
			if (iref == -1) 
			{
				iref = i;
				rc = ri;
			}
			else
			{
				switch (nalign)
				{
				case 0: if (ri.x > rc.x) rc.x = ri.x; break;
				case 1: if (ri.x < rc.x) rc.x = ri.x; break;
				case 2: if (ri.y > rc.y) rc.y = ri.y; break;
				case 3: if (ri.y < rc.y) rc.y = ri.y; break;
				case 4: if (ri.z > rc.z) rc.z = ri.z; break;
				case 5: if (ri.z < rc.z) rc.z = ri.z; break;
				}
			}
		}
	}

	if (iref == -1) { delete pnm; return 0; }

	for (int i = 0; i<pnm->Nodes(); ++i)
	{
		FSNode& node = pnm->Node(i);
		if (node.IsSelected())
		{
			vec3d ri = node.pos();

			switch (nalign)
			{
			case 0:
			case 1: ri.x = rc.x; break;
			case 2:
			case 3: ri.y = rc.y; break;
			case 4:
			case 5: ri.z = rc.z; break;
			}

			node.pos(ri);
		}
	}

	return pnm;
}


//////////////////////////////////////////////////////////////////////
// FESetShellThickness
//////////////////////////////////////////////////////////////////////

FESetShellThickness::FESetShellThickness() : FEModifier("Set shell thickness")
{
	AddDoubleParam(0, "h", "shell thickness");
}

FSMesh* FESetShellThickness::Apply(FSMesh *pm)
{
	FSMesh* pnm = new FSMesh(*pm);

	double thick = GetFloatValue(0);
	double percent = 0;	// TODO: Add a parameter for this

	// tag selected elements
	int nselect = 0;
	for (int i = 0; i < pnm->Elements(); ++i)
	{
		FSElement& el = pnm->Element(i);
		if (el.IsSelected()) {
			el.m_ntag = 1; nselect++;
		}
		else el.m_ntag = 0;
	}

	// if no elements are selected, let's tag the entire mesh
	if (nselect == 0) pnm->TagAllElements(1);

	if (thick != 0 )
	{
		for (int i=0; i<pnm->Elements(); ++i)
		{
			FSElement& el = pnm->Element(i);
			if (el.m_ntag == 1)
			{
				double* h = el.m_h;
				for (int j=0; j<el.Nodes(); ++j) h[j] = thick;
			}
		}
	}
	else if (percent != 0)
	{
		for (int i=0; i<pnm->Elements(); ++i)
		{
			FSElement& el = pnm->Element(i);
			if (el.m_ntag == 1)
			{
				double* h = el.m_h;
				double H = h[0] * percent;
				for (int j=0; j<el.Nodes(); ++j) h[j] = H;
			}
		}
	}

	return pnm;
}

//------------------------------------------------------------------------
// FESetFiberOrientation
//------------------------------------------------------------------------

FESetFiberOrientation::FESetFiberOrientation() : FEModifier("Set fiber orientation")
{
	AddChoiceParam(0, "generator")->SetEnumNames("vector\0node numbering\0");
	AddVecParam(vec3d(1,0,0), "vector");
	AddIntParam(0, "n0")->SetState(0);
	AddIntParam(1, "n1")->SetState(0);
}

bool FESetFiberOrientation::UpdateData(bool bsave)
{
	if (bsave)
	{
		int n = GetIntValue(0);
		switch (n)
		{
		case 0:
			GetParam(1).SetState(Param_ALLFLAGS);
			GetParam(2).SetState(0);
			GetParam(3).SetState(0);
			break;
		case 1:
			GetParam(1).SetState(0);
			GetParam(2).SetState(Param_ALLFLAGS);
			GetParam(3).SetState(Param_ALLFLAGS);
			break;
		}
		return true;
	}

	return false;
}

FSMesh* FESetFiberOrientation::Apply(FSMesh *pm)
{
	FSMesh* pnm = new FSMesh(*pm);

	int ngen = GetIntValue(0);
	switch (ngen)
	{
	case 0: SetFiberVector(pnm); break;
	case 1: SetFiberNodes (pnm); break;
	default:
		delete pnm;
		assert(false);
		return nullptr;
	}

	return pnm;
}

void FESetFiberOrientation::SetFiberVector(FSMesh *pm)
{
	vec3d r = GetVecValue(1);
	r.Normalize();

	int nsel = 0;
	for (int i=0; i<pm->Elements(); ++i)
		if (pm->Element(i).IsSelected()) nsel++;

	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsSelected() || (nsel==0))
			el.m_fiber = r;
	}
}

void FESetFiberOrientation::SetFiberNodes(FSMesh *pm)
{
	int nsel = 0;
	for (int i = 0; i<pm->Elements(); ++i)
		if (pm->Element(i).IsSelected()) nsel++;

	vec3d r1, r2, n;
	int node0 = GetIntValue(2);
	int node1 = GetIntValue(3);
	for (int i = 0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsSelected() || (nsel == 0))
		{
			r1 = pm->Node(el.m_node[ node0 ]).r;
			r2 = pm->Node(el.m_node[ node1 ]).r;
			n = r2 - r1;
			n.Normalize();
			el.m_fiber = n;
		}
	}
}

//------------------------------------------------------------------------
// FESetAxesOrientation
//------------------------------------------------------------------------

FESetAxesOrientation::FESetAxesOrientation() : FEModifier("Set axes orientation")
{
	AddChoiceParam(0, "generator")->SetEnumNames("vector\0node numbering\0angles\0cylindrical\0<none>\0");
	AddVecParam(vec3d(1, 0, 0), "a");
	AddVecParam(vec3d(0, 1, 0), "d");
	AddIntParam(1, "n0")->SetState(0);
	AddIntParam(2, "n1")->SetState(0);
	AddIntParam(4, "n2")->SetState(0);
    AddScienceParam(0, UNIT_DEGREE, "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi");
}

bool FESetAxesOrientation::UpdateData(bool bsave)
{
	if (bsave)
	{
		int n = GetIntValue(0);

		switch (n)
		{
		case 0: 
		case 3:
			GetParam(1).SetState(Param_ALLFLAGS);
			GetParam(2).SetState(Param_ALLFLAGS);
			GetParam(3).SetState(0);
			GetParam(4).SetState(0);
			GetParam(5).SetState(0);
            GetParam(6).SetState(0);
            GetParam(7).SetState(0);
			break;
		case 1:
			GetParam(1).SetState(0);
			GetParam(2).SetState(0);
			GetParam(3).SetState(Param_ALLFLAGS);
			GetParam(4).SetState(Param_ALLFLAGS);
			GetParam(5).SetState(Param_ALLFLAGS);
            GetParam(6).SetState(0);
            GetParam(7).SetState(0);
			break;
        case 2:
            GetParam(1).SetState(0);
            GetParam(2).SetState(0);
            GetParam(3).SetState(0);
            GetParam(4).SetState(0);
            GetParam(5).SetState(0);
            GetParam(6).SetState(Param_ALLFLAGS);
            GetParam(7).SetState(Param_ALLFLAGS);
            break;
		case 4:
			GetParam(1).SetState(0);
			GetParam(2).SetState(0);
			GetParam(3).SetState(0);
			GetParam(4).SetState(0);
			GetParam(5).SetState(0);
			GetParam(6).SetState(0);
			GetParam(7).SetState(0);
			break;
		default:
			return false;
		}

		return true;
	}
	else return false;
}

FSMesh* FESetAxesOrientation::Apply(FSMesh *pm)
{
	FSMesh* pnm = new FSMesh(*pm);

	pnm->TagAllElements(-1);
	int nsel = 0;
	for (int i=0; i<pnm->Elements(); ++i)
	{
		FSElement& el = pnm->Element(i);
		if (el.IsSelected())
		{
			el.m_ntag = 1;
			nsel++;
		}
	}
	if (nsel == 0) pnm->TagAllElements(1);

	int ngen = GetIntValue(0);
	bool bret = false;
	switch (ngen)
	{
		case 0: bret = SetAxesVectors(pnm); break;
		case 1: bret = SetAxesNodes(pnm); break;
        case 2: bret = SetAxesAngles(pnm); break;
		case 3: bret = SetAxesCylindrical(pnm); break;
		case 4: bret = ClearAxes(pnm); break;
//		case 2: SetAxesCopy  (pnm); break;
		default:
			assert(false);
	}

	if (bret == false)
	{	
		delete pnm;
		pnm = 0;
	}
	
	return pnm;
}

bool FESetAxesOrientation::SetAxesVectors(FSMesh *pm)
{
	vec3d a = GetVecValue(1);
	vec3d d = GetVecValue(2);

	vec3d c = a^d;
	vec3d b = c^a;
	a.Normalize();
	b.Normalize();
	c.Normalize();
	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			mat3d& m = el.m_Q;
			m.zero();
			m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
			m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
			m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;
			el.m_Qactive = true;
		}
	}

	return true;
}

bool FESetAxesOrientation::SetAxesNodes(FSMesh *pm)
{
	int node[3] = {0, 1, 2};
	node[0] = GetIntValue(3)-1;
	node[1] = GetIntValue(4)-1;
	node[2] = GetIntValue(5)-1;

	vec3d r1, r2, r3, a, b, c, d;
	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			int neln = el.Nodes();
			if ((node[0] < 0) || (node[0] >= neln)) return false;
			if ((node[1] < 0) || (node[1] >= neln)) return false;
			if ((node[2] < 0) || (node[2] >= neln)) return false;

			r1 = pm->Node(el.m_node[ node[0] ]).r;
			r2 = pm->Node(el.m_node[ node[1] ]).r;
			r3 = pm->Node(el.m_node[ node[2] ]).r;
			a = r2 - r1;
			d = r3 - r1;
			c = a^d;
			b = c^a;
			a.Normalize();
			b.Normalize();
			c.Normalize();
			mat3d& m = el.m_Q;
			m.zero();
			m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
			m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
			m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;
			el.m_Qactive = true;
		}
	}

	return true;
}

bool FESetAxesOrientation::SetAxesAngles(FSMesh *pm)
{
    double theta = GetFloatValue(6)*PI/180;
    double phi = GetFloatValue(7)*PI/180;

    for (int i=0; i<pm->Elements(); ++i)
    {
        FSElement& el = pm->Element(i);
        if (el.m_ntag == 1)
        {
            mat3d& m = el.m_Q;
            m.zero();
            m[0][0] = sin(phi)*cos(theta); m[0][1] = -sin(theta); m[0][2] = -cos(phi)*cos(theta);
            m[1][0] = sin(phi)*sin(theta); m[1][1] = cos(theta);  m[1][2] = -cos(phi)*sin(theta);
            m[2][0] = cos(phi);            m[2][1] = 0;           m[2][2] = sin(phi);
            el.m_Qactive = true;
        }
    }

    return true;
}

bool FESetAxesOrientation::SetAxesCylindrical(FSMesh* pm)
{
	vec3d c(0, 0, 0); // center of axis

	vec3d a = GetVecValue(1); // axis
	vec3d d = GetVecValue(2); // reference vector
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			vec3d p = pm->ElementCenter(el);

			// find the vector to the axis
			vec3d b = (p - c) - a * (a*(p - c)); b.Normalize();

			// setup the rotation vector
			vec3d x_unit(vec3d(1, 0, 0));
			quatd q(x_unit, b);

			// rotate the reference vector
			vec3d r(d); r.Normalize();
			q.RotateVector(r);

			// setup a local coordinate system with r as the x-axis
			vec3d d(0, 1, 0);
			q.RotateVector(d);
			if (fabs(d*r) > 0.99)
			{
				d = vec3d(0, 0, 1);
				q.RotateVector(d);
			}

			// find basis vectors
			vec3d e1 = r;
			vec3d e3 = (e1 ^ d); e3.Normalize();
			vec3d e2 = e3 ^ e1;

			// setup rotation matrix
			mat3d& Q = el.m_Q;
			Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
			Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
			Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

			el.m_Qactive = true;
		}
	}

	return true;
}

bool FESetAxesOrientation::ClearAxes(FSMesh* pm)
{
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			el.m_Qactive = false;
		}
	}

	return true;
}

bool FESetAxesOrientation::SetAxesCopy(FSMesh *pm)
{
/*	assert(m_pms);
	
	int i, j, n;
	
	// create the array of source data
	int NY = m_pms->Elements();
	vector<vec3d> Y(NY);
	for (i=0; i<NY; ++i)
	{
		FSElement& el = m_pms->Element(i);
		n = el.Nodes();
		vec3d c(0,0,0);
		for (j=0; j<n; ++j) c += m_pms->Node(el.m_node[j]).r;
		c /= n;
		Y[i] = c;
	}
	
	// set up the nearest node search
	FSNNQuery q(&Y);
	q.Init();
	
	// do the mapping
	int N = pm->Elements();
	for (i=0; i<N; ++i)
	{
		FSElement& el = pm->Element(i);
		n = el.Nodes();
		vec3d c(0,0,0);
		for (j=0; j<n; ++j) c += pm->Node(el.m_node[j]).r;
		c /= n;
		n = q.Find(c);
		
		FSElement& els = m_pms->Element(n);
		el.m_Q = els.m_Q;
		el.m_Qactive = els.m_Qactive;
		
		// if the element is a shell, we project the fiber on the shell
		if (el.IsShell())
		{
			vec3d x[4];
			n = el.Nodes();
			for (j=0; j<n; ++j) x[j] = pm->Node(el.m_node[j]).r;
			vec3d e1, e2;
			if (n==4)
			{
				e1 = -x[0] + x[1] + x[2] - x[3];
				e2 = -x[0] - x[1] + x[2] + x[3];
			}
			else
			{
				e1 = x[1] - x[0];
				e2 = x[2] - x[0];
			}
			vec3d f = e1^e2;
			f.Normalize();
			
			el.m_fiber -= f*(f*el.m_fiber);
		}
	}
*/
	return true;
}

//=============================================================================
// FEMirrorMesh
//-----------------------------------------------------------------------------

FEMirrorMesh::FEMirrorMesh() : FEModifier("Mirror") 
{ 
	AddIntParam(0, "plane", "Mirror plane")->SetEnumNames("X-plane\0Y-plane\0Z-plane\0face selection\0custom\0");
	AddVecParam(vec3d(0,0,0), "center", "Center");
	AddVecParam(vec3d(1,0,0), "normal", "Normal")->SetVisible(false);
}

bool FEMirrorMesh::UpdateData(bool bsave)
{
	if (bsave)
	{
		Param& p = GetParam(2);
		int n = GetIntValue(0);
		if ((n == 4) && (p.IsVisible() == false))
		{
			p.SetVisible(true);
			return true;
		}
		else if ((n != 4) && p.IsVisible())
		{
			p.SetVisible(false);
			return true;
		}
	}
	return false;
}

FSMesh* FEMirrorMesh::Apply(FSMesh *pm)
{
	int nplane = GetIntValue(0);
	vec3d rc = GetVecValue(1);

	vec3d N;

	switch (nplane)
	{
	case 0: N = vec3d(1, 0, 0); break;
	case 1: N = vec3d(0, 1, 0); break;
	case 2: N = vec3d(0, 0, 1); break;
	case 3:
	{
		// for user selection we need to figure out a center and normal
		FEFaceSelection* sel = new FEFaceSelection(pm);
		if (sel->Count() == 0) return nullptr;
		BOX b = sel->GetBoundingBox();
		rc += b.Center();

		for (int i = 0; i < sel->Count(); ++i)
		{
			FSFace& face = *sel->Face(i);
			N += to_vec3d(face.m_fn);
		}
		N.Normalize();
	}
	break;
	case 4:
		N = GetVecValue(2);
		N.Normalize();
		if (N.norm() == 0.0) return nullptr;
		break;
	default:
		assert(false);
		return nullptr;
	}

	FSMesh* pmn = new FSMesh(*pm);

	// mirror the nodes
	for (int i=0; i<pmn->Nodes(); ++i)
	{
		FSNode& n = pmn->Node(i);
		vec3d r = n.r - rc;
		n.r = rc + (r - N*(2.0*(N*r)));
	}

	// invert elements
	FEMeshBuilder meshBuilder(*pmn);
	meshBuilder.InvertSelectedElements();

	return pmn;
}

//=============================================================================
// FEQuad2Tri
//-----------------------------------------------------------------------------

FSMesh* FEQuad2Tri::Apply(FSMesh *pm)
{
	int i;
	int NN = pm->Nodes();
	int NE0 = pm->Elements();
	int NF0 = pm->Faces();
	int NC = pm->Edges();

	// count number of elements
	int NE1 = 0;
	for (i=0; i<NE0; ++i)
	{
		if (pm->Element(i).IsType(FE_QUAD4)) NE1 += 2; else NE1++;
	}

	// count number of faces
	int NF1 = 0;
	for (i=0; i<NF0; ++i)
	{
		if (pm->Face(i).Nodes() == 4) NF1 += 2; else NF1++;
	}

	// create a new mesh
	FSMesh* pnew = new FSMesh;
	pnew->Create(NN, NE1, NF1, NC);

	// copy nodes
	for (i=0; i<NN; ++i) pnew->Node(i) = pm->Node(i);

	// copy edges
	for (i=0; i<NC; ++i) pnew->Edge(i) = pm->Edge(i);

	// copy elements
	NE1 = 0;
	for (i=0; i<NE0; ++i)
	{
		FSElement& e0 = pm->Element(i);
		if (e0.IsType(FE_QUAD4))
		{
			FSElement& e1 = pnew->Element(NE1++);
			FSElement& e2 = pnew->Element(NE1++);

			e1 = e0;
			e2 = e0;

			e1.SetType(FE_TRI3);
			e2.SetType(FE_TRI3);
			e1.m_node[0] = e0.m_node[0];
			e1.m_node[1] = e0.m_node[1];
			e1.m_node[2] = e0.m_node[2];
			e1.m_h[0] = e0.m_h[0];
			e1.m_h[1] = e0.m_h[1];
			e1.m_h[2] = e0.m_h[2];

			e2.m_node[0] = e0.m_node[2];
			e2.m_node[1] = e0.m_node[3];
			e2.m_node[2] = e0.m_node[0];
			e2.m_h[0] = e0.m_h[2];
			e2.m_h[1] = e0.m_h[3];
			e2.m_h[2] = e0.m_h[0];
		}
		else
		{
			FSElement& e1 = pnew->Element(NE1++);
			e1 = e0;
		}
	}

	// copy faces
	NF1 = 0;
	for (i=0; i<NF0; ++i)
	{
		FSFace& f0 = pm->Face(i);
		if (f0.Nodes() == 4)
		{
			FSFace& f1 = pnew->Face(NF1++);
			FSFace& f2 = pnew->Face(NF1++);

			f1 = f0;
			f2 = f0;

			f1.SetType(FE_FACE_TRI3);
			f2.SetType(FE_FACE_TRI3);

			f1.n[0] = f0.n[0];
			f1.n[1] = f0.n[1];
			f1.n[2] = f0.n[2];

			f2.n[0] = f0.n[2];
			f2.n[1] = f0.n[3];
			f2.n[2] = f0.n[0];
		}
		else
		{
			FSFace& f1 = pnew->Face(NF1++);
			f1 = f0;
		}
	}

	pnew->BuildMesh();

	return pnew;
}

//=============================================================================
// FETri2Quad
//-----------------------------------------------------------------------------

FSMesh* FETri2Quad::Apply(FSMesh* pm)
{
	assert(pm);
	if (pm == nullptr) return nullptr;

	// before we get started, let's make sure this is a tet4 mesh
	if (pm->IsType(FE_TRI3) == false) return nullptr;

	// build the edge tables
	FSEdgeList ET(*pm);
	FSElementEdgeList EET(*pm, ET);

	// create a new mesh
	int N0 = pm->Nodes();
	int F0 = pm->Elements();
	int E0 = ET.size();

	FSMesh* pnew = new FSMesh;
	int N1 = N0 + E0 + F0;
	int F1 = 3*F0;

	pnew->Create(N1, F1, 0, 0);

	// 1. BUILD NODES
	// copy the nodes from the mesh
	for (int i = 0; i < N0; ++i)
	{
		FSNode& n0 = pm->Node(i);
		FSNode& n1 = pnew->Node(i);
		n1.r = n0.r;
		n1.m_gid = n0.m_gid;
	}

	// add the edge nodes
	int n = N0;
	for (int i = 0; i < E0; ++i)
	{
		std::pair<int, int>& e0 = ET[i];
		FSNode& n1 = pnew->Node(n++);

		vec3d a = pm->Node(e0.first).r;
		vec3d b = pm->Node(e0.second).r;

		n1.r = (a + b) * 0.5;
	}

	// add the face nodes
	for (int i = 0; i < F0; ++i)
	{
		FSFace& f0 = pm->Face(i);
		FSNode& n1 = pnew->Node(n++);

		vec3d a = pm->Node(f0.n[0]).r;
		vec3d b = pm->Node(f0.n[1]).r;
		vec3d c = pm->Node(f0.n[2]).r;

		n1.r = (a + b + c) / 3.0;
	}

	// node lookup table
	const int NLT[3][4] = {
		{ 0, 3, 6, 5 },
		{ 1, 4, 6, 3 },
		{ 2, 5, 6, 4}
	};

	// create the new elements
	int ne = 0;
	for (int i = 0; i < F0; ++i)
	{
		FSElement& e0 = pm->Element(i);

		int n[7];
		int* en = e0.m_node;
		n[0] = en[0];
		n[1] = en[1];
		n[2] = en[2];
		n[3] = N0 + EET.EdgeIndex(i, 0);
		n[4] = N0 + EET.EdgeIndex(i, 1);
		n[5] = N0 + EET.EdgeIndex(i, 2);
		n[6] = N0 + E0 + i;

		for (int j = 0; j < 3; ++j)
		{
			FSElement& e1 = pnew->Element(ne++);

			e1.SetType(FE_QUAD4);
			e1.m_gid = e0.m_gid;
			for (int k = 0; k < 4; ++k) e1.m_node[k] = n[NLT[j][k]];
		}
	}

	// build the other mesh structures
	pnew->RebuildMesh();

	return pnew;
}

//=============================================================================
// RefineMesh
//-----------------------------------------------------------------------------

RefineMesh::RefineMesh() : FEModifier("Refine")
{
	AddIntParam(1, "iterations:", "iterations");
	AddBoolParam(false, "Smooth surface");
	AddBoolParam(false, "hex 2d-split");
}

FSMesh* RefineMesh::Apply(FSMesh* pm)
{
	// get parameters
	int niter = GetIntValue(0);
	bool bsmooth = GetBoolValue(1);
	if (niter < 1) return 0;

	bool bhex2d = GetBoolValue(2);

	// select a modifier
	FEModifier* mod = 0;
	if (pm->IsType(FE_QUAD4))
	{
		mod = new FEQuadSplitModifier;
	}
	if (pm->IsType(FE_TRI3))
	{
		mod = new FETriSplitModifier;
	}
	if (pm->IsType(FE_TET4))
	{
		mod = new FETetSplitModifier;
	}
	if (pm->IsType(FE_HEX8) && bhex2d)
	{
		FEHex2DSplitModifier* hexmod = new FEHex2DSplitModifier;
		hexmod->DoSurfaceSmoothing(bsmooth);
		mod = hexmod;
	}
	else
	{
		// The hex split modifier also works with selections.
		// So, we only need to test if the selected elements are hex8.
		int nsel = pm->CountSelectedElements();
		bool isHex = false;
		if ((nsel == 0) && pm->IsType(FE_HEX8)) isHex = true;
		if (nsel != 0)
		{
			isHex = true;
			for (int i = 0; i < pm->Elements(); ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.IsSelected() && !el.IsType(FE_HEX8))
				{
					isHex = false;
					break;
				}
			}
		}
		if (isHex)
		{
			FEHexSplitModifier* hexmod = new FEHexSplitModifier;
			hexmod->DoSurfaceSmoothing(bsmooth);
			mod = hexmod;
		}
	}
	if (mod == 0) return 0;

	// apply the refine modifier
	FSMesh* pmold = pm;
	setProgress(0.0);
	for (int i=0; i<niter; ++i)
	{
		FSMesh* pmnew = mod->Apply(pmold);
		if (i != 0) delete pmold;
		pmold = pmnew;

		if (pmold == nullptr)
		{
			string err = mod->GetErrorString();
			if (!err.empty()) SetError(err.c_str());
			break;
		}

		setProgress(100.0*(i + 1.0) / (double)niter);
	}

	delete mod;

	return pmold;
}

//=============================================================================
// FEConvertMesh
//-----------------------------------------------------------------------------

// NOTE: To add a new mesh convertor:
// 1. Add a new enum value in the enum below, but add it before the END_OF_LIST item. 
// 2. Add a new string in the FEConvertMeshOptions array in the same location
// 3. update the switch statement inside UpdateData.
// 4. Add the new option in the switch statement in Apply
// Make sure when calling buildMeshConvertOptions that the list parameter value is END_OF_LIST!
enum ConvertMeshOptions {
	QUAD4_TO_TRI3,
	HEX8_TO_TET4,
	TET4_TO_TET5,
	TET4_TO_TET10,
	TET4_TO_TET15,
	TET4_TO_TET20,
	TET4_TO_HEX8,
	TET5_TO_TET4,
	TET10_TO_TET4,
	TET15_TO_TET4,
	HEX8_TO_HEX20,
	HEX20_TO_HEX8,
	PENTA6_TO_TET4,
	PENTA6_TO_PENTA15,
	QUAD4_TO_QUAD8,
	QUAD8_TO_QUAD4,
	TRI3_TO_TRI6,
	TRI6_TO_TRI3,
	TRI3_TO_QUAD4,
	LINEAR_TO_QUADRATIC,
	QUADRATIC_TO_LINEAR,
	END_OF_LIST // this has to be the last item!
};

const char* FEConvertMeshOptions[] = {
		"Quad4 to Tri3",
		"Hex8 to Tet4",
		"Tet4 to Tet5",
		"Tet4 to Tet10",
		"Tet4 to Tet15",
		"Tet4 to Tet20",
		"Tet4 to Hex8",
		"Tet5 to Tet4",
		"Tet10 to Tet4",
		"Tet15 to Tet4",
		"Hex8 to Hex20",
		"Hex20 to Hex8",
		"Penta6 to Tet4",
		"Penta6 to Penta15",
		"Quad4 to Quad8",
		"Quad8 to Quad4",
		"Tri3 to Tri6",
		"Tri6 to Tri3",
		"Tri3 to Quad4",
		"Linear to Quadratic",
		"Quadratic to Linear" };

// function to build the enum string for the options parameter. Make sure that the last argument
// to this function is END_OF_LIST
char* buildMeshConvertOptions(char* sz, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	va_start(args, sz);
	int n = va_arg(args, int);
	while (n != END_OF_LIST)
	{
		const char* szn = FEConvertMeshOptions[n]; assert(szn);
		int l = strlen(szn);
		strcpy(sz, szn);
		sz += l + 1;
		n = va_arg(args, int);
	}
	*sz = 0;
	
	va_end(args);

	return sz;
}

void buildAllMeshConvertOptions(char* sz)
{
	char* tmp = sz;
	for (int i = 0; i < END_OF_LIST; ++i)
	{
		tmp = buildMeshConvertOptions(tmp, i, END_OF_LIST);
	}
}

int getModifierID(const char* szmod)
{
	for (int i = 0; i < END_OF_LIST; ++i)
	{
		const char* szn = FEConvertMeshOptions[i]; assert(szn);
		if (strcmp(szmod, szn) == 0) return i;
	}
	return -1;
}

FEConvertMesh::FEConvertMesh() : FEModifier("Convert")
{
	AddChoiceParam(0, "convert", "Convert");
	AddBoolParam(false, "smooth", "smooth surface");

	m_currentType = -2;
	UpdateData(true);
}

bool FEConvertMesh::UpdateData(bool bsave)
{
	GObject* po = GObject::GetActiveObject();
	if (po == nullptr) return false;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return false;

	// get the mesh type
	int meshType = pm->GetMeshType();

	if (meshType == m_currentType) return false;

	// get the enum params
	Param& p = GetParam(0);
	char sz[512] = { 0 };
	switch (meshType)
	{
	case FE_QUAD4: buildMeshConvertOptions(sz, QUAD4_TO_TRI3, QUAD4_TO_QUAD8, END_OF_LIST); break;
	case FE_HEX8 : buildMeshConvertOptions(sz, HEX8_TO_HEX20, HEX8_TO_TET4, END_OF_LIST); break;
	case FE_TET4 : buildMeshConvertOptions(sz, TET4_TO_HEX8, TET4_TO_TET10, TET4_TO_TET15, TET4_TO_TET20, TET4_TO_TET5, END_OF_LIST); break;
	case FE_TET5 : buildMeshConvertOptions(sz, TET5_TO_TET4, END_OF_LIST); break;
	case FE_TET10: buildMeshConvertOptions(sz, TET10_TO_TET4, END_OF_LIST); break;
	case FE_TET15: buildMeshConvertOptions(sz, TET15_TO_TET4, END_OF_LIST); break;
	case FE_HEX20: buildMeshConvertOptions(sz, HEX20_TO_HEX8, END_OF_LIST); break;
    case FE_PENTA6: buildMeshConvertOptions(sz, PENTA6_TO_TET4, PENTA6_TO_PENTA15, END_OF_LIST); break;
	case FE_QUAD8: buildMeshConvertOptions(sz, QUAD8_TO_QUAD4, END_OF_LIST); break;
	case FE_TRI3 : buildMeshConvertOptions(sz, TRI3_TO_TRI6, TRI3_TO_QUAD4, END_OF_LIST); break;
	case FE_TRI6 : buildMeshConvertOptions(sz, TRI6_TO_TRI3, END_OF_LIST); break;
	default:
		// add them all
		buildAllMeshConvertOptions(sz);
	}
	p.CopyEnumNames(sz);
	p.SetIntValue(0);

	m_currentType = meshType;

	return false;
}

FSMesh* FEConvertMesh::Apply(FSMesh* pm)
{
	// get the chosen option
	int nsel = GetIntValue(0);
	if (nsel < 0) return nullptr;

	// convert to string
	const char* sz = GetParam(0).GetEnumName(nsel);
	if (sz == nullptr) return nullptr;

	// get the corresponding modifier ID
	int nmod = getModifierID(sz);

	// get the smooth option
	bool bsmooth = GetBoolValue(1);

	// create the specific convertor
	m_mod = nullptr;
	switch (nmod)
	{
	case QUAD4_TO_TRI3 : m_mod = new FEQuad2Tri; break;
	case TRI3_TO_QUAD4 : m_mod = new FETri2Quad; break;
	case HEX8_TO_TET4  : m_mod = new FEHex2Tet; break;
	case TET4_TO_TET5  : m_mod = new FETet4ToTet5; break;
	case TET4_TO_TET10 : m_mod = new FETet4ToTet10(bsmooth); break;
	case TET4_TO_TET15 : m_mod = new FETet4ToTet15(bsmooth); break;
	case TET4_TO_TET20 : m_mod = new FETet4ToTet20; break;
	case TET4_TO_HEX8  : m_mod = new FETet4ToHex8(bsmooth); break;
	case TET5_TO_TET4  : m_mod = new FETet5ToTet4; break;
	case TET10_TO_TET4 : m_mod = new FETet10ToTet4; break;
	case TET15_TO_TET4 : m_mod = new FETet15ToTet4; break;
	case HEX8_TO_HEX20 : m_mod = new FEHex8ToHex20(bsmooth); break;
	case HEX20_TO_HEX8 : m_mod = new FEHex20ToHex8; break;
    case PENTA6_TO_TET4: m_mod = new FEPenta6ToTet4; break;
    case PENTA6_TO_PENTA15: m_mod = new FEPenta6ToPenta15; break;
	case QUAD4_TO_QUAD8: m_mod = new FEQuad4ToQuad8(bsmooth); break;
	case QUAD8_TO_QUAD4: m_mod = new FEQuad8ToQuad4; break;
	case TRI3_TO_TRI6  : m_mod = new FETri3ToTri6(bsmooth); break;
	case TRI6_TO_TRI3  : m_mod = new FETri6ToTri3; break;
	case LINEAR_TO_QUADRATIC: m_mod = new FELinearToQuadratic; break;
	case QUADRATIC_TO_LINEAR: m_mod = new FEQuadraticToLinear; break;
	default:
		FEModifier::SetError("Unknown converter selected");
		assert(false);
		return 0;
	}

	// apply it
	if (m_mod)
	{
		FSMesh* newMesh = m_mod->Apply(pm);
		FEModifier* tmp = m_mod; m_mod = nullptr;
		delete tmp;
		return newMesh;
	}
	else return 0;
}

// return progress
FSTaskProgress FEConvertMesh::GetProgress()
{
	if (m_mod) return m_mod->GetProgress();
	else return FEModifier::GetProgress();
}

//=============================================================================
// FEAddNode
//-----------------------------------------------------------------------------


FEAddNode::FEAddNode() : FEModifier("Add Node")
{
	AddVecParam(vec3d(0,0,0), "position", "position");
}

FSMesh* FEAddNode::Apply(FSMesh* pm)
{
	FSMesh* newMesh = new FSMesh(*pm);

	vec3d r = GetVecValue(0);
	GObject* po = pm->GetGObject();
	if (po) r = po->GetTransform().GlobalToLocal(r);

	FEMeshBuilder meshBuilder(*newMesh);
	meshBuilder.AddNode(r);

	return newMesh;
}

//=============================================================================
// FEAddTriangle
//-----------------------------------------------------------------------------
FEAddTriangle::FEAddTriangle() : FEModifier("Add Triangle")
{
	AddIntParam(0, "node0", "Node 1");
	AddIntParam(0, "node1", "Node 2");
	AddIntParam(0, "node2", "Node 3");
}

FSMesh* FEAddTriangle::Apply(FSMesh* pm)
{
	if (m_stack.empty())
	{
		int n0 = GetIntValue(0) - 1;
		int n1 = GetIntValue(1) - 1;
		int n2 = GetIntValue(2) - 1;

		int NN = pm->Nodes();
		if ((n0 < 0) || (n0 >= NN)) return nullptr;
		if ((n1 < 0) || (n1 >= NN)) return nullptr;
		if ((n2 < 0) || (n2 >= NN)) return nullptr;

		FSMesh* newMesh = new FSMesh(*pm);

		FEMeshBuilder meshBuilder(*newMesh);
		meshBuilder.AddTriangle(n0, n1, n2);

		return newMesh;
	}
	else
	{
		FSMesh* newMesh = new FSMesh(*pm);
		FEMeshBuilder meshBuilder(*newMesh);
		meshBuilder.AddTriangles(m_stack);
		return newMesh;
	}
}

void FEAddTriangle::push_stack()
{
	m_stack.push_back(GetIntValue(0) - 1);
	m_stack.push_back(GetIntValue(1) - 1);
	m_stack.push_back(GetIntValue(2) - 1);
}

void FEAddTriangle::pop_stack()
{
	if (m_stack.size() >= 3)
	{
		m_stack.pop_back();
		m_stack.pop_back();
		m_stack.pop_back();
	}
}

//=============================================================================
// FEInvertElements
//-----------------------------------------------------------------------------


FEInvertMesh::FEInvertMesh() : FEModifier("Invert Mesh")
{
	AddBoolParam(true, "Invert elements", "Invert elements");
	AddBoolParam(true, "Invert faces", "Invert faces");
}

FSMesh* FEInvertMesh::Apply(FSMesh* pm)
{
	bool invertElems = GetBoolValue(0);
	bool invertFaces = GetBoolValue(1);

	FSMesh* newMesh = new FSMesh(*pm);
	FEMeshBuilder meshBuilder(*newMesh);

	if (invertElems)
	{
		meshBuilder.InvertSelectedElements();
	}
	else if (invertFaces)
	{
		meshBuilder.InvertSelectedFaces();
	}
	else
	{
		delete newMesh;
		return nullptr;
	}

	return newMesh;
}

//=============================================================================
// FEDetachElements
//-----------------------------------------------------------------------------

FEDetachElements::FEDetachElements() : FEModifier("Detach Elements")
{
	AddBoolParam(true, "Repartition selection");
}

FSMesh* FEDetachElements::Apply(FSMesh* pm)
{
	// Figure out which nodes need to be duplicated
	pm->TagAllNodes(-1);
	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsSelected())
		{
			// solid element
			int nf = el.Faces();
			for (int j=0; j<nf; ++j)
			{
				int nbr = el.m_nbr[j];
				if (nbr >= 0)
				{
					FSElement& elj = pm->Element(nbr);
					if (elj.IsSelected() == false)
					{
						FSFace f = el.GetFace(j);
						int nf = f.Nodes();
						for (int k=0; k<nf; ++k) pm->Node(f.n[k]).m_ntag = 1;
					}
				}
			}

			// shell element
			int nc = el.Edges();
			for (int j=0; j<nc; ++j)
			{
				int nbr = el.m_nbr[j];
				if (nbr >= 0)
				{
					FSElement& elj = pm->Element(nbr);
					if (elj.IsSelected() == false)
					{
						FSEdge e = el.GetEdge(j);
						int ne = e.Nodes();
						for (int k = 0; k<ne; ++k) pm->Node(e.n[k]).m_ntag = 1;
					}
				}
			}
		}
	}

	// count the new nodes
	int NN = pm->Nodes();
	int nn1 = NN;
	for (int i=0; i<NN; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag == 1) 
		{
			node.m_ntag = nn1++;
		}
	}

	// create a new mesh
	FSMesh* newMesh = new FSMesh;
	newMesh->Create(nn1, NE);

	// create nodes
	for (int i=0; i<NN; ++i) 
	{
		FSNode& si = pm->Node(i);
		FSNode& di = newMesh->Node(i);
		di = si;
		if (si.m_ntag != -1)
		{
			FSNode& di2 = newMesh->Node(si.m_ntag);
			di2 = si;
		}
	}

	// create elements
	for (int i=0; i<NE; ++i)
	{
		FSElement& si = pm->Element(i);
		FSElement& di = newMesh->Element(i);
		di = si;

		if (si.IsSelected())
		{
			for (int j=0; j<si.Nodes(); ++j)
			{
				FSNode& nj = pm->Node(si.m_node[j]);
				if (nj.m_ntag != -1)
				{
					di.m_node[j] = nj.m_ntag;
				}
			}
		}
	}

	// see if we need to repartition
	bool repart = GetBoolValue(0);
	if (repart)
	{
		int ng = pm->CountElementPartitions();
		for (int i=0; i<NE; ++i)
		{
			FSElement& si = pm->Element(i);
			FSElement& di = newMesh->Element(i);

			if (si.IsSelected())
			{
				di.m_gid += ng;
			}
		}
	}

	// update the mesh
	newMesh->RebuildMesh();
	
	return newMesh;		
}
