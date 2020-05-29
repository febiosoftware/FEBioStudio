#pragma once

#include "FEItemListBuilder.h"
#include "FEGroup.h"
#include "FESelection.h"

class FEModel;
class GPartSelection;
class GFaceSelection;
class GEdgeSelection;
class GNodeSelection;

//-----------------------------------------------------------------------------
// The GGroup performs the same function for GObjects as the FEGroup is for
// meshes. Its main purpose is to convert groups into FEItemList.
//
class GGroup : public FEItemListBuilder
{
public:
	GGroup(FEModel* ps, int ntype);
	~GGroup(void);

	FENodeList*	BuildNodeList() { return 0; }
	FEFaceList*	BuildFaceList() { return 0; }
	FEElemList*	BuildElemList() { return 0; }

protected:
	FEModel*	m_ps;
};

//-----------------------------------------------------------------------------

class GNodeList : public GGroup
{
public:
	GNodeList(FEModel* ps) : GGroup(ps, GO_NODE){}
	GNodeList(FEModel* ps, GNodeSelection* pn);

	vector<GNode*>	GetNodeList();

	FEItemListBuilder* Copy() override;

	FENodeList* BuildNodeList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GFaceList : public GGroup
{
public:
	GFaceList(FEModel* ps) : GGroup(ps, GO_FACE){}
	GFaceList(FEModel* ps, GFaceSelection* pf);

	vector<GFace*>	GetFaceList();

	FEItemListBuilder* Copy() override;

	FENodeList* BuildNodeList() override;
	FEFaceList*	BuildFaceList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GEdgeList : public GGroup
{
public:
	GEdgeList(FEModel* ps) : GGroup(ps, GO_EDGE){}
	GEdgeList(FEModel* ps, GEdgeSelection* pe);

	vector<GEdge*>	GetEdgeList();

	GEdge* GetEdge(int n);

	FEItemListBuilder* Copy() override;

	FENodeList* BuildNodeList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GPartList : public GGroup
{
public:
	GPartList(FEModel* ps) : GGroup(ps, GO_PART){}
	GPartList(FEModel* ps, GPartSelection* pg);

	void Create(GObject* po);

	vector<GPart*>	GetPartList();

	FEItemListBuilder* Copy() override;

	FENodeList* BuildNodeList() override;
	FEElemList* BuildElemList() override;

	bool IsValid() const override;

	static GPartList* CreateNew();
	static void SetModel(FEModel* mdl);
	static FEModel* m_model;
};
