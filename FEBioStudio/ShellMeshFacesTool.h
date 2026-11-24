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
#include "Tool.h"

// Creates a quad-dominant shell mesh (using NetGen) of the selected faces of an OCC (e.g. STEP) object.
// The result is added to the model as a new shell object, which can then be extruded or swept.
class CShellMeshFacesTool : public CBasicTool
{
	Q_OBJECT

public:
	CShellMeshFacesTool(CMainWindow* wnd);

	bool OnApply() override;

private:
	double	m_elemSize;		// max element size (<= 0 --> use the parent object's mesher setting)
	bool	m_quadratic;	// generate second-order elements (TRI6/QUAD8, and HEX20/PENTA15 when extruding)
	bool	m_quad;			// quad dominant (otherwise pure triangles)
	bool	m_hideSource;	// hide the original object after creating the shell

	// optional extrusion (sweep) of the shell mesh into a solid mesh
	bool	m_extrude;		// extrude the shell into a solid (hex/penta) object
	int		m_dirMode;		// 0 = local normal, 1 = average normal, 2 = user-defined direction
	vec3d	m_dir;			// user-defined direction (global coordinates)
	bool	m_flip;			// reverse the extrusion direction
	double	m_distance;		// extrusion distance
	int		m_layers;		// number of layers
	double	m_bias;			// layer thickness ratio
	bool	m_keepShell;	// also keep the shell object when extruding

	// surface partitioning of the new (non-CAD) object
	bool	m_autoPartition;	// automatically partition the surface based on the crease angle
	double	m_creaseAngle;		// crease angle in degrees
	double	m_collapseTol;		// tolerance for eliminating degenerate elements (0 = automatic)
};
