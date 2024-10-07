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
	SELECT_FE_ELEMS,
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
	FESelection(SelectionType ntype);
	virtual ~FESelection();

	int Type() const { return m_ntype; }

	bool IsMovable() const { return m_movable; }
	void SetMovable(bool b) { m_movable = b; }

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

	virtual FEItemListBuilder* CreateItemList() = 0;

	virtual string GetName() { return "current selection"; }

protected:
	virtual void Update() = 0;
	virtual int Count() const = 0;

protected:
	BOX		m_box;
	int		m_nsize;
	int		m_ntype;
	bool	m_movable;
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
	GObjectSelection(GModel* ps);
	int Count() const override;
	virtual void Invert() override;
	virtual void Update() override;
	virtual void Translate(vec3d dr) override;
	virtual void Rotate(quatd q, vec3d c) override;
	virtual void Scale(double s, vec3d dr, vec3d c) override;

	virtual quatd GetOrientation() override;
	virtual vec3d GetPivot();
	virtual vec3d GetScale() override;

	FEItemListBuilder* CreateItemList() override;

	GObject* Object(int i);

	string GetName() override;

protected:
	GModel*		m_mdl;
	vector<int>	m_item;
};

// base class for geometry selections
class GSelection : public FESelection
{
public:
	GSelection(GModel* mdl, SelectionType type) : FESelection(type), m_mdl(mdl) {}

	GModel* GetGModel() { return m_mdl; }
	const GModel* GetGModel() const { return m_mdl; }

private:
	GModel* m_mdl;
};

class GPartSelection : public GSelection
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
		GPartSelection*		m_ps;
		GPart*		m_pg;
		int			m_npart;
	};

public:
	GPartSelection(GModel* ps);
	int Count() const override;
	void Invert();
	void Update();
	void Translate(vec3d dr) {}
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation ();

	GPart* Part(size_t n) { return m_partList[n]; }


	FEItemListBuilder* CreateItemList();

	void UpdateBoundingBox();

protected:
	std::vector<GPart*> m_partList;
};

class GFaceSelection : public GSelection
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
		GFaceSelection* m_sel;
		GFace*	m_pf;
		int		m_nsurf;
	};

	FEItemListBuilder* CreateItemList();

public:
	GFaceSelection(GModel* ps);
	int Count() const override;
	void Invert();
	void Update();
	void Translate(vec3d dr);
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation () { return quatd(0,0,0); }

	GFace* Face(size_t n) { return m_faceList[n]; }

	void UpdateBoundingBox();

protected:
	std::vector<GFace*> m_faceList;
};

class GEdgeSelection : public GSelection
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
		GEdgeSelection*	m_ps;
		GEdge*		m_pe;
		int			m_nedge;
	};

public:
	GEdgeSelection(GModel* ps);
	int Count() const override;
	void Invert();
	void Update();
	void Translate(vec3d dr) {}
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation () { return quatd(0,0,0); }

	FEItemListBuilder* CreateItemList() override;

	GEdge* Edge(size_t n) { return m_edgeList[n]; }

	void UpdateBoundingBox();

protected:
	std::vector<GEdge*> m_edgeList;
};

class GNodeSelection : public GSelection
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
		GNodeSelection* m_sel;
		GNode* m_pn;
		size_t	m_index;
	};

public:
	GNodeSelection(GModel* ps);
	int Count() const override;
	void Invert();
	void Update();
	void Translate(vec3d dr);
	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}
	quatd GetOrientation () { return quatd(0,0,0); }

	FEItemListBuilder* CreateItemList();

	void UpdateBoundingBox();

	GNode* Node(size_t n) { return m_nodeList[n]; }

protected:
	std::vector<GNode*> m_nodeList;
};

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

public:
	GDiscreteSelection(GModel* ps);
	int Count() const override;
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

// base class for selections of mesh items.
class FEMeshSelection : public FESelection
{
public:
	FEMeshSelection(SelectionType selectionType) : FESelection(selectionType) {}
};

class FEElementSelection : public FEMeshSelection
{
public:
	class Iterator
	{
	public:
		Iterator(FEElementSelection* pm);

		operator FEElement_*() { return m_pelem; }
		FEElement_* operator -> () { return m_pelem; }

		void operator ++ ();

	protected:
		FEElementSelection* m_sel;
		FEElement_*	m_pelem;
		size_t	m_n;
	};

public:
	FEElementSelection(FSMesh* pm);
	int Count() const override;
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSMesh* GetMesh() { return m_pMesh; }

	FEItemListBuilder* CreateItemList();

	FEElement_* Element(size_t i);
	int ElementIndex(size_t i) const;

	const std::vector<int>& ItemList() const { return m_item; }

protected:
	FSMesh*				m_pMesh;
	std::vector<int>	m_item;
};

class FEFaceSelection : public FEMeshSelection
{
public:
	class Iterator
	{
	public:
		Iterator(FEFaceSelection* pm);

		operator FSFace*() { return m_pface; }
		FSFace* operator -> () { return m_pface; }

		void operator ++ (); 

	protected:
		FEFaceSelection*	m_psel;
		FSFace*		m_pface;
		int			m_n;
	};

public:
	FEFaceSelection(FSMeshBase* pm);

	Iterator begin();

	FSMeshBase* GetMesh() { return m_pMesh; }

	FSFace* Face(size_t n);
	int FaceIndex(size_t n) const { return m_item[n]; }

public:
	int Count() const override;
	void Invert() override;
	void Update() override;
	void Translate(vec3d dr) override;
	void Rotate(quatd q, vec3d c) override;
	void Scale(double s, vec3d dr, vec3d c) override;
	quatd GetOrientation() override;
	FEItemListBuilder* CreateItemList() override;

	const std::vector<int>& ItemList() const { return m_item; }

protected:
	FSMeshBase*		m_pMesh;
	std::vector<int>	m_item;
};

class FEEdgeSelection : public FEMeshSelection
{
public:
	class Iterator
	{
	public:
		Iterator(FEEdgeSelection* sel);

		operator FSEdge*() { return m_pedge; }
		FSEdge* operator -> () { return m_pedge; }

		void operator ++ (); 

	protected:
		FEEdgeSelection*	m_sel;
		FSEdge*		m_pedge;
		int			m_n;
	};

public:
	FEEdgeSelection(FSLineMesh* pm);
	int Count() const override;
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSLineMesh* GetMesh() { return m_pMesh; }

	FSEdge* Edge(size_t n) { return m_pMesh->EdgePtr(m_items[n]); }
	int EdgeIndex(size_t n) const { return m_items[n]; }

	FEItemListBuilder* CreateItemList();

protected:
	FSLineMesh*		m_pMesh;
	std::vector<int>	m_items;
};

class FENodeSelection : public FEMeshSelection
{
public:
	class Iterator
	{
	public:
		Iterator(FENodeSelection* pm);

		operator FSNode*() { return m_pnode; }
		FSNode* operator -> () { return m_pnode; }

		void operator ++ (); 

	protected:
		FENodeSelection* m_psel;
		FSNode*		m_pnode;
		int			m_n;
	};

public:
	FENodeSelection(FSLineMesh* pm);
	int Count() const override;
	virtual void Invert();
	virtual void Update();
	virtual void Translate(vec3d dr);
	virtual void Rotate(quatd q, vec3d c);
	virtual void Scale(double s, vec3d dr, vec3d c);
	virtual quatd GetOrientation();

	FSLineMesh* GetMesh() { return m_pMesh; }
	const FSLineMesh* GetMesh() const { return m_pMesh; }

	FEItemListBuilder* CreateItemList();

	FENodeSelection::Iterator First();

	FSNode* Node(size_t n);
	int NodeIndex(size_t n) const { return m_items[n]; }

	const std::vector<int>& Items() const { return m_items; }

protected:
	FSLineMesh*	m_pMesh;
	std::vector<int>	m_items;
};
