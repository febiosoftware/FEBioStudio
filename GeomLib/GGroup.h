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

#include <MeshLib/FSItemListBuilder.h>
#include "FSGroup.h"
#include <MeshTools/FESelection.h>

class GModel;
class GPartSelection;
class GFaceSelection;
class GEdgeSelection;
class GNodeSelection;

//-----------------------------------------------------------------------------
// The GGroup performs the same function for GObjects as the FSGroup is for
// meshes. Its main purpose is to convert groups into FEItemList.
//
class GGroup : public FSItemListBuilder
{
public:
	GGroup(GModel* ps, int ntype, unsigned int flags);
	~GGroup(void);

	FSNodeList*	BuildNodeList() override { return 0; }
	FSEdgeList*	BuildEdgeList() override { return 0; }
	FSFaceList*	BuildFaceList() override { return 0; }
	FSElemList*	BuildElemList() override { return 0; }

protected:
	GModel*	m_ps;
};

//-----------------------------------------------------------------------------

class GNodeList : public GGroup
{
public:
	GNodeList(GModel* ps) : GGroup(ps, GO_NODE, FE_NODE_FLAG){}
	GNodeList(GModel* ps, GNodeSelection* pn);

	std::vector<GNode*>	GetNodeList();

	FSItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GFaceList : public GGroup
{
public:
	GFaceList(GModel* ps) : GGroup(ps, GO_FACE, FE_NODE_FLAG | FE_FACE_FLAG){}
	GFaceList(GModel* ps, GFaceSelection* pf);
	GFaceList(GModel* ps, GFace* pf);

	std::vector<GFace*>	GetFaceList();

	FSItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;
	FSFaceList*	BuildFaceList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GEdgeList : public GGroup
{
public:
	GEdgeList(GModel* ps) : GGroup(ps, GO_EDGE, FE_NODE_FLAG){}
	GEdgeList(GModel* ps, GEdgeSelection* pe);

	std::vector<GEdge*>	GetEdgeList();

	GEdge* GetEdge(int n);

	FSItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;

	FSEdgeList* BuildEdgeList() override;

	bool IsValid() const override;
};

//-----------------------------------------------------------------------------

class GPartList : public GGroup
{
public:
	GPartList(GModel* ps);
	GPartList(GModel* ps, GPartSelection* pg);

	void Create(GObject* po);

	std::vector<GPart*>	GetPartList();

	FSItemListBuilder* Copy() override;

	FSNodeList* BuildNodeList() override;
	FSElemList* BuildElemList() override;
	FSFaceList*	BuildFaceList() override;

	FSPartSet* BuildPartSet();

	bool IsValid() const override;
};
