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
#include "GLPlotStaticMesh.h"
#include <FSCore/ClassDescriptor.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;
using namespace std;

REGISTER_CLASS(GLPlotStaticMesh, CLASS_PLOT, "static_mesh", 0);

GLPlotStaticMesh::GLPlotStaticMesh()
{
	SetTypeString("static_mesh");

	static int n = 1;
	char sz[256] = { 0 };
	sprintf(sz, "StaticMesh%d", n++);
	SetName(sz);

	AddVecParam(vec3d(0, 0, 0), "translate");
	AddVecParam(vec3d(0, 0, 0), "rotate");
	AddDoubleParam(1, "scale")->SetFloatRange(0, 10, 0.01);
	AddColorParam(GLColor(220, 200, 200), "color");

	// set the render-order to 1, so this gets drawn after the model is drawn
	SetRenderOrder(1);
}

void GLPlotStaticMesh::Render(GLRenderEngine& re, GLContext& rc)
{
	if (mesh.Nodes() == 0) return;

	vec3d p = GetVecValue(POS);
	vec3d q = GetVecValue(ROT);
	q *= DEG2RAD;
	quatd Q(q);
	GLColor c = GetColorValue(COLOR);
	double s = GetFloatValue(SCALE);
	re.pushTransform();
	re.translate(p);
	re.rotate(Q);
	re.scale(s, s, s);

	re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::NONE, false);
	re.renderGMesh(mesh, true);
	re.popTransform();
}
