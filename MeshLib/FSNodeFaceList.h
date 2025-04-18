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
#include <vector>
#include <assert.h>
#include <FSCore/NestedArray.h>

class FSFace;
class FSMeshBase;

struct NodeFaceRef {
	int		fid;	// face index (into mesh' Face array)
	int		nid;	// local node index
	FSFace*	pf;		// face pointer
};

class FSNodeFaceList
{
public:
	FSNodeFaceList();
	~FSNodeFaceList(void);

	void Build(FSMeshBase* pm);
	bool BuildSorted(FSMeshBase* pm);

	void Clear();

	bool IsEmpty() const;

	int Valence(int i) const { return (int) m_face.items(i); }
	FSFace* Face(int n, int i) { return m_face.item(n, i).pf; }
	const FSFace* Face(int n, int i) const { return m_face.item(n, i).pf; }
	int FaceIndex(int n, int i) const { return m_face.item(n, i).fid; }
	int FaceNodeIndex(int n, int i) const { return m_face.item(n, i).nid; }

	bool HasFace(int n, FSFace* pf) const;

	int FindFace(const FSFace& f);

	int FindFace(int inode, int n[10], int m);

protected:
	bool Sort(int node);

protected:
	FSMeshBase*	m_pm;

	NestedArray<NodeFaceRef> m_face;
};
