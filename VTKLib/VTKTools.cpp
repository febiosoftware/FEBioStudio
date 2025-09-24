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

namespace vtk {
class Octree
{
public:
	struct Box {
		Box() 
		{
			x0 = x1 = 0.0;
			y0 = y1 = 0.0;
			z0 = z1 = 0.0;
		}

		Box(double X0, double Y0, double Z0, double X1, double Y1, double Z1)
		{
			x0 = X0;
			y0 = Y0;
			z0 = Z0;
			x1 = X1;
			y1 = Y1;
			z1 = Z1;
		}

		void add(const vec3d& r)
		{
			if (r.x < x0) x0 = r.x;
			if (r.x > x1) x1 = r.x;
			if (r.y < y0) y0 = r.y;
			if (r.y > y1) y1 = r.y;
			if (r.z < z0) z0 = r.z;
			if (r.z > z1) z1 = r.z;
		}

		bool IsInside(const vec3d& r) const
		{
			return ((r.x >= x0) && (r.y >= y0) && (r.z >= z0) &&
					(r.x <= x1) && (r.y <= y1) && (r.z <= z1));
		}

		double x0, y0, z0;
		double x1, y1, z1;
	};

public:
	Octree(Octree::Box box, int levels) : m_box(box), m_level(levels)
	{
		if (levels == 0)
		{
			for (int i = 0; i < 8; ++i) m_child[i] = nullptr;
			return;
		}

		double x0 = box.x0, x1 = box.x1;
		double y0 = box.y0, y1 = box.y1;
		double z0 = box.z0, z1 = box.z1;
		int n = 0;
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
				{
					double xa = x0 + i * (x1 - x0) * 0.5;
					double ya = y0 + j * (y1 - y0) * 0.5;
					double za = z0 + k * (z1 - z0) * 0.5;
					double xb = x0 + (i + 1.0) * (x1 - x0) * 0.5;
					double yb = y0 + (j + 1.0) * (y1 - y0) * 0.5;
					double zb = z0 + (k + 1.0) * (z1 - z0) * 0.5;
					Box boxi(xa, ya, za, xb, yb, zb);
					m_child[n++] = new Octree(boxi, levels - 1);
				}
	}
	~Octree() { for (int i = 0; i < 8; ++i) delete m_child[i]; }

	int addNode(std::vector<vec3d>& points, const vec3d& r)
	{
		const double eps = 1e-12;
		if (m_level == 0)
		{
			if (m_box.IsInside(r))
			{
				for (int i = 0; i < m_nodes.size(); ++i)
				{
					vec3d& ri = points[m_nodes[i]];
					if ((ri - r).SqrLength() <= eps)
					{
						// node is already in list
						return m_nodes[i];
					}
				}

				// if we get here, the node is in this box, 
				// but not in the points array yet, so add it
				points.push_back(r);
				m_nodes.push_back((int)points.size() - 1);
				return (int)points.size() - 1;
			}
			else return -1;
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				int n = m_child[i]->addNode(points, r);
				if (n >= 0) return n;
			}
			return -1;
		}
	}

private:
	int			m_level;
	Box			m_box;
	Octree*		m_child[8];
	std::vector<int>	m_nodes;
};
} // namespace vtk

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

		vtk::Octree::Box box;
		for (int i = 0; i < nodes; ++i)
		{
			vec3d& r = points[i];
			if (i == 0)
			{
				box.x0 = box.x1 = r.x;
				box.y0 = box.y1 = r.y;
				box.z0 = box.z1 = r.z;
			}
			else
			{
				box.add(r);
			}
		}
	
		const int MAX_LEVELS = 3;
		// L = log8(N / P); L = levels, N = nodes, P = target nr. of points per cell. 
		int levels = (int) (log10((double)nodes/4096.0)/log10(8.0));
		if (levels <= 1) levels = 1;
		if (levels >= MAX_LEVELS) levels = MAX_LEVELS;

		vtk::Octree o(box, levels);
		std::vector<vec3d> mergedPoints;
		for (int i = 0; i < nodes; ++i)
		{
			vec3d ri = points[i];
			int id = o.addNode(mergedPoints, ri);
			nodeMap[i] = id;
		}
		uniqueNodes = mergedPoints.size();
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
