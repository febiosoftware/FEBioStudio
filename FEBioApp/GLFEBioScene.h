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
#include <GLLib/GLScene.h>
#include <CUILib/GLSceneView.h>
#include <GLLib/ColorTexture.h>
#include <GLLib/GLMesh.h>
#include <QMutex>
#include "FEBioApp.h"

class FESurface;
class GLLegendBar;

class GLSceneObject : public GLSceneItem
{
public:
	GLSceneObject();
	bool LoadFromFile(const std::string& fileName);

	void SetColor(GLColor c);

	void SetPosition(vec3d v);

	void SetRotation(quatd q);

public:
	void render(GLRenderEngine& engine, GLContext& rc) override;

private:
	GLMesh		m_mesh;
	GLColor		m_col;
	vec3d		m_pos;
	quatd		m_rot;
};

class GLFEBioScene : public GLScene, public CFEBioModelDataSource
{
public:
	GLFEBioScene(FEBioModel& fem);

	~GLFEBioScene();

	void SetGLView(CGLSceneView* view) { m_view = view; }

	void Render(GLRenderEngine& engine, GLContext& rc) override;

	void RenderCanvas(QPainter& painter, GLContext& rc) override;

	// get the bounding box of the entire scene
	BOX GetBoundingBox();

	// get the bounding box of the current selection
	BOX GetSelectionBox() { return GetBoundingBox(); }

	void SetDataSourceName(const std::string& dataName);

	void SetColorMap(const std::string& colorMapName);

	void SetDataRange(double rngMin, double rngMax);

	void AddSceneObject(GLSceneObject* po);

public: // overrides from CFEBioModelDataSource

	void Clear() override;

	void Update(double time) override;

	void UpdateBoundingBox();

private:
	void BuildRenderMesh();

private:
	FEBioModel& m_fem;
	GLMesh* m_renderMesh;
	FESurface* m_febSurface;
	CGLSceneView* m_view;
	GLLegendBar* m_legend;
	QMutex	m_mutex;

	std::string	m_dataSource;
	bool m_useUserRange;
	double m_userRange[2];

	BOX	m_box;
	CColorTexture m_col;
};
