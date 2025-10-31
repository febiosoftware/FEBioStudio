#include "rhiDocView.h"
#include <GLWLib/GLLabel.h>
#include <GLWLib/GLCheckBox.h>
#include <GLWLib/GLComposite.h>
#include <GLWLib/GLLegendBar.h>
#include <GLWLib/GLTriad.h>
#include <GLLib/GLContext.h>
#include <QMouseEvent>
#include <QPainter>

rhiDocView::rhiDocView(CMainWindow* wnd) : rhiSceneView(wnd) 
{
	GLLabel* label = new GLLabel(0, 50, 450, 100, "Welcome to RHI!");
	label->set_font_size(40);
	m_Widget.AddWidget(label);

	const int W = 200;
	const int H = 30;

	GLComposite* menu = new GLComposite(0, 0, W, H);
	menu->align(GLWAlign::GLW_ALIGN_RIGHT | GLWAlign::GLW_ALIGN_TOP);

	GLCheckBox* meshLines = new GLCheckBox(0, 0, W, H, "mesh lines");
	meshLines->add_event_handler([this](GLWidget* w, int nevent) {
		GLCheckBox* b = dynamic_cast<GLCheckBox*>(w);
		rhiScene* s = dynamic_cast<rhiScene*>(this->GetScene());
		if (s) s->renderMesh = b->m_checked;
		});
	menu->add_widget(meshLines);

	GLCheckBox* meshNodes = new GLCheckBox(0, 0, W, H, "mesh nodes");
	meshNodes->add_event_handler([this](GLWidget* w, int nevent) {
		GLCheckBox* b = dynamic_cast<GLCheckBox*>(w);
		rhiScene* s = dynamic_cast<rhiScene*>(this->GetScene());
		if (s) s->renderNodes = b->m_checked;
		});
	menu->add_widget(meshNodes);
	m_Widget.AddWidget(menu);

	GLLegendBar* legend = new GLLegendBar(0, 0, 100, 400);
	legend->align(GLWAlign::GLW_ALIGN_RIGHT | GLWAlign::GLW_ALIGN_VCENTER);
	m_Widget.AddWidget(legend);

	triad = new GLTriad(0, 0, 100, 100);
	triad->align(GLWAlign::GLW_ALIGN_BOTTOM | GLWAlign::GLW_ALIGN_LEFT);
	m_Widget.AddWidget(triad);
}

void rhiDocView::RenderScene(rhiRenderer& re)
{
	GLScene* scene = GetScene();
	if (scene == nullptr) return;

	// render the 3D scene first
	GLContext rc;
	rc.m_cam = &scene->GetCamera();
	scene->Render(re, rc);

	// position the triad
	GLCamera& cam = scene->GetCamera();
	triad->setOrientation(cam.GetOrientation());

	// render into overlay (if requested)
	bool renderOverlay = false;
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(scene);
	if (RhiScene) renderOverlay = RhiScene->renderOverlay;

	re.useOverlayImage(renderOverlay);
	if (renderOverlay)
	{
		QImage img(re.pixelSize(), QImage::Format_RGBA8888_Premultiplied);
		img.fill(Qt::transparent);
		QPainter painter(&img);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

		GLPainter glpainter(&painter, nullptr);

		m_Widget.DrawWidgets(&glpainter);
		painter.end();

		float H = (float)img.height();
		float h = (float)triad->h();
		QRhiViewport vp = { (float)triad->x(), H - (float)triad->y() - h, (float)triad->w(), h };
		quatd q = cam.GetOrientation();
		QMatrix4x4 Q; Q.rotate(QQuaternion(q.w, q.x, q.y, q.z));
		re.setTriadInfo(Q, vp);

		re.setOverlayImage(img);
	}
}

void rhiDocView::mousePressEvent(QMouseEvent* ev)
{
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(GetScene());
	bool useOverlay = false;
	if (RhiScene) useOverlay = RhiScene->renderOverlay;

	QPoint pt = ev->pos();

	// let the widget manager handle it first
	int x = pt.x();
	int y = pt.y();
	GLWidget* pw = GLWidget::get_focus();
	if (useOverlay && m_Widget.handle(x, y, GLWEvent::GLW_PUSH) == 1)
	{
		requestUpdate();
		return;
	}

	rhiSceneView::mousePressEvent(ev);
}

void rhiDocView::mouseMoveEvent(QMouseEvent* ev)
{
	GLScene* scene = GetScene();

	if (scene == nullptr) return;
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(scene);
	bool useOverlay = false;
	if (RhiScene) useOverlay = RhiScene->renderOverlay;

	bool but1 = (ev->buttons() & Qt::LeftButton);

	// get the mouse position
	int x = (int)ev->position().x();
	int y = (int)ev->position().y();

	// let the widget manager handle it first
	if (but1 && useOverlay && (m_Widget.handle(x, y, GLWEvent::GLW_DRAG) == 1))
	{
		requestUpdate();
		return;
	}

	rhiSceneView::mouseMoveEvent(ev);
}

void rhiDocView::mouseReleaseEvent(QMouseEvent* event)
{
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(GetScene());
	bool useOverlay = false;
	if (RhiScene) useOverlay = RhiScene->renderOverlay;

	int x = (int)event->position().x();
	int y = (int)event->position().y();

	// let the widget manager handle it first
	if (useOverlay && m_Widget.handle(x, y, GLWEvent::GLW_RELEASE) == 1)
	{
		requestUpdate();
		return;
	}

	rhiSceneView::mouseReleaseEvent(event);
}
