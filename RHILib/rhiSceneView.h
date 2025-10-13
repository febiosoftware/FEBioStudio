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
#include "rhiwindow.h"
#include "rhiDocument.h"

class CMainWindow;

class rhiSceneView : public RhiWindow
{
public:
	rhiSceneView(CMainWindow* wnd, QRhi::Implementation graphicsApi);
	~rhiSceneView();

	void customInit() override;
	void customRender() override;

public:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	CMainWindow* m_wnd = nullptr;
	rhiDocument* m_doc = nullptr;

	std::unique_ptr<QRhiBuffer> m_ubuf;
	std::unique_ptr<QRhiShaderResourceBindings> m_colorTriSrb;
	std::unique_ptr<QRhiGraphicsPipeline> m_colorPipeline;

	QRhiResourceUpdateBatch* m_initialUpdates = nullptr;

	float m_opacity = 1;
	int m_opacityDir = -1;

	GLCamera m_cam;
	QPoint m_prevPos;	//!< last mouse position

	std::vector<std::unique_ptr<rhi::Mesh>> m_meshList;
};

// helper function for creating a RhiWindow inside a QWidget
QWidget* createRHIWidget(CMainWindow* wnd, QRhi::Implementation api);
