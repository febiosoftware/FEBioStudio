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
#include <GLLib/GLScene.h>

enum OBJECT_COLOR_MODE {
	DEFAULT_COLOR,
	OBJECT_COLOR,
	MATERIAL_TYPE,
	FSELEMENT_TYPE,
	PHYSICS_TYPE
};

class CModelDocument;
class GNode;
class GEdge;
class GFace;
class GPart;
class GLFiberRenderer;
class CGLModelScene;
class GModel;
class FSModel;
class FESelection;
class CImageModel;

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
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void UpdatePlaneCut(GLContext& rc, bool reset);
	void RenderBoxCut(GLRenderEngine& re, GLContext& rc, const BOX& box);

private:
	CGLModelScene* m_scene;
	GLPlaneCut	m_planeCut;
};

class GLObjectItem : public GLModelSceneItem
{
public:
	GLObjectItem(CGLModelScene* scene, GObject* po);
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void RenderGObject(GLRenderEngine& re, GLContext& rc);

	void RenderObject(GLRenderEngine& re, GLContext& rc);
	void RenderSurfaceMeshFaces(GLRenderEngine& re, GLContext& rc);

	void RenderParts(GLRenderEngine& re, GLContext& rc);
	void RenderSurfaces(GLRenderEngine& re, GLContext& rc);
	void RenderEdges(GLRenderEngine& re, GLContext& rc);
	void RenderNodes(GLRenderEngine& re, GLContext& rc);
	void RenderBeamParts(GLRenderEngine& re, GLContext& rc);
	void RenderSurfaceMeshEdges(GLRenderEngine& re, GLContext& rc);

	void RenderFEFacesFromGMesh(GLRenderEngine& re, GLContext& rc);
	void RenderMeshByDefault(GLRenderEngine& re, GLContext& rc);
	void RenderMeshByObjectColor(GLRenderEngine& re, GLContext& rc);
	void RenderMeshByElementType(GLRenderEngine& re, GLContext& rc, GLMesh& mesh);

	void RenderUnselectedBeamElements(GLRenderEngine& re, GLContext& rc);
	void RenderSelectedFEElements(GLRenderEngine& re, GLContext& rc);
	void RenderAllBeamElements(GLRenderEngine& re, GLContext& rc);
	void RenderSelectedFEFaces(GLRenderEngine& re, GLContext& rc);
	void RenderFEEdges(GLRenderEngine& re, GLContext& rc);
	void RenderFENodes(GLRenderEngine& re, GLContext& rc);
	void RenderSurfaceMeshNodes(GLRenderEngine& re, GLContext& rc);
	void RenderNormals(GLRenderEngine& re, GLContext& rc, double scale);

	void RenderSelection(GLRenderEngine& re, GLContext& rc);

private:
	void UpdateGMeshColor(GLMesh& msh);

	void ColorByDefault     (GLMesh& msh);
	void ColorByObject      (GLMesh& msh);
	void ColorByMaterialType(GLMesh& msh);
	void ColorByElementType (GLMesh& msh);
	void ColorByPhysics     (GLMesh& msh);

private:
	GObject* m_po;
	bool	m_clearCache;
};

class GLDiscreteItem : public GLModelSceneItem
{
	struct Line {
		vec3d a, b;
		GLColor col;
	};

public:
	GLDiscreteItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	std::vector<Line> m_lines;
};

class GLSelectionBox : public GLModelSceneItem
{
public:
	GLSelectionBox(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;
};

class GLMeshLinesItem : public GLModelSceneItem
{
public:
	GLMeshLinesItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;
};

class GLFeatureEdgesItem : public GLModelSceneItem
{
public:
	GLFeatureEdgesItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;
};

class GLPhysicsItem : public GLModelSceneItem
{
public:
	GLPhysicsItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void RenderRigidBodies(GLRenderEngine& re, GLContext& rc) const;
	void RenderRigidJoints(GLRenderEngine& re, GLContext& rc) const;
	void RenderRigidConnectors(GLRenderEngine& re, GLContext& rc) const;
	void RenderRigidWalls(GLRenderEngine& re, GLContext& rc) const;
	void RenderMaterialFibers(GLRenderEngine& re, GLContext& rc) const;
	void RenderLocalMaterialAxes(GLRenderEngine& re, GLContext& rc) const;
};

class GLSelectionItem : public GLModelSceneItem
{
public:
	GLSelectionItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void RenderSelectedParts(GLRenderEngine& re, GLContext& rc, GObject* po) const;
	void RenderSelectedSurfaces(GLRenderEngine& re, GLContext& rc, GObject* po) const;
	void RenderSelectedEdges(GLRenderEngine& re, GLContext& rc, GObject* po) const;
	void RenderSelectedNodes(GLRenderEngine& re, GLContext& rc, GObject* po) const;
};

class GLHighlighterItem : public GLModelSceneItem
{
public:
	GLHighlighterItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void drawNode(GLRenderEngine& re, GLContext& rc, GNode* node, GLColor c);
	void drawEdge(GLRenderEngine& re, GLContext& rc, GEdge* edge, GLColor c);
	void drawFace(GLRenderEngine& re, GLContext& rc, GFace* face, GLColor c);
	void drawPart(GLRenderEngine& re, GLContext& rc, GPart* part, GLColor c);

	void drawFENodeSet(GLRenderEngine& re, GLContext& rc, FSNodeSet* nodeSet, GLColor c);
	void drawFESurface(GLRenderEngine& re, GLContext& rc, FSSurface* surf, GLColor c);

private:
	GLColor			m_activeColor;		// color of active item
	GLColor			m_pickColor[2];		// color of picked items
};

class GLGridItem : public GLModelSceneItem
{
public:
	GLGridItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;
};

class GL3DImageItem : public GLModelSceneItem
{
public:
	GL3DImageItem(CGLModelScene* scene, CImageModel* img) : GLModelSceneItem(scene), m_img(img) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	CImageModel* m_img;
};

class CGLModelScene : public GLScene
{
public:
	CGLModelScene(CModelDocument* doc);

	void Render(GLRenderEngine& engine, GLContext& rc) override;

	BOX GetBoundingBox() override;

	BOX GetSelectionBox() override;

	void SetObjectColorMode(OBJECT_COLOR_MODE colorMode);

	OBJECT_COLOR_MODE ObjectColorMode() const;

	void Update() override;

	void UpdateFiberViz();

	GLMesh& GetSelectionMesh() { return m_selectionMesh; }

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
	void BuildScene(GLContext& rc);

public:
	void RenderRigidLabels(GLContext& rc);

	void RenderTags(GLContext& rc);

public:
	GLColor GetPartColor(GPart* pg);
	GLColor GetFaceColor(GFace& face);

	void BuildFiberViz(GLContext& rc);

	void UpdateRenderTransforms(GLContext& rc);

private:
	CModelDocument* m_doc;
	GLMesh			m_selectionMesh;
	
	OBJECT_COLOR_MODE	m_objectColor;

	bool m_buildScene;

	GLFiberRenderer* m_fiberViz;
};
