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
#include <FSCore/box.h>
#include <GLLib/GView.h>
#include "GGrid.h"
#include <QString>

class CGLContext;

class CGLScene
{
public:
	CGLScene();
	virtual ~CGLScene();

	CGView& GetView();

	virtual void Render(CGLContext& rc) = 0;

	// get the bounding box of the entire scene
	virtual BOX GetBoundingBox() = 0;

	// get the bounding box of the current selection
	virtual BOX GetSelectionBox() = 0;

	CGLCamera& GetCamera() { return m_view.GetCamera(); }

public:
	GGrid& GetGrid() { return m_grid; }
	double GetGridScale() { return m_grid.GetScale(); }
	quatd GetGridOrientation() { return m_grid.m_q; }
	void SetGridOrientation(const quatd& q) { m_grid.m_q = q; }

	void SetEnvironmentMap(QString filename) { m_envMap = filename; }
	void ActivateEnvironmentMap();
	void DeactivateEnvironmentMap();
	void LoadEnvironmentMap();

protected:
	CGView	m_view;
	GGrid	m_grid;		// the grid object

	unsigned int	m_envtex;	// enironment texture ID
	QString m_envMap; // file name used for environment mapping 
};
