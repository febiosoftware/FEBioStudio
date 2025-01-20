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
#include <FSCore/ParamBlock.h>
#include <FSCore/Serializable.h>
#include <FSCore/box.h>
#include <FSCore/FSObject.h>
#include <list>

class GObject;
class FSMesh;
class GLMesh;
class FSModel;

class GModifier : public FSObject
{
public:
	GModifier(void);
	virtual ~GModifier(void);

	virtual void Apply(GObject* po) = 0;
	virtual GLMesh* BuildGMesh(GObject* po) = 0;
	virtual FSMesh* BuildFEMesh(GObject* po) = 0; 
};

//-----------------------------------------------------------------------------
class GTwistModifier : public GModifier
{
public:
	enum { ORIENT, TWIST, SCALE, SMIN, SMAX };
public:
	GTwistModifier(FSModel* ps = 0);
	void Apply(GObject* po);
	virtual GLMesh* BuildGMesh(GObject* po);
	FSMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GPinchModifier : public GModifier
{
public:
	enum {SCALE, ORIENT};

public:
	GPinchModifier(FSModel* ps = 0);
	void Apply(GObject* po);
	virtual GLMesh* BuildGMesh(GObject* po);
	FSMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GBendModifier : public GModifier
{
public:
	enum { ORIENT, ANGLE, SMIN, SMAX, XPIVOT, YPIVOT, ZPIVOT };

public:
	GBendModifier();
	void Apply(GObject* po);
	virtual GLMesh* BuildGMesh(GObject* po);
	FSMesh* BuildFEMesh(GObject* po);

protected:
	void UpdateParams();
	void Apply(vec3d& r);

protected:
	BOX		m_box;
	vec3d	m_rc;
	vec3d	m_pvt;
	quatd	m_q;
	double	m_R0, m_L;
	double	m_smin, m_smax, m_a;
};

//-----------------------------------------------------------------------------
class GSkewModifier : public GModifier
{
public:
	enum { ORIENT, SKEW };

public:
	GSkewModifier(FSModel* ps = 0);
	void Apply(GObject* po);
	virtual GLMesh* BuildGMesh(GObject* po);
	FSMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GWrapModifier : public GModifier
{
public:
	enum { TRG_ID, METHOD, NSTEPS };
public:
	GWrapModifier();
	void Apply(GObject* po);
	virtual GLMesh* BuildGMesh(GObject* po) { return 0; }
	FSMesh* BuildFEMesh(GObject* po) { return 0; }
	void SetTarget(GObject* ptrg) { m_po = ptrg; }

protected:
	void ClosestPoint(GObject* po, std::vector<vec3d>& DS, std::vector<int>& tag);
	void NormalProjection(GObject* po, std::vector<vec3d>& DS, std::vector<int>& tag, int nsteps);

protected:
	GObject*	m_po;	//!< target object
};

//-----------------------------------------------------------------------------
class GExtrudeModifier : public GModifier
{
public:
	enum { DIST, NDIVS };

public:
	GExtrudeModifier();
	void Apply(GObject* po);
	GLMesh* BuildGMesh(GObject* po);
	FSMesh* BuildFEMesh(GObject* po);
};


//-----------------------------------------------------------------------------
class GRevolveModifier : public GModifier
{
public:
	enum { ANGLE, DIVS };

public:
	GRevolveModifier();
	void Apply(GObject* po);
	GLMesh* BuildGMesh(GObject* po);
	FSMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GModifierStack : public CSerializable
{
public:
	GModifierStack(GObject* po);
	~GModifierStack();

	GObject* GetOwner() { return m_po; }
	void SetOwner(GObject* po) { m_po = po; }

	void Add(GModifier* pmod) { m_Mod.push_back(pmod); }
	void Remove(GModifier* pmod);
	int Size() { return (int) m_Mod.size(); }

	FSMesh* GetFEMesh() { return m_pmesh; }
	void ClearMesh();

	void SetFEMesh(FSMesh* pm) { ClearMesh(); m_pmesh = pm; }

	GModifier* Modifier(int n)
	{
		std::list<GModifier*>::iterator pi = m_Mod.begin();
		for (int i=0; i<n; ++i) ++pi;
		return (*pi);
	}

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void Apply();

	void Clear();

	void Copy(GModifierStack* ps);

protected:
	GObject*	m_po;		// the object that owns this stack
	FSMesh*		m_pmesh;	// the original mesh

	std::list<GModifier*>	m_Mod;	// the actual modifier stack
};
