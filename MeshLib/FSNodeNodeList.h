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

class FSMesh;
class FSSurfaceMesh;

//-----------------------------------------------------------------------------
// This class stores for each node a list of nodes that are connected to it.
class FSNodeNodeList
{
public:
	FSNodeNodeList(FSMesh* pm, bool preservePartitions = false);
	FSNodeNodeList(FSSurfaceMesh* pm);
	~FSNodeNodeList();

	int Valence(int n) { return m_val[n]; }
	int Node(int n, int j) { return m_node[ m_off[n] + j]; }

	// call this before storing data on node-node connection
	void InitValues(double v = 0.0);

	// get the edge value
	double& Value(int n, int j) { return m_data[m_off[n] + j]; }

protected:
	void Build(FSMesh* pm, bool preservePartitions = false);
	void Build(FSSurfaceMesh* pm);

protected:
	std::vector<int>	m_val;		// Valence list
	std::vector<int>	m_off;		// Offset into node array
	std::vector<int>	m_node;		// node list

	std::vector<double>	m_data;
};
