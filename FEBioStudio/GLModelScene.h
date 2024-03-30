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

class CModelDocument;
class GPart;

class CGLModelScene : public CGLScene
{
public:
	CGLModelScene(CModelDocument* doc);

	void Render(CGLContext& rc) override;

	GLMeshRender& GetMeshRenderer();

	BOX GetBoundingBox() override;

	BOX GetSelectionBox() override;

private:
	void RenderModel(CGLContext& rc);
	void RenderGObject(CGLContext& rc, GObject* po);
	void RenderSelectionBox(CGLContext& rc);
	void RenderRigidBodies(CGLContext& rc);
	void RenderRigidJoints(CGLContext& rc);
	void RenderRigidConnectors(CGLContext& rc);
	void RenderRigidWalls(CGLContext& rc);
	void RenderMaterialFibers(CGLContext& rc);
	void RenderLocalMaterialAxes(CGLContext& rc);
	void RenderDiscrete(CGLContext& rc);
	void RenderMeshLines(CGLContext& rc);
	void RenderFeatureEdges(CGLContext& rc);

private:
	// rendering functions for GObjects
	void RenderObject          (CGLContext& rc, GObject* po);
	void RenderParts           (CGLContext& rc, GObject* po);
	void RenderSurfaces        (CGLContext& rc, GObject* po);
	void RenderEdges           (CGLContext& rc, GObject* po);
	void RenderNodes           (CGLContext& rc, GObject* po);
	void RenderSelectedParts   (CGLContext& rc, GObject* po);
	void RenderSelectedSurfaces(CGLContext& rc, GObject* po);
	void RenderSelectedEdges   (CGLContext& rc, GObject* po);
	void RenderSelectedNodes   (CGLContext& rc, GObject* po);
	void RenderBeamParts       (CGLContext& rc, GObject* po);

	// rendering functions for FEMeshes
	void RenderFEElements   (CGLContext& rc, GObject* po);
	void RenderFEFaces      (CGLContext& rc, GObject* po);
	void RenderFEEdges      (CGLContext& rc, GObject* po);
	void RenderFENodes      (CGLContext& rc, GObject* po);
	void RenderMeshLines    (CGLContext& rc, GObject* pm);

	void RenderAllBeamElements       (CGLContext& rc, GObject* po);
	void RenderUnselectedBeamElements(CGLContext& rc, GObject* po);
	void RenderSelectedBeamElements  (CGLContext& rc, GObject* po);

	// rendering functions for surface meshes
	void RenderSurfaceMeshFaces(CGLContext& rc, GObject* po);
	void RenderSurfaceMeshEdges(CGLContext& rc, GObject* po);
	void RenderSurfaceMeshNodes(CGLContext& rc, GObject* po);

	void RenderNormals(CGLContext& rc, GObject* po, double scale);
	void RenderRigidLabels(CGLContext& rc);

private:
	// set the GL material properties based on the material
	void SetMatProps(GMaterial* pm);
	void SetMatProps(CGLContext& rc, GPart* pg);

	// set some default GL material properties
	void SetDefaultMatProps();

	void ActivateEnvironmentMap();
	void DeactivateEnvironmentMap();
	void LoadEnvironmentMap();

private:
	CModelDocument* m_doc;
	GLMeshRender	m_renderer;

	bool	m_bshowEnv;	// show environment map
	unsigned int	m_envtex;	// enironment texture ID
};
