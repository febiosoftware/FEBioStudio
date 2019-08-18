#pragma once

class QOpenGLWidget;
#include "GLWidget.h"
#include <vector>

class CGLWidgetManager
{
public:
	enum Event {
		PUSH,
		DRAG,
		RELEASE
	};

public:
	~CGLWidgetManager();

	static CGLWidgetManager* GetInstance();

	void AddWidget(GLWidget* pw);
	void RemoveWidget(GLWidget* pw);
	int Widgets() { return (int)m_Widget.size(); }

	GLWidget* operator [] (int i) { return m_Widget[i]; }

	void AttachToView(QOpenGLWidget* pview);

	int handle(int x, int y, int nevent);

	void DrawWidgets(QPainter* painter);

	void SetActiveLayer(int l);

	// Make sure widget are within bounds. (Call when parent QOpenGLWidget changes size)
	void CheckWidgetBounds();

protected:
	void SnapWidget(GLWidget* pw);

protected:
	QOpenGLWidget*			m_pview;
	std::vector<GLWidget*>	m_Widget;
	unsigned int			m_layer;

private:
	CGLWidgetManager();
	CGLWidgetManager(const CGLWidgetManager& m);

	static CGLWidgetManager*	m_pmgr;
};
