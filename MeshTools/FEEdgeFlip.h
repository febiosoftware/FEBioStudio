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
#include "FESurfaceModifier.h"
#include <MeshLib/FSFaceEdgeList.h>

class FEEdgeFlip : public FESurfaceModifier
{
public:
	FEEdgeFlip();
	~FEEdgeFlip();

	FSSurfaceMesh* Apply(FSSurfaceMesh* pm) override;

private:
	void Cleanup();
	void MarkEdges(FSSurfaceMesh* mesh, bool selectedOnly);
	bool FlipEdge(int iedge, FSSurfaceMesh* mesh, bool forceFlip);
	bool ShouldFlip(int a[3], int b[3], FSSurfaceMesh* mesh);
	void DoFlipEdge(int iedge, int a[3], int b[3], int k0, int k1, FSSurfaceMesh* mesh);

private:
	EdgeList*			m_EL;
	FSFaceEdgeList*		m_FEL;
	FSEdgeFaceList*		m_EFL;
	std::vector<int>	m_tag;
};
