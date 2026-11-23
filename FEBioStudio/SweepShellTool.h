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
#include <MeshTools/ShellSweeper.h>
#include <vector>

class UISweepShellTool;
class GObject;

// Sweeps a shell mesh (the quad-dominant cross-section) along a path defined by selected edges.
// Usage: 1) select the shell object and press "Set section"; 2) select the edge(s) that define the path
// and press "Set path"; 3) press "Sweep".
class CSweepShellTool : public CAbstractTool
{
	Q_OBJECT

public:
	CSweepShellTool(CMainWindow* wnd);

	QWidget* createUi() override;

	void Deactivate() override;

private slots:
	void OnSetSection();
	void OnSetPath();
	void OnSweep();

private:
	bool SectionIsValid();
	void UpdateLabels();
	void Error(const QString& msg);

private:
	UISweepShellTool*	ui;
	GObject*			m_section;
	std::vector<PathSegment>	m_segs;	// path segments in global coordinates
};
