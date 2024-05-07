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
#include "GLScene.h"
#include <GLLib/glx.h>
#include <QImage>

CGLScene::CGLScene() 
{
	m_envtex = 0;
}

CGLScene::~CGLScene() {}

CGView& CGLScene::GetView() { return m_view; }

void CGLScene::Update()
{

}

void CGLScene::ActivateEnvironmentMap()
{
	if (m_envtex == 0) LoadEnvironmentMap();
	if (m_envtex == 0) return;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_envtex);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
}

void CGLScene::DeactivateEnvironmentMap()
{
	if (m_envtex == 0) return;

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
}

void CGLScene::LoadEnvironmentMap()
{
	if (m_envtex != 0) return;

	if (m_envMap.isEmpty()) return;

	// try to load the image
	QImage img;
	bool berr = img.load(m_envMap);
	if (berr == false) return;

	uchar* d = img.bits();
	int nx = img.width();
	int ny = img.height();

	// we need to flip and invert colors
	GLubyte* buf = new GLubyte[nx * ny * 3];

	GLubyte* b = buf;
	for (int j = ny - 1; j >= 0; --j)
		for (int i = 0; i < nx; ++i, b += 3)
		{
			GLubyte* s = d + (j * (4 * nx) + 4 * i);
			b[0] = s[2];
			b[1] = s[1];
			b[2] = s[0];
		}

	glGenTextures(1, &m_envtex);
	glBindTexture(GL_TEXTURE_2D, m_envtex);
	// set texture parameter for 2D textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nx, ny, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	delete[] buf;
}
