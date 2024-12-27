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
#include "GLPlot.h"

namespace Post {

class CGLMirrorPlane : public CGLPlot
{
public:
	enum { MAX_MIRROR_PLANES = 3 };
	enum { PLANE, SHOW_PLANE, TRANSPARENCY, OFFSET, RECURSION };

public:
	CGLMirrorPlane();
	~CGLMirrorPlane();

	// render the object to the 3D view
	void Render(GLRenderEngine& re, CGLContext& rc) override;

	bool UpdateData(bool bsave = true) override;

	static CGLMirrorPlane* GetMirrorPlane(int n);

private:
	void RenderPlane();

public:
	int		m_plane;
	float	m_transparency;
	bool	m_showPlane;
	float	m_offset;
	bool	m_recursive;

private:
	int		m_id;
	int m_render_id;

private:
	void AllocRenderID();
	void DeallocRenderID();
	static CGLMirrorPlane* m_mirrors[MAX_MIRROR_PLANES];
};
}
