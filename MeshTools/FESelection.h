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
#include <GeomLib/FSGroup.h>
#include <GeomLib/GModel.h>
#include <FEMLib/GDiscreteObject.h>
#include <vector>
//using namespace std;

// selection type
enum SelectionType
{
	SELECT_OBJECTS,
	SELECT_PARTS,
	SELECT_SURFACES,
	SELECT_CURVES,
	SELECT_NODES,
	SELECT_DISCRETE_OBJECT,
	SELECT_FE_ELEMENTS,
	SELECT_FE_FACES,
	SELECT_FE_EDGES,
	SELECT_FE_NODES
};

//-----------------------------------------------------------------------------
// CLASS FESelection
// Base class for selections
//
class FESelection  
{
public:
	FESelection(int ntype);
	virtual ~FESelection();

	int Type() const { return m_ntype; }

	BOX GetBoundingBox() { return m_box; }
	vec3d GetPivot() { return m_box.Center(); };
	int Size();

	bool Supports(unsigned int itemFlag) const;

	virtual void Invert() = 0;

	virtual void Translate(vec3d dr) = 0;
	virtual void Rotate(quatd q, vec3d c) = 0;
	virtual void Scale(double s, vec3d dr, vec3d c) = 0;

	virtual quatd GetOrientation() = 0;
	virtual vec3d GetScale() { return vec3d(1,1,1); }

	virtual int Next() { return -1; }
	virtual int Prev() { return -1; }

	virtual FEItemListBuilder* CreateItemList() = 0;

	virtual string GetName() { return "current selection"; }

protected:
	virtual void Update() = 0;
	virtual int Count() = 0;

protected:
	BOX		m_box;
	int		m_nsize;
	int		m_ntype;
};

inline int FESelection::Size()
{
	if (m_nsize == -1) m_nsize = Count();
	return m_nsize;
}

//-----------------------------------------------------------------------------

class GObjectSelection : public FESelection
{
public:
	GObjectSelection(GModel* ps) : FESelection(SELECT_OBJECTS) { m_mdl = ps; Update(); }
	int Count() override;
	virtual void Invert() override;
	virtual void Update() override;
	virtual void Translate(vec3d dr) override;
	virtual void Rotate(quatd q, vec3d c) override;
	virtual void Scale(double s, vec3d dr, vec3d c) override;

	virtual quatd GetOrientation() override;
	virtual vec3d GetPivot();
	virtual vec3d GetScale() override;

	int Next() override;
	int Prev() override;

	FEItemListBuilder* CreateItemList() override;

	GObject* Object(int i);

	string GetName() override;

protected:
	GModel*		m_mdl;
	vector<int>	m_item;
};

//-----------------------------------------------------------------------------

class GPartSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(GPartSelection* ps);

		Iterator& operator ++ ();

		GPart* operator -> () { return m_pg; }
		GPart& operator * () { return *m_pg; }

		operator GPart* () { return m_pg; }

	protected:
		GModel*		m_ps;
		GPart*		m_pg;
		int			m_npart;
	};

	int Next();
	int Prev();

public:
	GPartSelection(GModel* ps) : FESelection(SELECT_PARTS) { m_mdl = ps; Update(); }
	int Count();
	void Invert();
	void Update();
	void Translate(vec3d dr) {}
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation ();

	GModel* GetGModel() { return m_mdl; }

	FEItemListBuilder* CreateItemList();

protected:
	GModel*	m_mdl;
};

//-----------------------------------------------------------------------------

class GFaceSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(GFaceSelection* ps);

		Iterator& operator ++ ();

		GFace* operator -> () { return m_pf; }
		GFace& operator * () { return *m_pf; }

		operator GFace* () { return m_pf; }

	protected:
		GModel*	m_ps;
		GFace*	m_pf;
		int			m_nsurf;
	};

	int Next();
	int Prev();

	FEItemListBuilder* CreateItemList();

public:
	GFaceSelection(GModel* ps) : FESelection(SELECT_SURFACES) { m_ps = ps; Update(); }
	int Count();
	void Invert();
	void Update();
	void Translate(vec3d dr) {}
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation () { return quatd(0,0,0); }

	GModel* GetGModel() { return m_ps; }

protected:
	GModel*	m_ps;
};

//-----------------------------------------------------------------------------

class GEdgeSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(GEdgeSelection* ps);

		Iterator& operator ++ ();

		GEdge* operator -> () { return m_pe; }
		GEdge& operator * () { return *m_pe; }

		operator GEdge* () { return m_pe; }

	protected:
		GModel*	m_ps;
		GEdge*	m_pe;
		int			m_nedge;
	};

	int Next();
	int Prev();

public:
	GEdgeSelection(GModel* ps) : FESelection(SELECT_CURVES) { m_ps = ps; Update(); }
	int Count();
	void Invert();
	void Update();
	void Translate(vec3d dr) {}
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation () { return quatd(0,0,0); }

	GModel* GetGModel() { return m_ps; }

	FEItemListBuilder* CreateItemList();

protected:
	GModel*	m_ps;
};

//-----------------------------------------------------------------------------

class GNodeSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(GNodeSelection* ps);

		Iterator& operator ++ ();

		GNode* operator -> () { return m_pn; }
		GNode& operator * () { return *m_pn; }

		operator GNode* () { return m_pn; }

	protected:
		GNode*	m_pn;
		std::vector<GNode*> m_sel;
		int		m_node;
	};

	int Next();
	int Prev();

public:
	GNodeSelection(GModel* ps) : FESelection(SELECT_NODES) { m_ps = ps; Update(); }
	int Count();
	void Invert();
	void Update();
	void Translate(vec3d dr);
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation () { return quatd(0,0,0); }

	GModel* GetGModel() { return m_ps; }

	FEItemListBuilder* CreateItemList();

protected:
	GModel*	m_ps;
};

//-----------------------------------------------------------------------------

class GDiscreteSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(GDiscreteSelection* ps);

		Iterator& operator ++ ();

		GDiscreteObject* operator -> () { return m_pn; }
		GDiscreteObject& operator * () { return *m_pn; }

		operator GDiscreteObject* () { return m_pn; }

	protected:
		GModel*				m_ps;
		GDiscreteObject*	m_pn;
		int					m_item;
		int					m_comp;
	};

	int Next();
	int Prev();

public:
	GDiscreteSelection(GModel* ps);
	int Count();
	void Invert();
	void Update();

	void Translate(vec3d dr) {}
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation() { return quatd(0, 0, 0); }

	GModel* GetGModel() { return m_ps; }

	FEItemListBuilder* CreateItemList() { return 0; }

protected:
	GModel*	m_ps;
	int		m_count;
};

//-----------------------------------------------------------------------------

class FEElementSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(FSMesh* pm);

		operator FEElement_*() { return m_pelem; }
		FEElement_* operator -> () { return m_pelem; }

		void operator ++ ();

	protected:
		FSMesh*		m_pm;
		FEElement_*	m_pelem;
		int			m_n;
	};

public:
	FEElementSelection(FSMesh* pm);
	int Count();
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSMesh* GetMesh() { return m_pMesh; }

	FEItemListBuilder* CreateItemList();

	FEElement_* Element(int i);
    int ElementID(int i);

protected:
	FSMesh*		m_pMesh;
	vector<int>	m_item;
};

//-----------------------------------------------------------------------------

class FEFaceSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(FSMeshBase* pm);

		operator FSFace*() { return m_pface; }
		FSFace* operator -> () { return m_pface; }

		void operator ++ (); 

	protected:
		FSMeshBase*	m_pm;
		FSFace*		m_pface;
		int			m_n;
	};

public:
	FEFaceSelection(FSMeshBase* pm);
	int Count();
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSMeshBase* GetMesh() { return m_pMesh; }

	FEItemListBuilder* CreateItemList();

	Iterator begin();

protected:
	FSMeshBase*		m_pMesh;
};

//-----------------------------------------------------------------------------

class FEEdgeSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(FSLineMesh* pm);

		operator FSEdge*() { return m_pedge; }
		FSEdge* operator -> () { return m_pedge; }

		void operator ++ (); 

	protected:
		FSLineMesh*	m_pm;
		FSEdge*		m_pedge;
		int			m_n;
	};

public:
	FEEdgeSelection(FSLineMesh* pm);
	int Count();
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSLineMesh* GetMesh() { return m_pMesh; }

	FEItemListBuilder* CreateItemList();

protected:
	FSLineMesh*		m_pMesh;
};

//-----------------------------------------------------------------------------

class FENodeSelection : public FESelection
{
public:
	class Iterator
	{
	public:
		Iterator(FSLineMesh* pm);

		operator FSNode*() { return m_pnode; }
		FSNode* operator -> () { return m_pnode; }

		void operator ++ (); 

	protected:
		FSLineMesh*	m_pm;
		FSNode*		m_pnode;
		int			m_n;
	};

public:
	FENodeSelection(FSLineMesh* pm);
	int Count();
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSLineMesh* GetMesh() { return m_pMesh; }

	FEItemListBuilder* CreateItemList();

	FENodeSelection::Iterator First();

protected:
	FSLineMesh*	m_pMesh;
};
