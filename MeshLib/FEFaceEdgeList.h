#pragma once
#include <vector>
#include <set>
#include "FEFace.h"

class FEMeshBase;
class FEMesh;
class FESurfaceMesh;

class FENodeNodeTable
{
public:
	FENodeNodeTable(const FESurfaceMesh& mesh);
	FENodeNodeTable(const FEMesh& mesh, bool surfOnly = false);

	std::set<int>& operator [] (int i) { return NNT[i]; }

private:
	std::vector< std::set<int> > NNT;
};

class FEEdgeList
{
public:
	FEEdgeList(const FESurfaceMesh& mesh);
	FEEdgeList(const FEMesh& mesh, bool surfOnly = false);

	int size() const { return (int) ET.size(); }

	std::pair<int,int>& operator [] (int i) { return ET[i]; }
	const std::pair<int, int>& operator [] (int i) const { return ET[i]; }

private:
	std::vector< std::pair<int, int> > ET;
};

class FEFaceTable
{
public:
	FEFaceTable(const FEMesh& mesh);

	int size() const { return (int) FT.size(); }

	FEFace& operator [] (int i) { return FT[i]; }

	const FEFace& operator [] (int i) const { return FT[i]; }

private:
	std::vector<FEFace>	FT;
};

class FEFaceEdgeList
{
public:
	FEFaceEdgeList(const FEMeshBase& mesh, const FEEdgeList& ET);

	std::vector<int>& operator [] (int i) { return FET[i]; }

private:
	std::vector< std::vector<int> > FET;
};

class FEElementEdgeList
{
public:
	FEElementEdgeList(const FEMesh& mesh, const FEEdgeList& ET);

	std::vector<int>& operator [] (int i) { return EET[i]; }

private:
	std::vector< std::vector<int> > EET;
};

class FEElementFaceList
{
public:
	FEElementFaceList(const FEMesh& mesh, const FEFaceTable& FT);

	std::vector<int>& operator [] (int i) { return EFT[i]; }

private:
	std::vector< std::vector<int> > EFT;
};

class FEFaceFaceList
{
public:
	FEFaceFaceList(const FEMesh& mesh, const FEFaceTable& FT);

	int operator [] (int i) { return FFT[i]; }

private:
	std::vector<int> FFT;
};

class FEEdgeIndexList
{
public:
	FEEdgeIndexList(const FEMesh& mesh, const FEEdgeList& ET);

	int operator [] (int i) { return EET[i]; }

private:
	std::vector<int> EET;
};

class FEEdgeEdgeList
{
public:
	FEEdgeEdgeList(const FEMesh& mesh, int edgeId = -1);

	int size() { return (int)EEL.size(); }

	std::vector<int>& operator [] (int i) { return EEL[i]; }

private:
	std::vector< std::vector<int> > EEL;
};

class FEEdgeFaceList
{
public:
	FEEdgeFaceList(const FEMesh& mesh);
	FEEdgeFaceList(const FESurfaceMesh& mesh);

	int size() { return (int) EFL.size(); }

	std::vector<int>& operator [] (int i) { return EFL[i]; }

private:
	std::vector< std::vector<int> > EFL;
};
