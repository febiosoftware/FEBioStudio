#pragma once
#include "rhiSceneView.h"

class rhiDocView : public rhiSceneView
{
public:
	rhiDocView(CMainWindow* wnd);

	void RenderScene(GLRenderEngine& re) override;

public:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	CGLWidgetManager m_Widget;
	GLTriad* triad;
};
