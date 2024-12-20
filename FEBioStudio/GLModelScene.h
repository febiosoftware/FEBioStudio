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
#include "GLPlaneCut.h"
#include <GLLib/GLMeshRender.h>

enum OBJECT_COLOR_MODE {
	DEFAULT_COLOR,
	OBJECT_COLOR,
	MATERIAL_TYPE,
	FSELEMENT_TYPE,
	PHYSICS_TYPE
};

class CModelDocument;
class GPart;
class GLFiberRenderer;
class CGLModelScene;
class GModel;
class FSModel;

class GLModelSceneItem : public GLSceneItem
{
public:
	GLModelSceneItem(CGLModelScene* scene) : m_scene(scene) {}

protected:
	CGLModelScene* m_scene;
};

class GLPlaneCutItem : public GLCompositeSceneItem
{
public:
	GLPlaneCutItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	void UpdatePlaneCut(CGLContext& rc, bool reset);
	void RenderBoxCut(GLRenderEngine& re, CGLContext& rc, const BOX& box);

private:
	CGLModelScene* m_scene;
	GLPlaneCut	m_planeCut;
};

class GLObjectItem : public GLModelSceneItem
{
public:
	GLObjectItem(CGLModelScene* scene, GObject* po) : GLModelSceneItem(scene), m_po(po) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	void RenderGObject(GLRenderEngine& re, CGLContext& rc) const;

	void RenderObject(GLRenderEngine& re, CGLContext& rc) const;
	void RenderSurfaceMeshFaces(GLRenderEngine& re, CGLContext& rc) const;

	void RenderParts(GLRenderEngine& re, CGLContext& rc) const;
	void RenderSurfaces(GLRenderEngine& re, CGLContext& rc) const;
	void RenderEdges(GLRenderEngine& re, CGLContext& rc) const;
	void RenderNodes(GLRenderEngine& re, CGLContext& rc) const;
	void RenderBeamParts(GLRenderEngine& re, CGLContext& rc) const;
	void RenderSurfaceMeshEdges(CGLContext& rc) const;

	void RenderFEFacesFromGMesh(GLRenderEngine& re, CGLContext& rc) const;
	void RenderMeshByDefault(GLRenderEngine& re, CGLContext& rc) const;
	void RenderMeshByObjectColor(GLRenderEngine& re, CGLContext& rc) const;
	void RenderMeshByElementType(CGLContext& rc, GMesh& mesh) const;

	void RenderUnselectedBeamElements(CGLContext& rc) const;
	void RenderSelectedFEElements(GLRenderEngine& re, CGLContext& rc) const;
	void RenderAllBeamElements(GLRenderEngine& re, CGLContext& rc) const;
	void RenderSelectedFEFaces(GLRenderEngine& re, CGLContext& rc) const;
	void RenderFEEdges(GLRenderEngine& re, CGLContext& rc) const;
	void RenderFENodes(GLRenderEngine& re, CGLContext& rc) const;
	void RenderSurfaceMeshNodes(CGLContext& rc) const;
	void RenderNormals(CGLContext& rc, double scale) const;

	void RenderSelection(CGLContext& rc) const;

private:
	GObject* m_po;
};

class GLDiscreteItem : public GLModelSceneItem
{
	struct Line {
		vec3d a, b;
		GLColor col;
	};

public:
	GLDiscreteItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	std::vector<Line> m_lines;
};

class GLSelectionBox : public GLModelSceneItem
{
public:
	GLSelectionBox(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;
};

class GLMeshLinesItem : public GLModelSceneItem
{
public:
	GLMeshLinesItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;
};

class GLFeatureEdgesItem : public GLModelSceneItem
{
public:
	GLFeatureEdgesItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;
};

class GLPhysicsItem : public GLModelSceneItem
{
public:
	GLPhysicsItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	void RenderRigidBodies(GLRenderEngine& re, CGLContext& rc) const;
	void RenderRigidJoints(GLRenderEngine& re, CGLContext& rc) const;
	void RenderRigidConnectors(GLRenderEngine& re, CGLContext& rc) const;
	void RenderRigidWalls(GLRenderEngine& re, CGLContext& rc) const;
	void RenderMaterialFibers(GLRenderEngine& re, CGLContext& rc) const;
	void RenderLocalMaterialAxes(GLRenderEngine& re, CGLContext& rc) const;
};

class GLSelectionItem : public GLModelSceneItem
{
public:
	GLSelectionItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	void RenderSelectedParts(GLRenderEngine& re, CGLContext& rc, GObject* po) const;
	void RenderSelectedSurfaces(GLRenderEngine& re, CGLContext& rc, GObject* po) const;
	void RenderSelectedEdges(GLRenderEngine& re, CGLContext& rc, GObject* po) const;
	void RenderSelectedNodes(GLRenderEngine& re, CGLContext& rc, GObject* po) const;
};

class GLHighlighterItem : public GLModelSceneItem
{
public:
	GLHighlighterItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	void drawNode(GLRenderEngine& re, CGLContext& rc, GNode* node, GLColor c);
	void drawEdge(GLRenderEngine& re, CGLContext& rc, GEdge* edge, GLColor c);
	void drawFace(GLRenderEngine& re, CGLContext& rc, GFace* face, GLColor c);
	void drawPart(GLRenderEngine& re, CGLContext& rc, GPart* part, GLColor c);

	void drawFENodeSet(CGLContext& rc, GLMeshRender& renderer, FSNodeSet* nodeSet, GLColor c);
	void drawFESurface(CGLContext& rc, GLMeshRender& renderer, FSSurface* surf, GLColor c);

private:
	GLColor			m_activeColor;		// color of active item
	GLColor			m_pickColor[2];		// color of picked items
};

class GLGridItem : public GLModelSceneItem
{
public:
	GLGridItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;
};

class GL3DImageItem : public GLModelSceneItem
{
public:
	GL3DImageItem(CGLModelScene* scene, CImageModel* img) : GLModelSceneItem(scene), m_img(img) {}
	void render(GLRenderEngine& re, CGLContext& rc) override;

private:
	CImageModel* m_img;
};

class CGLModelScene : public CGLScene
{
public:
	CGLModelScene(CModelDocument* doc);

	void Render(GLRenderEngine& engine, CGLContext& rc) override;

	GLMeshRender& GetMeshRenderer();

	BOX GetBoundingBox() override;

	BOX GetSelectionBox() override;

	void SetObjectColorMode(OBJECT_COLOR_MODE colorMode);

	OBJECT_COLOR_MODE ObjectColorMode() const;

	void Update() override;

	void UpdateFiberViz();

	GMesh& GetSelectionMesh() { return m_selectionMesh; }

	void UpdateSelectionMesh(FESelection* sel);

	GModel* GetGModel();

	FSModel* GetFSModel();

	int GetSelectionMode() const;

	int GetItemMode() const;

	int GetMeshMode() const;

	int GetObjectColorMode() const;

	GObject* GetActiveObject() const;

	GLFiberRenderer* GetFiberRenderer();

	FESelection* GetCurrentSelection();

private:
	void BuildScene(CGLContext& rc);

public:
	void RenderRigidLabels(CGLContext& rc);

	void RenderTags(CGLContext& rc);

public:
	GLColor GetPartColor(CGLContext& rc, GPart* pg);
	GLColor GetFaceColor(CGLContext& rc, GFace& face);

	void BuildFiberViz(CGLContext& rc);

	void UpdateRenderTransforms(CGLContext& rc);

private:
	CModelDocument* m_doc;
	GLMeshRender	m_renderer;
	GMesh			m_selectionMesh;
	
	OBJECT_COLOR_MODE	m_objectColor;

	bool m_buildScene;

	GLFiberRenderer* m_fiberViz;
};
