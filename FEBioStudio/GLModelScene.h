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

class GLPlaneCutItem : public GLModelSceneItem
{
public:
	GLPlaneCutItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GLObjectItem : public GLModelSceneItem
{
public:
	GLObjectItem(CGLModelScene* scene, GObject* po) : GLModelSceneItem(scene), m_po(po) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;

private:
	void RenderGObject(GLRenderEngine& re, CGLContext& rc) const;

	void RenderObject(CGLContext& rc) const;
	void RenderObjectNew(GLRenderEngine& re, CGLContext& rc) const;
	void RenderSurfaceMeshFaces(CGLContext& rc) const;

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
	void render(GLRenderEngine& re, CGLContext& rc) const override;

private:
	std::vector<Line> m_lines;
};

class GLSelectionBox : public GLModelSceneItem
{
public:
	GLSelectionBox(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GLMeshLinesItem : public GLModelSceneItem
{
public:
	GLMeshLinesItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GLFeatureEdgesItem : public GLModelSceneItem
{
public:
	GLFeatureEdgesItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GLPhysicsItem : public GLModelSceneItem
{
public:
	GLPhysicsItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;

private:
	void RenderRigidBodies(CGLContext& rc) const;
	void RenderRigidJoints(CGLContext& rc) const;
	void RenderRigidConnectors(CGLContext& rc) const;
	void RenderRigidWalls(CGLContext& rc) const;
	void RenderMaterialFibers(CGLContext& rc) const;
	void RenderLocalMaterialAxes(CGLContext& rc) const;
};

// This is a mock item to disable clipping plane. 
// Probably want a different solution here
class GLDisableClipPlaneItem : public GLModelSceneItem
{
public:
	GLDisableClipPlaneItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GLSelectionItem : public GLModelSceneItem
{
public:
	GLSelectionItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GLGridItem : public GLModelSceneItem
{
public:
	GLGridItem(CGLModelScene* scene) : GLModelSceneItem(scene) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;
};

class GL3DImageItem : public GLModelSceneItem
{
public:
	GL3DImageItem(CGLModelScene* scene, CImageModel* img) : GLModelSceneItem(scene), m_img(img) {}
	void render(GLRenderEngine& re, CGLContext& rc) const override;

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

	GLRenderStats GetRenderStats();

	GModel* GetGModel();

	FSModel* GetFSModel();

	int GetSelectionMode() const;

	int GetItemMode() const;

	int GetMeshMode() const;

	int GetObjectColorMode() const;

	GObject* GetActiveObject() const;

	GLFiberRenderer* GetFiberRenderer();

public:
	void RenderSelection(CGLContext& rc);
	void RenderSelectedParts(CGLContext& rc, GObject* po);
	void RenderSelectedSurfaces(CGLContext& rc, GObject* po);
	void RenderSelectedEdges(CGLContext& rc, GObject* po);
	void RenderSelectedNodes(CGLContext& rc, GObject* po);

private:
	void BuildScene(CGLContext& rc);

public:
	// rendering functions for GObjects
	void RenderParts           (CGLContext& rc, GObject* po);
	void RenderSurfaces        (CGLContext& rc, GObject* po);
	void RenderEdges           (CGLContext& rc, GObject* po);
	void RenderNodes           (CGLContext& rc, GObject* po);
	void RenderBeamParts       (CGLContext& rc, GObject* po);

	// rendering functions for FEMeshes
	void RenderFEEdges      (CGLContext& rc, GObject* po);
	void RenderFENodes      (CGLContext& rc, GObject* po);

	void RenderSelectedFEFaces(CGLContext& rc, GObject* po);
	void RenderSelectedFEElements(CGLContext& rc, GObject* po);

	void RenderFEFacesFromGMesh(CGLContext& rc, GObject* po);

	void RenderAllBeamElements       (CGLContext& rc, GObject* po);
	void RenderUnselectedBeamElements(CGLContext& rc, GObject* po);
	void RenderSelectedBeamElements  (CGLContext& rc, GObject* po);

	// rendering functions for surface meshes
	void RenderSurfaceMeshEdges(CGLContext& rc, GObject* po);
	void RenderSurfaceMeshNodes(CGLContext& rc, GObject* po);

	void RenderNormals(CGLContext& rc, GObject* po, double scale);
	void RenderRigidLabels(CGLContext& rc);

	void RenderTags(CGLContext& rc);

private:
	void RenderMeshByDefault(CGLContext& rc, GObject& o, GMesh& mesh);
	void RenderMeshByObjectColor(CGLContext& rc, GObject& o, GMesh& mesh);
	void RenderMeshByMaterialType(CGLContext& rc, GObject& o, GMesh& mesh);
	void RenderMeshByPhysics(CGLContext& rc, GObject& o, GMesh& mesh);
	void RenderMeshByElementType(CGLContext& rc, GObject& o, GMesh& mesh);

public:
	// set the GL material properties based on the material
	void SetMatProps(GMaterial* pm);
	void SetMatProps(CGLContext& rc, GPart* pg);

	GLColor GetPartColor(CGLContext& rc, GPart* pg);

	// set some default GL material properties
	void SetDefaultMatProps();

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
