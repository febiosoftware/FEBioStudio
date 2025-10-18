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
#include "rhiScene.h"

class GLMeshItem : public GLSceneItem
{
public:
	GLMeshItem(GLMesh* pm) : m_pm(pm) {}
	~GLMeshItem() { delete m_pm; }

	void render(GLRenderEngine& re, GLContext& rc) override
	{
		if (m_pm)
		{
			GLMaterial mat;
			mat.type = GLMaterial::PLASTIC;
			mat.ambient = mat.diffuse = color;
			mat.shininess = shininess;
			mat.reflectivity = reflectivity;
			mat.opacity = opacity;
			re.setMaterial(mat);
			re.renderGMesh(*m_pm);
		}
	}

public:
	GLColor color = GLColor(200, 180, 160);
	float shininess = 0.8f;
	float reflectivity = 0.8f;
	float opacity = 1.0f;

private:
	GLMesh* m_pm;
};

void rhiScene::AddMesh(GLMesh* pm)
{
	if (pm == nullptr) return;
	BOX box = pm->GetBoundingBox();
	m_box += box;
	addItem(new GLMeshItem(pm));
	ZoomExtents(false);
}

void rhiScene::Render(GLRenderEngine& re, GLContext& rc)
{
	GLCamera& cam = GetCamera();
	re.setProjection(45.0f, 0.1f);
	re.positionCamera(cam);
	re.setBackgroundColor(bgcol);
	re.setLightPosition(0, light);
	re.setLightSpecularColor(0, specColor);
	GLScene::Render(re, rc);
}

void rhiScene::SetObjectColor(GLColor col)
{
	GLItemIterator it = begin();
	for (int i = 0; i < items(); ++i, ++it)
	{
		GLMeshItem* m = dynamic_cast<GLMeshItem*>(*it);
		if (m) m->color = col;
	}
}

void rhiScene::SetObjectShininess(float f)
{
	GLItemIterator it = begin();
	for (int i = 0; i < items(); ++i, ++it)
	{
		GLMeshItem* m = dynamic_cast<GLMeshItem*>(*it);
		if (m) m->shininess = f;
	}
}

void rhiScene::SetObjectReflectivity(float f)
{
	GLItemIterator it = begin();
	for (int i = 0; i < items(); ++i, ++it)
	{
		GLMeshItem* m = dynamic_cast<GLMeshItem*>(*it);
		if (m) m->reflectivity = f;
	}
}

void rhiScene::SetObjectOpacity(float f)
{
	GLItemIterator it = begin();
	for (int i = 0; i < items(); ++i, ++it)
	{
		GLMeshItem* m = dynamic_cast<GLMeshItem*>(*it);
		if (m) m->opacity = f;
	}
}
