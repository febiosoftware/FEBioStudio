/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include <GLLib/GLMesh.h>
#include <GLLib/GLScene.h>
#include <GLLib/ColorTexture.h>

class rhiScene : public GLScene
{
public:
	rhiScene();

	void AddMesh(GLMesh* pm);

	void Render(GLRenderEngine& re, GLContext& rc) override;

	void SetObjectColor(GLColor col);
	void SetObjectShininess(float f);
	void SetObjectReflectivity(float f);
	void SetObjectOpacity(float f);

	// get the bounding box of the entire scene
	BOX GetBoundingBox() override { return m_box; }

public:
	GLColor bgcol = GLColor(200, 200, 255);
	vec3f light = vec3f(1, 1, 1);
	GLColor specColor = GLColor::White();
	int texture = 0;
	int oldTexture = 0;
	bool renderMesh = false;
	bool renderNodes = false;
	CColorTexture tex1d;
	GLColor meshColor = GLColor(0, 0, 0);
	GLColor nodeColor = GLColor(0, 0, 0);
	bool useStipple = false;
	bool doClipping = false;
	double clipPlane[4] = { 0,0,1,0 }; // plane equation coefficients
	bool showGrid = false;
	bool renderOverlay = false;

private:
	BOX m_box;
};
