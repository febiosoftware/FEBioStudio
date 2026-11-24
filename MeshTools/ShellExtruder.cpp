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
#include "ShellLayers.h"
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
	m_collapseTol = 0.0;
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

	// analyze the shell mesh (TRI3/QUAD4 -> PENTA6/HEX8, TRI6/QUAD8 -> PENTA15/HEX20)
	ShellSection sec;
	if (!sec.Init(shell, m_err)) return nullptr;
	const int nn = sec.nn;
	const int ne = sec.Elements();

	std::vector<vec3d> r0(nn);
	for (int i = 0; i < nn; ++i) r0[i] = shell.Node(sec.base[i]).r;

	// area-weighted element and node normals
	std::vector<vec3d> nodeNormal(nn, vec3d(0, 0, 0));
	vec3d avg(0, 0, 0);
	for (int e = 0; e < ne; ++e)
	{
		vec3d N = sec.AreaNormal(e, r0);
		avg += N;
		for (int k : sec.enodes[e]) nodeNormal[k] += N;
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

	const int nl = m_layers;
	std::vector<vec3d> layerPos((nl + 1) * nn);
	for (int l = 0; l <= nl; ++l)
		for (int i = 0; i < nn; ++i)
			layerPos[l * nn + i] = r0[i] + dir[i] * offset[l];

	// nodes half way between the layers (quadratic elements only)
	std::vector<vec3d> midPos;
	if (sec.quadratic)
	{
		midPos.resize(nl * sec.ncorner);
		for (int l = 0; l < nl; ++l)
			for (int c = 0; c < sec.ncorner; ++c)
			{
				int i = sec.cornerNode[c];
				midPos[l * sec.ncorner + c] = r0[i] + dir[i] * (0.5 * (offset[l] + offset[l + 1]));
			}
	}

	FSMesh* mesh = sec.BuildSolid(r0, nl, layerPos, midPos);
	if (!FinishSolidMesh(*mesh, m_collapseTol, m_autoPartition, m_creaseAngle, m_err))
	{
		delete mesh;
		return nullptr;
	}
	return mesh;
}
