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

// Thin interface to the robust hex-dominant meshing code of
//   X. Gao, W. Jakob, M. Tarini, D. Panozzo, "Robust Hex-Dominant Mesh Generation
//   using Field-Guided Polyhedral Agglomeration", ACM TOG (SIGGRAPH) 2017.
//   https://github.com/gaoxifeng/robust_hex_dominant_meshing (MIT license)
// This header deliberately uses only standard types so that callers do not need Eigen.
#pragma once
#include <vector>
#include <string>
#include <functional>

namespace rhdm {

struct HexDomParams
{
	double	edgeLength = 0.0;		// target hex edge length (<= 0: automatic, 3.5 x average boundary edge length)
	int		iterationsPerLevel = 200;	// field smoothing iterations per hierarchy level (upstream uses 200 for tet meshes)
	int		threads = 0;			// worker threads (0 = all cores, 1 = serial/deterministic)
	bool	splitting = true;		// allow vertex/face insertion during extraction (upstream default)

	// Optional sharp features of the boundary, one entry per node (or empty): 0 = none,
	// 1 = on a sharp edge (featureDir holds its unit tangent, 3 values per node), 2 = corner.
	// The fields are then aligned so that lattice lines run along the sharp edges and lattice
	// points sit on the corners (FEBio Studio addition; upstream ignores sharp features).
	std::vector<int>	featureType;
	std::vector<double>	featureDir;
};

struct HexDomResult
{
	std::vector<double>					nodes;		// 3*N node coordinates
	std::vector<std::vector<int> >		faces;		// polygonal faces (node indices)
	std::vector<std::vector<int> >		cells;		// polyhedral cells (face indices)
	std::vector<bool>					isHex;		// true if cell is a hexahedron (6 quad faces)
	double								edgeLength = 0.0;	// edge length that was actually used
	int									hierarchyLevels = 0;
};

// progress callback: (fraction in [0,1], stage description). Return false to cancel.
typedef std::function<bool(double, const char*)> HexDomProgress;

// Build a hex-dominant mesh from a linear tetrahedral mesh.
//   nodes : 3*N node coordinates
//   tets  : 4*M zero-based node indices
// Returns false on failure (or cancel); errMsg is set in that case.
bool HexDominantMesh(const std::vector<double>& nodes, const std::vector<int>& tets,
	const HexDomParams& params, HexDomResult& result, std::string& errMsg,
	HexDomProgress progress = nullptr);

} // namespace rhdm
