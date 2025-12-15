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
#include <GLLib/GLTexture1D.h>
#include <GLLib/GView.h>
#include <PostLib/GLObject.h>
#include <PostLib/DataMap.h>
#include <FSCore/ColorMap.h>
#include <FSCore/ColorMapManager.h>
#include <FSCore/box.h>
#include <GLLib/ColorTexture.h>
#include <string>

// used for intersection testing
// defined in MeshLib/Intersect.h
struct Ray;
struct Intersection;
class FESelection;

namespace Post {

class CGLModel;
class GLPlotGroup;

class CGLPlot : public CGLVisual
{
protected:
	struct SUBELEMENT
	{
		float   vf[8];		// vector values
		float    h[8][8];	// shapefunctions
	};

public:
	CGLPlot(CGLModel* po = 0);
	virtual ~CGLPlot();

	virtual void UpdateTexture();

	void SetRenderOrder(int renderOrder);
	int GetRenderOrder() const;

	virtual void Reload();

	void SetGroup(GLPlotGroup* pg);
	GLPlotGroup* GetGroup();

public:
	virtual bool Intersects(Ray& ray, Intersection& q);

	virtual FESelection* SelectComponent(int index);

	virtual void ClearSelection();

	virtual LegendData GetLegendData() const { LegendData l; return l; }

private:
	int	m_renderOrder;
	GLPlotGroup* m_pgroup;	// parent group the plot belongs to
};

}
