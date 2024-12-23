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
#include "Document.h"

class CPostDocument;
class CGLPostScene;
class CImageModel;

class GLPostSceneItem : public GLSceneItem
{
public:
	GLPostSceneItem(CGLPostScene* scene) : m_scene(scene) {}

	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	CGLPostScene* m_scene;
};

class GLPost3DImageItem : public GLSceneItem
{
public:
	GLPost3DImageItem(CImageModel* img, CGLPostScene* scene) : m_scene(scene), m_img(img) {}

	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	CGLPostScene* m_scene;
	CImageModel* m_img;
};

class CGLPostScene : public CGLScene
{
public:
	CGLPostScene(CPostDocument* doc);

	void Render(GLRenderEngine& engine, CGLContext& rc) override;

	BOX GetBoundingBox() override;

	BOX GetSelectionBox() override;

	void ToggleTrackSelection();

	Post::CGLModel* GetGLModel();

	int GetItemMode() const;

private:
	void RenderTags(CGLContext& rc);

	void UpdateTracking();

private:
	void BuildScene(CGLContext& rc);

private:
	CPostDocument* m_doc;

	bool	m_btrack;
	int		m_ntrack[3];	// used for tracking
	double	m_trackScale;
	vec3d	m_trgPos;
	quatd	m_trgRot0;
	quatd	m_trgRot;
	quatd	m_trgRotDelta;

	bool	m_buildScene;
};
