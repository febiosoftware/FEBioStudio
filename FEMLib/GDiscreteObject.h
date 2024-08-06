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
#include "FEDiscreteMaterial.h"
#include <GeomLib/GItem.h>

#define FE_DISCRETE_OBJECT		0
#define FE_DISCRETE_SPRING		1
#define FE_GENERAL_SPRING		2
#define FE_ARMATURE				3
#define FE_LINEAR_SPRING_SET	4	// obsolete
#define FE_NONLINEAR_SPRING_SET	5	// obsolete
#define FE_DISCRETE_ELEMENT		6
#define FE_DEFORMABLE_SPRING	7
#define FE_DISCRETE_SPRING_SET	8

class GModel;

class GDiscreteObject : public FSObject
{
public:
	GDiscreteObject(GModel* gm, int ntype);
	~GDiscreteObject(void);

	int GetType() { return m_ntype; }

	// check selection state
	bool IsSelected() const { return ((m_state & GEO_SELECTED) != 0); }
	virtual void Select() { m_state = m_state | GEO_SELECTED; }
	virtual void UnSelect() { m_state = m_state & ~GEO_SELECTED; }

	virtual void SelectComponent(int n) { Select(); }
	virtual void UnselectComponent(int n) { UnSelect(); }

	// get/set state
	unsigned int GetState() const { return m_state; }
	void SetState(unsigned int state) { m_state = state; }

	// check visibility state (only used by GBaseObject's)
	bool IsVisible() const { return ((m_state & GEO_VISIBLE) != 0); }
	void Show() { m_state = m_state | GEO_VISIBLE; }
	void Hide() { m_state = 0; }

	// check active status
	bool IsActive() const { { return ((m_state & GEO_ACTIVE) != 0); } }
	void SetActive(bool b) { if (b) m_state = m_state | GEO_ACTIVE;	else m_state = m_state & ~GEO_ACTIVE; }

	// get/set object color
	GLColor GetColor() const;
	void SetColor(const GLColor& c);

	// get the model
	const GModel* GetModel() const;
	GModel* GetModel();

protected:
	int		m_ntype;
	int		m_state;
	GLColor	m_col;
	GModel*	m_gm;
};

//-----------------------------------------------------------------------------

class GLinearSpring : public GDiscreteObject
{
public:
	enum { MP_E };

public:
	GLinearSpring(GModel* gm);
	GLinearSpring(GModel* gm, int n1, int n2);

	void Save(OArchive& ar);
	void Load(IArchive& ar);
	
public:
	int	m_node[2];	// the two nodes
};

//-----------------------------------------------------------------------------

class GGeneralSpring : public GDiscreteObject
{
public:
	enum { MP_F };

public:
	GGeneralSpring(GModel* gm);
	GGeneralSpring(GModel* gm, int n1, int n2);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	int	m_node[2];
};

//-----------------------------------------------------------------------------
// A discrete element can only be defined as a member of a discrete element set
class GDiscreteElement : public GDiscreteObject
{
public:
	GDiscreteElement(GModel* gm);
	GDiscreteElement(GModel* gm, int n0, int n1);
	GDiscreteElement(const GDiscreteElement& el);
	void operator = (const GDiscreteElement& el);

	int GetID() const { return m_nid; }

	void SetNodes(int n0, int n1);

public:
	const int& Node(int n) const { return m_node[n]; }

protected:
	int		m_nid;
	int		m_node[2];

	static	int	m_ncount;
};

//-----------------------------------------------------------------------------
// base class for discrete element sets
class GDiscreteElementSet : public GDiscreteObject
{
public:
	GDiscreteElementSet(GModel* gm, int ntype);
	~GDiscreteElementSet();

	int size() const;

	GDiscreteElement& element(int i);

	void AddElement(int n0, int n1);
	void AddElement(GNode* node0, GNode* node2);

	void AddElement(const GDiscreteElement& el);

	void RemoveElement(int index);

	int FindElement(const GDiscreteElement& el) const;
	int FindElement(int id) const;

	void UnSelect();
	void Select();

	void SelectComponent(int n);
	void UnselectComponent(int n);

	void Clear();

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	vector<GDiscreteElement*>	m_elem;
};

//-----------------------------------------------------------------------------
class GDiscreteSpringSet : public GDiscreteElementSet
{
public:
	GDiscreteSpringSet(GModel* gm);

	void CopyDiscreteElementSet(GDiscreteElementSet* ds);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void SetMaterial(FSDiscreteMaterial* mat);
	FSDiscreteMaterial* GetMaterial();

private:
	FSDiscreteMaterial*	m_mat;
};

//-----------------------------------------------------------------------------
// NOTE: This class is obsolete. Maintained for backward compatibility
class GLinearSpringSet : public GDiscreteElementSet
{
public:
	enum { MP_E };

public:
	GLinearSpringSet(GModel* gm);

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
// NOTE: This class is obsolete. Maintained for backward compatibility
class GNonlinearSpringSet : public GDiscreteElementSet
{
public:
	enum { MP_F };

public:
	GNonlinearSpringSet(GModel* gm);

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
class GDeformableSpring : public GDiscreteObject
{
public:
	enum { MP_E, MP_DIV };

public:
	GDeformableSpring(GModel* gm);
	GDeformableSpring(GModel* gm, int n0, int n1);

	int NodeID(int i) const { return m_node[i]; }

	int Divisions() const { return GetIntValue(MP_DIV); }

protected:
	int		m_node[2];

public:
	int		m_ntag;
};

//-----------------------------------------------------------------------------
// This class is not really used, except to define the armature feature to
// the framework
class GArmature : public GDiscreteObject
{
public:
	GArmature(GModel* gm) : GDiscreteObject(gm, FE_ARMATURE) {}
};
