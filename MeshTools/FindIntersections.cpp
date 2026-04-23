/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "FindIntersections.h"

class FindIntersections::BTree
{
	struct Node
	{
		std::vector<int> elements; // indices of elements in this node
	};

public:
	BTree(const FSMesh& mesh) : mesh(mesh) {}

	void build(int levels)
	{
		// get the bounding box of the entire mesh
		BOX bbox;
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			bbox += mesh.Node(i).pos();
		}
		// inflate a little to ensure that all nodes are strictly inside the box
		bbox.Inflate(1e-6);

		// get a list of all element indices
		std::vector<int> allElements(mesh.Elements());
		for (int i = 0; i < mesh.Elements(); ++i)
		{
			allElements[i] = i;
		}

		// build the tree recursively
		buildNode(bbox, allElements, levels);
	}

	void buildNode(const BOX& box, std::vector<int>& elements, int levels)
	{
		if ((levels == 0) || (elements.size() <= 10)) // base case: create a leaf node
		{
			Node node;
			node.elements = elements;
			leaves.push_back(node);
			return;
		}

		// split the box into two along the longest axis
		int axis = 0;
		double xlen = box.Width();
		double ylen = box.Height();
		double zlen = box.Depth();
		if ((xlen >= ylen) && (xlen >= zlen)) axis = 0;
		if ((ylen >= xlen) && (ylen >= zlen)) axis = 1;
		if ((zlen >= xlen) && (zlen >= ylen)) axis = 2;
		double mid = 0.5 * (box.r0()(axis) + box.r1()(axis));
		BOX leftBox = box;
		BOX rightBox = box;
		const double eps = 1e-6;
		switch (axis)
		{
		case 0: leftBox.x1 = mid + eps; rightBox.x0 = mid - eps; break;
		case 1: leftBox.y1 = mid + eps; rightBox.y0 = mid - eps; break;
		case 2: leftBox.z1 = mid + eps; rightBox.z0 = mid - eps; break;
		}

		std::vector<int> leftElements, rightElements;
		leftElements = getElementsInBox(leftBox, elements);
		rightElements = getElementsInBox(rightBox, elements);

		if (!leftElements.empty ()) buildNode(leftBox, leftElements, levels - 1);
		if (!rightElements.empty()) buildNode(rightBox, rightElements, levels - 1);
	}

	std::vector<int> getElementsInBox(const BOX& box, const std::vector<int>& elements)
	{
		std::vector<int> result;
		for (int elemIndex : elements)
		{
			const FSElement& elem = mesh.Element(elemIndex);
			for (int i=0; i<elem.Nodes(); ++i)
			{
				vec3d nodePos = mesh.Node(elem.m_node[i]).pos();
				if (box.IsInside(nodePos))
				{
					result.push_back(elemIndex);
					break; // no need to check other nodes of this element
				}
			}
		}
		return result;
	}

public:
	const FSMesh& mesh;
	std::vector<Node> leaves;
};

FindIntersections::FindIntersections(FSMesh& mesh) : m_mesh(mesh)
{
}

bool shareNodes(const FSElement& elem1, const FSElement& elem2)
{
	for (int i = 0; i < elem1.Nodes(); ++i)
	{
		for (int j = 0; j < elem2.Nodes(); ++j)
		{
			if (elem1.m_node[i] == elem2.m_node[j])
			{
				return true; // Found a shared node
			}
		}
	}
	return false; // No shared nodes found
}

std::vector<int> FindIntersections::FindIntersectingElements()
{
	std::vector<int> intersectingElements;

	// This only works for triangular meshes for now.
	if (!m_mesh.IsType(FE_TRI3))
	{
		return intersectingElements; // Return empty list if not a triangular mesh
	}

	// determine the number of levels for the BTree based on the number of elements
	int numElements = m_mesh.Elements();
	int levels = (int)log2(numElements / 10);
	if (levels <= 0) levels = 0;
	if (levels > 20) levels = 20; // limit the number of levels to prevent excessive memory usage

	// setup the BTree
	BTree btree(m_mesh);
	btree.build(levels);

	std::set<int> intersectingSet; // to avoid duplicates

	// loop over all the leaves of the BTree and check for intersections between elements in the same leaf
	for (int n = 0; n < btree.leaves.size(); ++n)
	{
		std::vector<int>& leafElements = btree.leaves[n].elements;

		// Loop over all elements in the leaf and check for intersections
		for (int i = 0; i < leafElements.size(); ++i)
		{
			FSElement& elem = m_mesh.Element(leafElements[i]);
			// Check if the element intersects with any other element
			for (int j = 0; j < leafElements.size(); ++j)
			{
				FSElement& otherElem = m_mesh.Element(leafElements[j]);

				// skip if elements share a node (they are connected and cannot intersect)
				if (shareNodes(elem, otherElem))
					continue;

				// Check for intersection between elem and otherElem
				if (Intersects(elem, otherElem))
				{
					intersectingSet.insert(leafElements[i]);
					break;
				}
			}
		}
	}

	// convert the set to a vector
	for (int elemIndex : intersectingSet)
	{
		intersectingElements.push_back(elemIndex);
	}
	return intersectingElements;
}

bool edgeTriangleIntersect(const vec3d& p0, const vec3d& p1, const vec3d v[3])
{
	vec3d e1 = v[1] - v[0];
	vec3d e2 = v[2] - v[0];
	double c12 = e1 * e2;
	if (c12 == 0.0) return false; // points are collinear so this is not a valid triangle

	// get the normal
	vec3d N = (e1 ^ e2);

	// get the intersection point
	vec3d t = p1 - p0;
	double det = N * t;
	if (det == 0.0) return false;
	double lam = -N * (p0 - v[0]) / det;
	if ((lam < 0) || (lam > 1.0)) return false;
	vec3d q = p0 + t * lam;

	// use edge tests to determine whether the point q lies inside the triangle
	vec3d c1 = (v[1] - v[0]) ^ (q - v[0]);
	vec3d c2 = (v[2] - v[1]) ^ (q - v[1]);
	vec3d c3 = (v[0] - v[2]) ^ (q - v[2]);
	double s1 = N * c1;
	double s2 = N * c2;
	double s3 = N * c3;

	if ((s1 >= 0) && (s2 >= 0) && (s3 >= 0)) return true;

	return false;
}

bool FindIntersections::Intersects(const FSElement& elem1, const FSElement& elem2)
{
	// get the vertex positions of the two elements (assumes triangular elements)
	vec3d v1[3], v2[3];
	for (int i = 0; i < 3; ++i)
	{
		v1[i] = m_mesh.Node(elem1.m_node[i]).pos();
		v2[i] = m_mesh.Node(elem2.m_node[i]).pos();
	}

	// see if any edge of elem1 intersects with elem2
	for (int i = 0; i < 3; ++i)
	{
		vec3d p0 = v1[i];
		vec3d p1 = v1[(i+1)%3];
		if (edgeTriangleIntersect(p0, p1, v2))
		{
			return true;
		}
	}
	
	return false;
}