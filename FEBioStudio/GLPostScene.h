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
#include "GLViewScene.h"

class CGLModelDocument;
class CGLPostScene;
class CImageModel;
class FESelection;

namespace Post {
	class CGLPlot;
	class FEPostModel;
	class CGLModel;
}

class GLPostSceneItem : public GLSceneItem
{
public:
	GLPostSceneItem(CGLPostScene* scene) : m_scene(scene) {}

protected:
	CGLPostScene* m_scene;
};

class GLPostModelItem : public GLPostSceneItem
{
public:
	GLPostModelItem(CGLPostScene* scene) : GLPostSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void RenderModel(GLRenderEngine& re, GLContext& rc);
	void RenderNodes(GLRenderEngine& re, GLContext& rc);
	void RenderEdges(GLRenderEngine& re);
	void RenderFaces(GLRenderEngine& re, GLContext& rc);
	void RenderElems(GLRenderEngine& re, GLContext& rc);
	void RenderSelection(GLRenderEngine& re);
	void RenderNormals(GLRenderEngine& re);
	void RenderOutline(GLRenderEngine& re, GLContext& rc);
	void RenderGhost(GLRenderEngine& re);
	void RenderMeshLines(GLRenderEngine& re, GLContext& rc);
	void RenderDiscrete(GLRenderEngine& re, GLContext& rc);
	void RenderDiscreteAsLines(GLRenderEngine& re, GLContext& rc);
	void RenderDiscreteAsSolid(GLRenderEngine& re);
	void RenderMinMaxMarkers(GLRenderEngine& re);
	void RenderDiscreteElement(GLRenderEngine& re, int n);
	void RenderDiscreteElementAsSolid(GLRenderEngine& re, int n, double W);

private:
	void RenderMesh(GLRenderEngine& re, GLMesh& mesh, int surfId);
	void RenderMeshEdges(GLRenderEngine& re, GLMesh& mesh);

private:
	GLTexture1D	m_tex;
};

class GLPostPlotItem : public GLPostSceneItem
{
public:
	GLPostPlotItem(CGLPostScene* scene) : GLPostSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;
};

class GLPostPlaneCutItem : public GLCompositeSceneItem
{
public:
	GLPostPlaneCutItem(CGLPostScene* scene) : m_scene(scene) {}

	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	CGLPostScene* m_scene;
};

class GLPostMirrorItem : public GLCompositeSceneItem
{
public:
	GLPostMirrorItem(CGLPostScene* scene) : m_scene(scene) {}

	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void renderMirror(GLRenderEngine& re, GLContext& rc, int start, int end);

private:
	CGLPostScene* m_scene;
};

class GLPostObjectItem : public GLPostSceneItem
{
public:
	GLPostObjectItem(CGLPostScene* scene) : GLPostSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;
};

class GLPost3DImageItem : public GLPostSceneItem
{
public:
	GLPost3DImageItem(CImageModel* img, CGLPostScene* scene) : GLPostSceneItem(scene), m_img(img) {}

	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	CImageModel* m_img;
};

class CGLPostScene : public GLViewScene
{
public:
	CGLPostScene(CGLModelDocument* doc);

	void Render(GLRenderEngine& engine, GLContext& rc) override;

	BOX GetBoundingBox() override;

	BOX GetSelectionBox() override;

	void ToggleTrackSelection();

	Post::CGLModel* GetGLModel();

	Post::FEPostModel* GetFSModel();

	FESelection* GetSelection();

	int GetItemMode() const;

	void Update() override;

	LegendData GetLegendData(int n) override;

private:
	void CreateTags(GLContext& rc);

	void UpdateTracking();

private:
	void BuildScene();

private:
	CGLModelDocument* m_doc;

	bool	m_btrack;
	int		m_ntrack[3];	// used for tracking
	double	m_trackScale;
	vec3d	m_trgPos;
	quatd	m_trgRot0;
	quatd	m_trgRot;
	quatd	m_trgRotDelta;

	bool	m_buildScene;
};
