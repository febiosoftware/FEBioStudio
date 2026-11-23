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
#include "ShellExtruder.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshBuilder.h>
#include <vector>
#include <cmath>

ShellExtruder::ShellExtruder()
{
	m_mode = LOCAL_NORMAL;
	m_direction = vec3d(0, 0, 1);
	m_flip = false;
	m_distance = 1.0;
	m_layers = 1;
	m_bias = 1.0;
	m_autoPartition = false;
	m_creaseAngle = 30.0;
}

FSMesh* ShellExtruder::Apply(const FSMesh& shell)
{
	m_err.clear();
	if (m_layers < 1) { m_err = "The number of layers must be at least 1."; return nullptr; }
	if (m_distance == 0.0) { m_err = "The extrusion distance must be nonzero."; return nullptr; }
	// a negative distance extrudes against the direction
	const bool reverse = (m_flip != (m_distance < 0.0));
	const double distance = fabs(m_distance);
	if (m_bias <= 0.0) { m_err = "The mesh bias must be positive."; return nullptr; }

	// collect the shell elements and the nodes they use
	const int N0 = shell.Nodes();
	std::vector<int> nodeMap(N0, -1);
	std::vector<int> elems;
	int nn = 0;
	for (int i = 0; i < shell.Elements(); ++i)
	{
		const FSElement& el = shell.Element(i);
		int ne = 0;
		if (el.Type() == FE_TRI3) ne = 3;
		else if (el.Type() == FE_QUAD4) ne = 4;
		else { m_err = "Only linear triangle and quad shell elements can be extruded (second-order shell meshes are not supported)."; return nullptr; }

		elems.push_back(i);
		for (int k = 0; k < ne; ++k)
			if (nodeMap[el.m_node[k]] == -1) nodeMap[el.m_node[k]] = nn++;
	}
	if (elems.empty()) { m_err = "The shell mesh has no elements."; return nullptr; }

	std::vector<int> base(nn);
	for (int i = 0; i < N0; ++i) if (nodeMap[i] >= 0) base[nodeMap[i]] = i;

	// area-weighted element and node normals
	std::vector<vec3d> nodeNormal(nn, vec3d(0, 0, 0));
	std::vector<vec3d> elemNormal(elems.size());
	vec3d avg(0, 0, 0);
	for (size_t e = 0; e < elems.size(); ++e)
	{
		const FSElement& el = shell.Element(elems[e]);
		vec3d N;
		if (el.Type() == FE_TRI3)
		{
			vec3d a = shell.Node(el.m_node[0]).r, b = shell.Node(el.m_node[1]).r, c = shell.Node(el.m_node[2]).r;
			N = ((b - a) ^ (c - a)) * 0.5;
		}
		else
		{
			vec3d a = shell.Node(el.m_node[0]).r, b = shell.Node(el.m_node[1]).r, c = shell.Node(el.m_node[2]).r, d = shell.Node(el.m_node[3]).r;
			N = ((c - a) ^ (d - b)) * 0.5;
		}
		elemNormal[e] = N;
		avg += N;
		int ne = (el.Type() == FE_TRI3 ? 3 : 4);
		for (int k = 0; k < ne; ++k) nodeNormal[nodeMap[el.m_node[k]]] += N;
	}

	// the extrusion direction at each node
	std::vector<vec3d> dir(nn);
	if (m_mode == USER_DIRECTION)
	{
		double L = m_direction.norm();
		if (L == 0.0) { m_err = "The extrusion direction is a zero vector."; return nullptr; }
		for (int i = 0; i < nn; ++i) dir[i] = m_direction / L;
	}
	else if (m_mode == AVERAGE_NORMAL)
	{
		double L = avg.norm();
		if (L < 1e-12) { m_err = "The average normal of the faces is zero (e.g. a closed or strongly curved surface). Use the local normal option or specify a direction."; return nullptr; }
		for (int i = 0; i < nn; ++i) dir[i] = avg / L;
	}
	else
	{
		for (int i = 0; i < nn; ++i)
		{
			double L = nodeNormal[i].norm();
			if (L < 1e-12) { m_err = "Could not determine a normal at one of the nodes. Specify a direction instead."; return nullptr; }
			dir[i] = nodeNormal[i] / L;
		}
	}
	if (reverse) for (int i = 0; i < nn; ++i) dir[i] = -dir[i];

	// layer offsets (geometric progression for the layer thickness)
	std::vector<double> offset(m_layers + 1, 0.0);
	{
		double t0 = (fabs(m_bias - 1.0) < 1e-12) ? distance / m_layers : distance * (1.0 - m_bias) / (1.0 - pow(m_bias, m_layers));
		double t = t0;
		for (int l = 1; l <= m_layers; ++l)
		{
			offset[l] = offset[l - 1] + t;
			t *= m_bias;
		}
	}

	// build the solid mesh
	int nl = m_layers;
	FSMesh* mesh = new FSMesh();
	mesh->Create(nn * (nl + 1), (int)elems.size() * nl);
	for (int l = 0; l <= nl; ++l)
		for (int i = 0; i < nn; ++i)
			mesh->Node(l * nn + i).r = shell.Node(base[i]).r + dir[i] * offset[l];

	int ie = 0;
	for (int l = 0; l < nl; ++l)
	{
		for (size_t e = 0; e < elems.size(); ++e)
		{
			const FSElement& sel = shell.Element(elems[e]);
			FSElement& el = mesh->Element(ie++);
			el.m_gid = 0;

			int ne = (sel.Type() == FE_TRI3 ? 3 : 4);
			int v[4];
			for (int k = 0; k < ne; ++k) v[k] = nodeMap[sel.m_node[k]];

			// the base face normal must point to the top face for a positive Jacobian
			vec3d dm(0, 0, 0);
			for (int k = 0; k < ne; ++k) dm += dir[v[k]];
			if (elemNormal[e] * dm < 0)
			{
				if (ne == 3) std::swap(v[1], v[2]);
				else std::swap(v[1], v[3]);
			}

			int b = l * nn, t = (l + 1) * nn;
			if (ne == 3)
			{
				el.SetType(FE_PENTA6);
				for (int k = 0; k < 3; ++k) { el.m_node[k] = b + v[k]; el.m_node[k + 3] = t + v[k]; }
			}
			else
			{
				el.SetType(FE_HEX8);
				for (int k = 0; k < 4; ++k) { el.m_node[k] = b + v[k]; el.m_node[k + 4] = t + v[k]; }
			}
		}
	}

	mesh->RebuildMesh();
	if (m_autoPartition)
	{
		// same as the Auto Partition modifier in the Mesh panel
		FSMeshBuilder meshBuilder(*mesh);
		meshBuilder.AutoPartition(m_creaseAngle);
	}
	return mesh;
}
