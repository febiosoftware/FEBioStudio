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

// Cuts cross-sections of a vessel-like triangulated surface (e.g. an artery STL with two open ends), spaced along the
// centerline (uniformly or adaptively), and optionally sweeps the first (quad-dominant meshed) section through all of them
// into a hex-dominant solid mesh (Sweep Sections).
class CVesselSectionsTool : public CBasicTool
{
	Q_OBJECT

public:
	CVesselSectionsTool(CMainWindow* wnd);

	bool OnApply() override;

private:
	int		m_intervals;		// number of intervals along the centerline (sections = intervals + 1)
	bool	m_smoothCaps;		// smooth the outline of the cap(s) the sweeps start from
	double	m_dElemSize;		// bifurcation: element size of the branches' outlet meshes (0 = match the trunk at the junction)
	int		m_dIntervals;		// bifurcation: intervals in each daughter branch
	bool	m_branchFromTrunk;	// bifurcation: start the branches from the trunk's end faces
	double	m_adapt;			// spacing adaptivity (0 = uniform)
	double	m_elemSize;			// element size of the start section's mesh (0 = auto)
	bool	m_quadratic;		// second-order elements
	int		m_action;			// 0 = create the sections only, 1 = create the sections and sweep
	int		m_layers;			// sweep: layers between consecutive sections
	bool	m_keepSections;		// sweep: also keep the section objects
	bool	m_alignTwist;		// sweep: rotate sections in-plane (max 15 degrees each) to align outlines
	bool	m_autoPartition;	// sweep: auto-partition the surface
	double	m_creaseAngle;
	double	m_collapseTol;
	bool	m_hideSource;		// hide the original surface object
};
