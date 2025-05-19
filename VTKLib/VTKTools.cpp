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
#include "VTKTools.h"

FSMesh* VTKTools::BuildFEMesh(const VTK::vtkPiece& vtkMesh, bool splitPolys)
{
	FSMesh* pm = new FSMesh();
	std::vector<int> nodeMap;
	if (BuildFEMesh(vtkMesh, pm, nodeMap, splitPolys, false)) return pm;
	else
	{
		delete pm;
		return nullptr;
	}
}

bool VTKTools::BuildFEMesh(const VTK::vtkPiece& vtkMesh, FSMesh* pm, std::vector<int>& nodeMap, bool splitPolys, bool mapNodes)
{
	if (pm == nullptr) return false;

	// get the number of nodes and elements
	int nodes = (int)vtkMesh.Points();

	// get the point coordinates
	std::vector<vec3d> points(nodes);
	for (int i = 0; i < nodes; ++i)
	{
		VTK::vtkPoint pt = vtkMesh.Point(i);
		points[i] = vec3d(pt.x, pt.y, pt.z);
	}

	// count the number of unique nodes
	size_t uniqueNodes = 0;
	if (mapNodes)
	{
		nodeMap.assign(nodes, -1);
		for (int i = 0; i < nodes; ++i)
		{
			vec3d ri = points[i];
			int id = -1;
			for (int j = 0; j < i; ++j)
			{
				double norm2 = (ri - points[j]).norm2();
				if (norm2 < 1e-12)
				{
					id = nodeMap[j];
					assert(id != -1);
					break;
				}
			}

			if (id == -1)
			{
				nodeMap[i] = uniqueNodes++;
			}
			else nodeMap[i] = id;
		}
	}
	else
	{
		nodeMap.assign(nodes, -1);
		for (int i = 0; i < nodes; ++i) nodeMap[i] = i;
		uniqueNodes = nodes;
	}

	int elems = 0;
	for (int i = 0; i < vtkMesh.Cells(); i++)
	{
		VTK::vtkCell cell = vtkMesh.Cell(i);
		switch (cell.m_cellType)
		{
		case VTK::vtkCell::VTK_LINE      : elems += 1; break;
		case VTK::vtkCell::VTK_TRIANGLE  : elems += 1; break;
		case VTK::vtkCell::VTK_QUAD      : elems += 1; break;
		case VTK::vtkCell::VTK_TETRA     : elems += 1; break;
		case VTK::vtkCell::VTK_HEXAHEDRON: elems += 1; break;
		case VTK::vtkCell::VTK_WEDGE     : elems += 1; break;
		case VTK::vtkCell::VTK_PYRAMID   : elems += 1; break;
		case VTK::vtkCell::VTK_POLYGON:
		{
			switch (cell.m_numNodes)
			{
			case 0:
			case 1:
			case 2:
				return false;
				break;
			case 3:
			case 4:
				elems += 1;
				break;
			default:
				if (splitPolys)
					elems += cell.m_numNodes - 2;
				else
					return false;
			}
		}
		break;
		case VTK::vtkCell::VTK_LAGRANGE_HEXAHEDRON:
			elems += 1;
			break;
		default:
			return false;
		}
	}

	// create a new mesh
	pm->Create(uniqueNodes, elems);

	// copy nodal data
	uniqueNodes = 0;
	for (int i = 0; i < nodes; ++i)
	{
		if (nodeMap[i] >= uniqueNodes)
		{
			FSNode& node = pm->Node(uniqueNodes++);
			node.r = points[i];
		}
	}
	assert(uniqueNodes == pm->Nodes());

	// copy element data
	elems = 0;
	for (int i = 0; i < vtkMesh.Cells(); ++i)
	{
		VTK::vtkCell cell = vtkMesh.Cell(i);

		if (cell.m_cellType == VTK::vtkCell::VTK_POLYGON)
		{
			if (cell.m_numNodes == 3)
			{
				FSElement& el = pm->Element(elems++);
				el.m_gid = cell.m_label; assert(el.m_gid >= 0);
				if (el.m_gid < 0) el.m_gid = 0;
				el.SetType(FE_TRI3);
				for (int j = 0; j < 3; ++j) el.m_node[j] = nodeMap[cell.m_node[j]];
			}
			else if (cell.m_numNodes == 4)
			{
				FSElement& el = pm->Element(elems++);
				el.m_gid = cell.m_label; assert(el.m_gid >= 0);
				if (el.m_gid < 0) el.m_gid = 0;
				el.SetType(FE_QUAD4);
				for (int j = 0; j < 4; ++j) el.m_node[j] = nodeMap[cell.m_node[j]];
			}
			else
			{
				// Simple triangulation algorithm. Assumes polygon is convex.
				int* n = cell.m_node;
				for (int j = 0; j < cell.m_numNodes - 2; ++j)
				{
					FSElement& el = pm->Element(elems++);
					el.SetType(FE_TRI3);
					el.m_gid = cell.m_label; assert(el.m_gid >= 0);
					if (el.m_gid < 0) el.m_gid = 0;
					el.m_node[0] = nodeMap[n[0]];
					el.m_node[1] = nodeMap[n[j + 1]];
					el.m_node[2] = nodeMap[n[j + 2]];
				}
			}
		}
		else if (cell.m_cellType == VTK::vtkCell::VTK_LAGRANGE_HEXAHEDRON)
		{
			if (cell.m_numNodes == 8)
			{
				FSElement& el = pm->Element(elems++);
				el.m_gid = cell.m_label; assert(el.m_gid >= 0);
				if (el.m_gid < 0) el.m_gid = 0;
				el.SetType(FE_HEX8);
				for (int j = 0; j < 8; ++j) el.m_node[j] = nodeMap[cell.m_node[j]];
			}
			else return false;
		}
		else
		{
			FSElement& el = pm->Element(elems++);
			el.m_gid = cell.m_label; assert(el.m_gid >= 0);
			if (el.m_gid < 0) el.m_gid = 0;

			switch (cell.m_cellType)
			{
			case VTK::vtkCell::VTK_LINE      : el.SetType(FE_BEAM2 ); break;
			case VTK::vtkCell::VTK_TRIANGLE  : el.SetType(FE_TRI3  ); break;
			case VTK::vtkCell::VTK_QUAD      : el.SetType(FE_QUAD4 ); break;
			case VTK::vtkCell::VTK_TETRA     : el.SetType(FE_TET4  ); break;
			case VTK::vtkCell::VTK_HEXAHEDRON: el.SetType(FE_HEX8  ); break;
			case VTK::vtkCell::VTK_WEDGE     : el.SetType(FE_PENTA6); break;
			case VTK::vtkCell::VTK_PYRAMID   : el.SetType(FE_PYRA5 ); break;
			default:
				delete pm;
				return false;
			}

			int nn = el.Nodes();
			assert(nn == cell.m_numNodes);
			for (int j = 0; j < nn; ++j) el.m_node[j] = nodeMap[cell.m_node[j]];
		}
	}

	pm->RebuildMesh();

	// invert the node map
	std::vector<int> tmp(nodeMap);
	nodeMap.assign(uniqueNodes, -1);
	uniqueNodes = 0;
	for (int i = 0; i < nodes; ++i)
	{
		if (tmp[i] >= uniqueNodes)
		{
			nodeMap[uniqueNodes++] = i;
		}
	}

	return true;
}
