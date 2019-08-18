#pragma once

#include <vector>
using namespace std;

class FEMesh;
class FESurfaceMesh;

//-----------------------------------------------------------------------------
// This class stores for each node a list of nodes that are connected to it.
class FENodeNodeList
{
public:
	FENodeNodeList(FEMesh* pm);
	FENodeNodeList(FESurfaceMesh* pm);
	~FENodeNodeList();

	int Valence(int n) { return m_val[n]; }
	int Node(int n, int j) { return m_node[ m_off[n] + j]; }

	// call this before storing data on node-node connection
	void InitValues(double v = 0.0);

	// get the edge value
	double& Value(int n, int j) { return m_data[m_off[n] + j]; }

protected:
	void Build(FEMesh* pm);
	void Build(FESurfaceMesh* pm);

protected:
	vector<int>	m_val;		// Valence list
	vector<int>	m_off;		// Offset into node array
	vector<int>	m_node;		// node list

	vector<double>	m_data;
};
