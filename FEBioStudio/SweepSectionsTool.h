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
#include <FSCore/math3d.h>
#include <vector>

class UISweepSectionsTool;
class GObject;

// Sweeps a meshed cross-section (selected surface or faces of an existing mesh) through a series of other
// cross-sections into a solid hex/penta mesh. Only the start section's mesh is used: the other (target) sections
// contribute only their position, orientation and size. The path is a smooth curve through the centroids of the
// sections (circular arcs where the section normals allow it), and the swept section is rotated to follow the path and
// scaled to the size of each target.
// Usage: 1) select the start surface/faces, press "Set start section"; 2) select the target surface(s)/faces, press
// "Set target sections" (or "Add target sections"); 3) press "Sweep".
class CSweepSectionsTool : public CAbstractTool
{
	Q_OBJECT

public:
	CSweepSectionsTool(CMainWindow* wnd);

	QWidget* createUi() override;

private slots:
	void OnSetStart();
	void OnSetTargets();
	void OnAddTargets();
	void OnClearTargets();
	void OnSweep();

public:
	struct Section
	{
		GObject*	po = nullptr;
		std::vector<int> faces;		// face indices into po->GetFEMesh()
		bool		wholeObject = false;	// the section is the entire shell mesh of po (all its elements)
		bool		geometric = false;		// unmeshed surface(s): only the geometry (render mesh) was used
		vec3d		c;				// centroid (global)
		vec3d		n;				// area-weighted normal (global)
		double		area = 0.0;
	};

private:
	bool GetSections(std::vector<Section>& sections, bool forStart, QString& err);
	bool ObjectExists(GObject* po);
	void UpdateLabels();
	void Error(const QString& msg);

private:
	UISweepSectionsTool*	ui;
	Section					m_start;
	bool					m_haveStart;
	std::vector<Section>	m_targets;
};
