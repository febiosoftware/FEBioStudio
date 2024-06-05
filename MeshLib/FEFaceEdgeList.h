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
#include <set>
#include "FEFace.h"

class FSMeshBase;
class FSMesh;
class FSSurfaceMesh;
class FSLineMesh;

class FSNodeNodeTable
{
public:
	FSNodeNodeTable(const FSSurfaceMesh& mesh);
	FSNodeNodeTable(const FSMesh& mesh, bool surfOnly = false);

	std::set<int>& operator [] (int i) { return NNT[i]; }

private:
	std::vector< std::set<int> > NNT;
};

class FSEdgeList
{
public:
	FSEdgeList();
	FSEdgeList(const FSSurfaceMesh& mesh);
	FSEdgeList(const FSMesh& mesh, bool surfOnly = false);

	void BuildFromMeshEdges(FSLineMesh& mesh);

	int size() const { return (int) ET.size(); }

	std::pair<int,int>& operator [] (int i) { return ET[i]; }
	const std::pair<int, int>& operator [] (int i) const { return ET[i]; }

private:
	std::vector< std::pair<int, int> > ET;
};

class FSFaceTable
{
public:
	FSFaceTable(const FSMesh& mesh);

	int size() const { return (int) FT.size(); }

	FSFace& operator [] (int i) { return FT[i]; }

	const FSFace& operator [] (int i) const { return FT[i]; }

private:
	std::vector<FSFace>	FT;
};

class FSFaceEdgeList
{
public:
	FSFaceEdgeList(const FSMeshBase& mesh, const FSEdgeList& ET);

	std::vector<int>& operator [] (int i) { return FET[i]; }

private:
	std::vector< std::vector<int> > FET;
};

class FSElementEdgeList
{
public:
	FSElementEdgeList(const FSMesh& mesh, const FSEdgeList& ET);

	std::vector<int>& operator [] (int i) { return EET[i]; }

	size_t Valence(int nel) const { return EET[nel].size(); }

	int EdgeIndex(int nel, int nedge) const { return EET[nel][nedge]; }

private:
	std::vector< std::vector<int> > EET;
};

class FSElementFaceList
{
public:
	FSElementFaceList(const FSMesh& mesh, const FSFaceTable& FT);

	std::vector<int>& operator [] (int i) { return EFT[i]; }

	size_t Valence(int iel) const { return EFT[iel].size(); }
	int FaceIndex(int iel, int nface) const { return EFT[iel][nface]; }

private:
	std::vector< std::vector<int> > EFT;
};

class FSFaceFaceList
{
public:
	FSFaceFaceList(const FSMesh& mesh, const FSFaceTable& FT);

	int operator [] (int i) { return FFT[i]; }

private:
	std::vector<int> FFT;
};

class FSEdgeIndexList
{
public:
	FSEdgeIndexList(const FSMesh& mesh, const FSEdgeList& ET);

	int operator [] (int i) { return EET[i]; }

private:
	std::vector<int> EET;
};

class FSEdgeEdgeList
{
public:
	FSEdgeEdgeList(const FSMesh& mesh, int edgeId = -1);

	int size() { return (int)EEL.size(); }

	std::vector<int>& operator [] (int i) { return EEL[i]; }

private:
	std::vector< std::vector<int> > EEL;
};

class FSEdgeFaceList
{
public:
	FSEdgeFaceList(const FSMesh& mesh);
	FSEdgeFaceList(const FSSurfaceMesh& mesh);

	int size() { return (int) EFL.size(); }

	std::vector<int>& operator [] (int i) { return EFL[i]; }

private:
	std::vector< std::vector<int> > EFL;
};
