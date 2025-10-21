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
#include <FSCore/ColorMapManager.h>

class GLMeshItem : public GLSceneItem
{
public:
	GLMeshItem(GLMesh* pm) : m_pm(pm) 
	{
		// set some texture coordinates
		if (m_pm)
		{
			BOX box = m_pm->GetBoundingBox();
			for (int i = 0; i < m_pm->Faces(); ++i)
			{
				GLMesh::FACE& face = m_pm->Face(i);
				for (int j = 0; j < 3; ++j)
				{
					face.t[j].x = (face.vr[j].x - box.x0)/box.Width();
					face.t[j].y = 0;
					face.t[j].z = 0;
				}
			}
		}
	}
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
			mat.diffuseMap = (useTexture ? GLMaterial::TEXTURE_1D : GLMaterial::NONE);
			re.setMaterial(mat);
			re.renderGMesh(*m_pm);

			if (renderMesh)
			{
				re.setColor(meshColor);
				re.renderGMeshEdges(*m_pm);
			}

			if (renderNodes)
			{
				re.setColor(nodeColor);
				re.renderGMeshNodes(*m_pm);
			}
		}
	}

public:
	GLColor color = GLColor(200, 180, 160);
	float shininess = 0.8f;
	float reflectivity = 0.8f;
	float opacity = 1.0f;
	bool useTexture = false;
	bool renderMesh = false;
	GLColor meshColor = GLColor(0, 0, 0);
	bool renderNodes = false;
	GLColor nodeColor = GLColor(0, 0, 0);

private:
	GLMesh* m_pm;
};

rhiScene::rhiScene()
{
	tex1d.Create(ColorMapManager::JET, 256, true);
}

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
	re.setProjection(45.0f, 0.01f);
	re.positionCamera(cam);
	re.setBackgroundColor(bgcol);
	re.setLightPosition(0, light);
	re.setLightSpecularColor(0, specColor);

	if (oldTexture != texture)
	{
		oldTexture = texture;
		switch (texture)
		{
		case 1: tex1d.Create(ColorMapManager::JET   , 1024, true); break;
		case 2: tex1d.Create(ColorMapManager::PARULA, 1024, true); break;
		case 3: tex1d.Create(ColorMapManager::GRAY  , 1024, true); break;
			break;
		}
		if (texture > 0) re.setTexture(tex1d.GetTexture());
	}

	GLItemIterator it = begin();
	for (int i = 0; i < items(); ++i, ++it)
	{
		GLMeshItem* m = dynamic_cast<GLMeshItem*>(*it);
		if (m)
		{
			m->useTexture = (texture > 0);
			m->renderMesh = renderMesh;
			m->meshColor = meshColor;
			m->renderNodes = renderNodes;
			m->nodeColor = nodeColor;
		}
	}

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
