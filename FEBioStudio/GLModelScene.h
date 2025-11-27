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
#include "GLViewScene.h"

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

	const GLCamera& GetCamera() const;

protected:
	CGLModelScene* m_scene;
};

class GLPlaneCutItem : public GLModelSceneItem
{
public:
	GLPlaneCutItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void UpdatePlaneCut(GLContext& rc, bool reset);
	void RenderBoxCut(GLRenderEngine& re, const BOX& box);

private:
	GLPlaneCut	m_planeCut;
};

class GLObjectSurfaceItem;

class GLObjectItem : public GLModelSceneItem
{
public:
	GLObjectItem(CGLModelScene* scene, GObject* po);
	~GLObjectItem();
	void render(GLRenderEngine& re, GLContext& rc) override;

	GObject* GetGObject() const { return m_po; }

	Transform GetTransform() const;
	void SetTransform(const Transform& T);

	GLMesh* GetSelectionMesh();

private:
	void RenderGObject(GLRenderEngine& re, GLContext& rc);

	void RenderSelection(GLRenderEngine& re);

private:
	GObject* m_po = nullptr;
	GLObjectSurfaceItem* m_surfItem = nullptr;
	Transform m_renderTransform;
};

class GLObjectSurfaceItem : public GLModelSceneItem
{
public:
	GLObjectSurfaceItem(CGLModelScene* scene, GObject* po) : GLModelSceneItem(scene), m_po(po) {}
	~GLObjectSurfaceItem() {}

	void BuildSurfaceMesh();
	void BuildSurfaceFEMesh(bool useContourMap);

	void render(GLRenderEngine& re, GLContext& rc) override;

	GLMesh* GetSelectionMesh();

private:
	void RenderGeomSurface(GLRenderEngine& re, GLContext& rc);
	void RenderFEMeshSurface(GLRenderEngine& re, GLContext& rc);

	void RenderSurfaceMeshFaces(GLRenderEngine& re, GLContext& rc);

	void RenderUnselectedBeamElements(GLRenderEngine& re);

	void RenderBeamParts(GLRenderEngine& re);
	void RenderNodes(GLRenderEngine& re);

	void RenderSurfaceMeshEdges(GLRenderEngine& re);
	void RenderAllBeamElements(GLRenderEngine& re);
	void RenderFEEdges(GLRenderEngine& re);
	void RenderFENodes(GLRenderEngine& re, GLContext& rc);
	void RenderSurfaceMeshNodes(GLRenderEngine& re, GLContext& rc);
	void renderTaggedGMeshNodes(GLRenderEngine& re, const GLMesh& mesh, int tag);

private:
	std::vector<GLMaterial> m_mat; // material per face
	GObject* m_po = nullptr;
	unsigned int m_uid = 0; // uid of GLMesh used to create the other meshes
	std::unique_ptr<GLMesh> m_surfMesh;
	std::unique_ptr<GLMesh> m_selectionMesh;
	std::unique_ptr<GLMesh> m_surfFEMesh;
	std::unique_ptr<GLMesh> m_nodeFEMesh;
};

class GLMeshLinesItem : public GLModelSceneItem
{
public:
	GLMeshLinesItem(CGLModelScene* scene, GObject* po) : GLModelSceneItem(scene), m_po(po) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	GObject* m_po = nullptr;
};

class GLFeatureEdgesItem : public GLModelSceneItem
{
public:
	GLFeatureEdgesItem(CGLModelScene* scene, GObject* po);
	~GLFeatureEdgesItem();
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void BuildRenderMesh();

	void ResetMesh(GLMesh* m = nullptr);
	bool NeedsUpdate() const;

private:
	GObject* m_po = nullptr;
	unsigned int m_uid = 0;
	std::unique_ptr<GLMesh> m_mesh;
};

class GLObjectNormalsItem : public GLModelSceneItem
{
public:
	GLObjectNormalsItem(CGLModelScene* scene, GObject* po) : GLModelSceneItem(scene), m_po(po) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	GObject* m_po = nullptr;
	std::unique_ptr<GLMesh> m_normalMesh;
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

class GLPhysicsItem : public GLModelSceneItem
{
public:
	GLPhysicsItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void RenderRigidBodies(GLRenderEngine& re, GLContext& rc) const;
	void RenderRigidJoints(GLRenderEngine& re, double scale) const;
	void RenderRigidConnectors(GLRenderEngine& re, double scale) const;
	void RenderRigidWalls(GLRenderEngine& re) const;
	void RenderLocalMaterialAxes(GLRenderEngine& re, GLContext& rc) const;
};

class GLFiberVizItem : public GLModelSceneItem
{
public:
	GLFiberVizItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void BuildFiberViz(GLContext& rc);

private:
	GLFiberRenderer* m_fiberViz = nullptr;
};

class GLSelectionItem : public GLModelSceneItem
{
public:
	GLSelectionItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void RenderSelectedParts   (GLRenderEngine& re, GLContext& rc, GLObjectItem* po) const;
	void RenderSelectedSurfaces(GLRenderEngine& re, GLContext& rc, GLObjectItem* po) const;
	void RenderSelectedEdges   (GLRenderEngine& re, GLContext& rc, GLObjectItem* po) const;
	void RenderSelectedNodes   (GLRenderEngine& re, GLContext& rc, GLObjectItem* po) const;
};

class GLHighlighterItem : public GLModelSceneItem
{
public:
	GLHighlighterItem(CGLModelScene* scene);
	void render(GLRenderEngine& re, GLContext& rc) override;

private:
	void drawNode(GLRenderEngine& re, GLContext& rc, GNode* node, GLColor c);
	void drawEdge(GLRenderEngine& re, GEdge* edge, GLColor c);
	void drawFace(GLRenderEngine& re, GLContext& rc, GFace* face, GLColor c);
	void drawPart(GLRenderEngine& re, GLContext& rc, GPart* part, GLColor c);

	void drawFENodeSet(GLRenderEngine& re, FSNodeSet* nodeSet, GLColor c);
	void drawFESurface(GLRenderEngine& re, FSSurface* surf, GLColor c);

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

class CGLModelScene : public GLViewScene
{
public:
	CGLModelScene(CModelDocument* doc);

	void Render(GLRenderEngine& engine, GLContext& rc) override;

	BOX GetBoundingBox() override;

	void SetObjectColorMode(OBJECT_COLOR_MODE colorMode);

	OBJECT_COLOR_MODE ObjectColorMode() const;

	void Update() override;

	GLMesh& GetSelectionMesh() { return m_selectionMesh; }

	void UpdateSelectionMesh(FESelection* sel);

	GModel* GetGModel();

	FSModel* GetFSModel();

	int GetSelectionMode() const;

	int GetItemMode() const;

	int GetMeshMode() const;

	int GetObjectColorMode() const;

	GObject* GetActiveObject() const;

	FESelection* GetCurrentSelection();

public:
	GLObjectItem* FindGLObjectItem(GObject* po);
	GLObjectItem* GetActiveGLObjectItem() const { return m_activeObjectItem; }
	std::vector<GLObjectItem*> GetGLObjectItems() const { return m_glObjectList; }

private:
	void BuildScene(GLContext& rc);

public:
	void RenderRigidLabels();

	void UpdateTags(GLContext& rc);

public:
	GLMaterial GetPartMaterial(GPart* pg);
	GLMaterial GetFaceMaterial(GFace& face);

	void UpdateRenderTransforms(GLContext& rc);

	LegendData GetLegendData(int n) override;

	void SetColorMap(int map) { m_colormap = map; Update(); }
	int GetColorMap() const { return m_colormap; }

	void ColorizeMesh(GObject* po);

private:
	CModelDocument* m_doc;
	GLMesh			m_selectionMesh;

	std::vector<GLObjectItem*> m_glObjectList;
	GLObjectItem* m_activeObjectItem = nullptr;

	int m_colormap = 6; // = JET
	bool m_showLegend = false;
	
	OBJECT_COLOR_MODE	m_objectColor;

	bool m_buildScene;
};
