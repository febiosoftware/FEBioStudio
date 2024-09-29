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
#include "GLMesh.h"
#include <MeshLib/GMesh.h>

class GLTexture1D;

class GLShader
{
public:
	GLShader() {}
	virtual ~GLShader() {}

	virtual void Activate() {}
	virtual void Deactivate() {}

	virtual void Render(const GMesh::FACE& face) {}
};

class GLStandardShader : public GLShader
{
public:
	float ambient[4] = { 0.f, 0.f, 0.f, 1.f };
	float diffuse[4] = { 0.f, 0.f, 0.f, 1.f };
	float specular[4] = { 0.f, 0.f, 0.f, 1.f };
	float emission[4] = { 0.f, 0.f, 0.f, 1.f };
	float shininess = 0.f;

	void Activate();

	void Render(const GMesh::FACE& face);
};

class GLTexture1DShader : public GLShader
{
public:
	void SetTexture(GLTexture1D* tex);

	void Activate() override;
	void Deactivate() override;
	void Render(const GMesh::FACE& face) override;

private:
	GLTexture1D* m_tex;
};

class GLStandardModelShader : public GLShader
{
public:
	GLStandardModelShader();
	GLStandardModelShader(const GLColor& c);

	void SetColor(const GLColor& c) { m_col = c; }

	void Activate() override;
	void Deactivate() override;
	void Render(const GMesh::FACE& face) override;

private:
	GLColor m_col;
};

class GLFaceColorShader : public GLShader
{
public:
	void Activate() override;
	void Render(const GMesh::FACE& face) override;
};
