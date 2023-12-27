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
#include "../FEBioStudio/GLScene.h"
#include "../FEBioStudio/GLSceneView.h"
#include "../PostLib/ColorMap.h"
#include <GLLib/GLMesh.h>
#include "FEBioAppDocument.h"
#include <QMutex>

class GMesh;
class FESurface;

class GLFEBioScene : public CGLScene, public CFEBioModelDataSource
{
public:
	GLFEBioScene(FEBioModel& fem);

	~GLFEBioScene();

	void SetGLView(CGLSceneView* view) { m_view = view; }

	void Render(CGLContext& rc);

	// get the bounding box of the entire scene
	BOX GetBoundingBox();

	// get the bounding box of the current selection
	BOX GetSelectionBox() { return GetBoundingBox(); }

	void SetDataSourceName(const std::string& dataName) { m_dataSource = dataName; }

	void SetColorMap(const std::string& colorMapName);

	void SetDataRange(double rngMin, double rngMax);

public: // overrides from CFEBioModelDataSource

	void Clear() override;

	void Update(double time) override;

	void UpdateBoundingBox();

private:
	void BuildRenderMesh();

private:
	FEBioModel& m_fem;
	GMesh* m_renderMesh;
	GLTriMesh m_glmesh;
	FESurface* m_febSurface;
	CGLSceneView* m_view;
	QMutex	m_mutex;

	std::string	m_dataSource;
	bool m_useUserRange;
	double m_userRange[2];

	BOX	m_box;
	Post::CColorTexture m_col;
};
