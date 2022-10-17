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

#include "FEItemListBuilder.h"
#include "FEGroup.h"
#include "FESelection.h"

class FSModel;
class GPartSelection;
class GFaceSelection;
class GEdgeSelection;
class GNodeSelection;

//-----------------------------------------------------------------------------
// The GGroup performs the same function for GObjects as the FSGroup is for
// meshes. Its main purpose is to convert groups into FEItemList.
//
class GGroup : public FEItemListBuilder
{
public:
	GGroup(FSModel* ps, int ntype, unsigned int flags);
	~GGroup(void);

	FSNodeList*	BuildNodeList() { return 0; }
	FEFaceList*	BuildFaceList() { return 0; }
	FEElemList*	BuildElemList() { return 0; }

protected:
	FSModel*	m_ps;
};

//-----------------------------------------------------------------------------

class GNodeList : public GGroup
{
public:
	GNodeList(FSModel* ps) : GGroup(ps, GO_NODE, FE_NODE_FLAG){}
	GNodeList(FSModel* ps, GNodeSelection* pn);

	vector<GNode*>	GetNodeList();

	FEItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GFaceList : public GGroup
{
public:
	GFaceList(FSModel* ps) : GGroup(ps, GO_FACE, FE_NODE_FLAG | FE_FACE_FLAG){}
	GFaceList(FSModel* ps, GFaceSelection* pf);
	GFaceList(FSModel* ps, GFace* pf);

	vector<GFace*>	GetFaceList();

	FEItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;
	FEFaceList*	BuildFaceList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GEdgeList : public GGroup
{
public:
	GEdgeList(FSModel* ps) : GGroup(ps, GO_EDGE, FE_NODE_FLAG){}
	GEdgeList(FSModel* ps, GEdgeSelection* pe);

	vector<GEdge*>	GetEdgeList();

	GEdge* GetEdge(int n);

	FEItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GPartList : public GGroup
{
public:
	GPartList(FSModel* ps) : GGroup(ps, GO_PART, FE_NODE_FLAG | FE_FACE_FLAG | FE_ELEM_FLAG){}
	GPartList(FSModel* ps, GPartSelection* pg);

	void Create(GObject* po);

	vector<GPart*>	GetPartList();

	FEItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;
	FEElemList* BuildElemList() override;
	FEFaceList*	BuildFaceList() override;

	bool IsValid() const override;

	static GPartList* CreateNew();
	static void SetModel(FSModel* mdl);
	static FSModel* m_model;
};
