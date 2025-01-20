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

#include "stdafx.h"
#include "FENikeExport.h"
#include "FEPostModel.h"
#include <stdio.h>

using namespace Post;

FENikeExport::FENikeExport(void)
{
}

FENikeExport::~FENikeExport(void)
{
}

bool FENikeExport::Save(FEPostModel &fem, const char *szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	int i;

	FEState& s = *fem.CurrentState();
	FSMesh* pm = s.GetFEMesh();

	// export nodes
	for (i=0; i<pm->Nodes(); ++i)
	{
		const vec3f& r = s.m_NODE[i].m_rt;
		fprintf(fp, "%8d%5d%20lg%20lg%20lg%5d\n", i+1, 0, r.x, r.y, r.z, 0);
	}

	// export elements
	for (i=0; i<pm->Elements(); ++i)
	{
		FSElement_& e = pm->ElementRef(i);
		int* n = e.m_node;
		fprintf(fp,"%8d%5d%8d%8d%8d%8d%8d%8d%8d%8d\n", i+1, 1,n[0]+1,n[1]+1,n[2]+1,n[3]+1,n[4]+1,n[5]+1,n[6]+1,n[7]+1);
	}

	fclose(fp);

	return true;
}
