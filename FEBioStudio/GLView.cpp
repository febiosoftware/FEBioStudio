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

#include <GL/glew.h>
#include "GLView.h"
#ifdef __APPLE__
#include <OpenGL/GLU.h>
#else
#include <GL/glu.h>
#endif
#include "MainWindow.h"
#include "BuildPanel.h"
#include "CreatePanel.h"
#include "ModelDocument.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include "GLHighlighter.h"
#include "GLCursor.h"
#include <math.h>
#include <MeshLib/MeshTools.h>
#include "GLViewTransform.h"
#include <GLLib/glx.h>
#include <GLLib/GDecoration.h>
#include <PostLib/ColorMap.h>
#include <GLLib/GLCamera.h>
#include <GLLib/GLContext.h>
#include <QMenu>
#include <QMessageBox>
#include <ImageLib/ImageModel.h>
#include "PostDocument.h"
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLModel.h>
#include <GeomLib/GModel.h>
#include "Commands.h"
#include "PostObject.h"
#include "ImageSliceView.h"
#include <MeshTools/FEExtrudeFaces.h>
#include <chrono>
using namespace std::chrono;

static GLubyte poly_mask[128] = {
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170,
	85, 85, 85, 85,
	170, 170, 170, 170
};

bool intersectsRect(const QPoint& p0, const QPoint& p1, const QRect& rt)
{
	// see if either point lies inside the rectangle
	if (rt.contains(p0)) return true;
	if (rt.contains(p1)) return true;

	// get the point coordinates
	int ax = p0.x();
	int ay = p0.y();
	int bx = p1.x();
	int by = p1.y();

	// get the rect coordinates
	int x0 = rt.x();
	int y0 = rt.y();
	int x1 = x0 + rt.width();
	int y1 = y0 + rt.height();
	if (y0 == y1) return false;
	if (x0 == x1) return false;

	// check horizontal lines
	if (ay == by)
	{
		if ((ay > y0) && (ay < y1))
		{
			if ((ax < x0) && (bx > x1)) return true;
			if ((bx < x0) && (ax > x1)) return true;
			return false;
		}
		else return false;
	}

	// check vertical lines
	if (ax == bx)
	{
		if ((ax > x0) && (ax < x1))
		{
			if ((ay < y0) && (by > y1)) return true;
			if ((by < y0) && (ay > y1)) return true;
			return false;
		}
		else return false;
	}

	// for the general case, we see if any of the four edges of the rectangle are crossed
	// top edge
	int x = ax + ((y0 - ay) * (bx - ax)) / (by - ay);
	if ((x > x0) && (x < x1))
	{
		if ((ay < y0) && (by > y0)) return true;
		if ((by < y0) && (ay > y0)) return true;
		return false;
	}

	// bottom edge
	x = ax + ((y1 - ay) * (bx - ax)) / (by - ay);
	if ((x > x0) && (x < x1))
	{
		if ((ay < y1) && (by > y1)) return true;
		if ((by < y1) && (ay > y1)) return true;
		return false;
	}

	// left edge
	int y = ay + ((x0 - ax) * (by - ay)) / (bx - ax);
	if ((y > y0) && (y < y1))
	{
		if ((ax < x0) && (bx > x0)) return true;
		if ((bx < x0) && (ax > x0)) return true;
		return false;
	}

	// right edge
	y = ay + ((x1 - ax) * (by - ay)) / (bx - ax);
	if ((y > y0) && (y < y1))
	{
		if ((ax < x1) && (bx > x1)) return true;
		if ((bx < x1) && (ax > x1)) return true;
		return false;
	}

	return false;
}

//=============================================================================
bool SelectRegion::LineIntersects(int x0, int y0, int x1, int y1) const
{
	return (IsInside(x0, y0) || IsInside(x1, y1));
}

bool SelectRegion::TriangleIntersect(int x0, int y0, int x1, int y1, int x2, int y2) const
{
	return (LineIntersects(x0, y0, x1, y1) || LineIntersects(x1, y1, x2, y2) || LineIntersects(x2, y2, x0, y0));
}

//=============================================================================
BoxRegion::BoxRegion(int x0, int x1, int y0, int y1)
{
	m_x0 = (x0<x1 ? x0 : x1); m_x1 = (x0<x1 ? x1 : x0);
	m_y0 = (y0<y1 ? y0 : y1); m_y1 = (y0<y1 ? y1 : y0);
}

bool BoxRegion::IsInside(int x, int y) const
{
	return ((x >= m_x0) && (x <= m_x1) && (y >= m_y0) && (y <= m_y1));
}

bool BoxRegion::LineIntersects(int x0, int y0, int x1, int y1) const
{
	return intersectsRect(QPoint(x0, y0), QPoint(x1, y1), QRect(m_x0, m_y0, m_x1 - m_x0, m_y1 - m_y0));
}

CircleRegion::CircleRegion(int x0, int x1, int y0, int y1)
{
	m_xc = x0;
	m_yc = y0;

	double dx = (x1 - x0);
	double dy = (y1 - y0);
	m_R = (int)sqrt(dx*dx + dy*dy);
}

bool CircleRegion::IsInside(int x, int y) const
{
	double rx = x - m_xc;
	double ry = y - m_yc;
	int r = rx*rx + ry*ry;
	return (r <= m_R*m_R);
}

bool CircleRegion::LineIntersects(int x0, int y0, int x1, int y1) const
{
	if (IsInside(x0, y0) || IsInside(x1, y1)) return true;

	int tx = x1 - x0;
	int ty = y1 - y0;

	int D = tx*(m_xc - x0) + ty*(m_yc - y0);
	int N = tx*tx + ty*ty;
	if (N == 0) return false;

	if ((D >= 0) && (D <= N))
	{
		int px = x0 + D*tx / N - m_xc;
		int py = y0 + D*ty / N - m_yc;

		if (px*px + py*py <= m_R*m_R) return true;
	}
	else return false;

	return false;
}

FreeRegion::FreeRegion(vector<pair<int, int> >& pl) : m_pl(pl)
{
	if (m_pl.empty() == false)
	{
		vector<pair<int, int> >::iterator pi = m_pl.begin();
		m_x0 = m_x1 = pi->first;
		m_y0 = m_y1 = pi->second;
		for (pi = m_pl.begin(); pi != m_pl.end(); ++pi)
		{
			int x = pi->first;
			int y = pi->second;
			if (x < m_x0) m_x0 = x; if (x > m_x1) m_x1 = x;
			if (y < m_y0) m_y0 = y; if (y > m_y1) m_y1 = y;
		}
	}
}

bool FreeRegion::IsInside(int x, int y) const
{
	if (m_pl.empty()) return false;
	if ((x < m_x0) || (x > m_x1) || (y < m_y0) || (y > m_y1))
	{
		return false;
	}

	int nint = 0;
	int N = (int)m_pl.size();
	for (int i = 0; i<N; ++i)
	{
		int ip1 = (i + 1) % N;
		double x0 = (double)m_pl[i].first;
		double y0 = (double)m_pl[i].second;
		double x1 = (double)m_pl[ip1].first;
		double y1 = (double)m_pl[ip1].second;

		double yc = (double)y + 0.0001;

		if (((y1>yc) && (y0<yc)) || ((y0>yc) && (y1<yc)))
		{
			double xi = x1 + ((x0 - x1)*(y1 - yc)) / (y1 - y0);
			if (xi >(double)x) nint++;
		}
	}
	return ((nint>0) && (nint % 2));
}

CGLPivot::CGLPivot(CGLView* view) : m_Ttor(view), m_Rtor(view), m_Stor(view)
{
	m_mode = PIVOT_SELECTION_MODE::SELECT_NONE;
	m_pos = vec3d(0, 0, 0);
}

void CGLPivot::Render(int ntrans, double scale, bool bact)
{
	switch (ntrans)
	{
	case TRANSFORM_MOVE  : m_Ttor.SetScale(scale); m_Ttor.Render(m_mode, bact); break;
	case TRANSFORM_ROTATE: m_Rtor.SetScale(scale); m_Rtor.Render(m_mode, bact); break;
	case TRANSFORM_SCALE : m_Stor.SetScale(scale); m_Stor.Render(m_mode, bact); break;
	}
}

int CGLPivot::Pick(int ntrans, int x, int y)
{
	switch (ntrans)
	{
	case TRANSFORM_MOVE  : m_mode = m_Ttor.Pick(x, y); break;
	case TRANSFORM_ROTATE: m_mode = m_Rtor.Pick(x, y); break;
	case TRANSFORM_SCALE : m_mode = m_Stor.Pick(x, y); break;
	}
	return m_mode;
}

//-----------------------------------------------------------------------------
CGLView::CGLView(CMainWindow* pwnd, QWidget* parent) : CGLSceneView(parent), m_pWnd(pwnd), m_pivot(this), m_select(this), m_planeCut(this)
{
	m_bsnap = false;

	m_btrack = false;

	m_showFPS = false;

	Reset();

	m_wa = 0;
	m_wt = 0;

	m_bsel = false;

	m_nview = VIEW_USER;

	m_nsnap = SNAP_NONE;

	m_bshift = false;
	m_bctrl = false;

	m_bpick = false;

	m_userPivot = false;

	m_coord = COORD_GLOBAL;

	m_showPlaneCut = false;
	m_planeCutMode = Planecut_Mode::PLANECUT;

	// attach the highlighter to this view
	GLHighlighter::AttachToView(this);

	// attach the 3D cursor to this view
	GLCursor::AttachToView(this);

	m_recorder.AttachToView(this);

	m_showContextMenu = true;

	m_ballocDefaultWidgets = true;
	m_Widget = nullptr;

	m_ptitle = nullptr;
	m_psubtitle = nullptr;
	m_ptriad = nullptr;
	m_pframe = nullptr;
	m_legend = nullptr;
}

CGLView::~CGLView()
{
}

void CGLView::ShowContextMenu(bool b)
{
	m_showContextMenu = b;
}

void CGLView::AllocateDefaultWidgets(bool b)
{
	m_ballocDefaultWidgets = b;
}

std::string CGLView::GetOGLVersionString()
{
	return m_oglVersionString;
}

CGLDocument* CGLView::GetDocument()
{
	return m_pWnd->GetGLDocument();
}

void CGLView::UpdateCamera(bool hitCameraTarget)
{
	CGLScene* scene = GetActiveScene();
	if (scene)
	{
		CGLCamera& cam = scene->GetCamera();
		cam.Update(hitCameraTarget);
	}
}

void CGLView::resizeGL(int w, int h)
{
	QOpenGLWidget::resizeGL(w, h);
	if (m_Widget) m_Widget->CheckWidgetBounds();
}

void CGLView::changeViewMode(View_Mode vm)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	SetViewMode(vm);

	// switch to ortho view if we're not in it
	bool bortho = scene->GetView().OrhographicProjection();
	if (bortho == false)
	{
		m_pWnd->toggleOrtho();
	}
}

void CGLView::SetColorMap(unsigned int n)
{
	m_colorMap.SetColorMap(n);
}

Post::CColorMap& CGLView::GetColorMap()
{
	return m_colorMap.ColorMap();
}

void CGLView::mousePressEvent(QMouseEvent* ev)
{
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	int ntrans = pdoc->GetTransformMode();

	int x = (int)ev->position().x();
	int y = (int)ev->position().y();

	// get the active view
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	// let the widget manager handle it first
	GLWidget* pw = GLWidget::get_focus();
	if (m_Widget && (m_Widget->handle(x, y, CGLWidgetManager::PUSH) == 1))
	{
		m_pWnd->UpdateFontToolbar();
		repaint();
		return;
	}

	if (pw && (GLWidget::get_focus() == 0))
	{
		// If we get here, the current widget selection was cleared
		m_pWnd->UpdateFontToolbar();
		repaint();
	}

	// store the current point
	m_x0 = m_x1 = ev->pos().x();
	m_y0 = m_y1 = ev->pos().y();
	m_pl.clear();
	m_pl.push_back(pair<int, int>(m_x0, m_y0));

	m_bshift = (ev->modifiers() & Qt::ShiftModifier   ? true : false);
	m_bctrl  = (ev->modifiers() & Qt::ControlModifier ? true : false);

	m_select.SetStateModifiers(m_bshift, m_bctrl);

	Qt::MouseButton but = ev->button();

	m_bextrude = false;

	int pivotMode = m_pivot.GetSelectionMode();

	if (but == Qt::LeftButton)
	{
		GLViewSettings& vs = GetViewSettings();
		if (vs.m_bselbrush && (m_bshift || m_bctrl))
		{
			m_select.BrushSelectFaces(m_x0, m_y0, (m_bctrl == false), true);
			ev->accept();
			repaint();
			return;
		}
		if ((m_bshift || m_bctrl) && (pivotMode == PIVOT_SELECTION_MODE::SELECT_NONE)) m_bsel = true;
		if ((pivotMode != PIVOT_SELECTION_MODE::SELECT_NONE) && m_bshift && (ntrans == TRANSFORM_MOVE))
		{
			GMeshObject* po = dynamic_cast<GMeshObject*>(pdoc->GetActiveObject());
			int nmode = pdoc->GetItemMode();
			if (po && (nmode == ITEM_FACE))
			{
				m_bextrude = true;
			}
		}
	}
	else m_bsel = false;

	if (GLHighlighter::IsTracking())
	{
		ev->accept();
		return;
	}

	CGLCamera& cam = pdoc->GetView()->GetCamera();
	cam.SetMoving(true);

	if (ntrans == TRANSFORM_MOVE)
	{
		m_rt = m_rg = vec3d(0, 0, 0);
	}
	else if (ntrans == TRANSFORM_ROTATE)
	{
		m_wt = 0; m_wa = 0;
	}
	else if (ntrans == TRANSFORM_SCALE)
	{
		// determine the direction of scale
		if (!m_bshift)
		{
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_X) m_ds = vec3d(1, 0, 0);
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Y) m_ds = vec3d(0, 1, 0);
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Z) m_ds = vec3d(0, 0, 1);
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_XY) m_ds = vec3d(1, 1, 0);
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_YZ) m_ds = vec3d(0, 1, 1);
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_XZ) m_ds = vec3d(1, 0, 1);
		}
		else m_ds = vec3d(1, 1, 1);

		CModelDocument* mdoc = dynamic_cast<CModelDocument*>(GetDocument());
		if (mdoc)
		{
			FESelection* ps = mdoc->GetCurrentSelection();
			if (ps && ((m_coord == COORD_LOCAL)|| postDoc))
			{
				quatd q = ps->GetOrientation();
				q.RotateVector(m_ds);
			}
		}

		m_ds.Normalize();
		m_st = m_sa = 1;
	}

	ev->accept();
}

void CGLView::mouseMoveEvent(QMouseEvent* ev)
{
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	CModelDocument* mdoc = dynamic_cast<CModelDocument*>(pdoc);

	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	bool bshift = (ev->modifiers() & Qt::ShiftModifier   ? true : false);
	bool bctrl  = (ev->modifiers() & Qt::ControlModifier ? true : false);
	bool balt   = (ev->modifiers() & Qt::AltModifier     ? true : false);

	int ntrans = pdoc->GetTransformMode();

	bool but1 = (ev->buttons() & Qt::LeftButton);
	bool but2 = (ev->buttons() & Qt::MiddleButton);
	bool but3 = (ev->buttons() & Qt::RightButton);

	m_select.SetStateModifiers(bshift, bctrl);

	// get the mouse position
	int x = ev->pos().x();
	int y = ev->pos().y();

	// get the active view
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	// let the widget manager handle it first
	if (but1 && (m_Widget && (m_Widget->handle(x, y, CGLWidgetManager::DRAG) == 1)))
	{
		repaint();
		m_pWnd->UpdateFontToolbar();
		return;
	}

	// if no buttons are pressed, then we update the pivot only
	if ((but1 == false) && (but2 == false) && (but3 == false))
	{
		int ntrans = pdoc->GetTransformMode();
		if (ntrans != TRANSFORM_NONE)
		{
			if (SelectPivot(x, y)) 
			repaint();
		}
		else
		{
			if (pdoc->GetSelectionMode() == SELECT_EDGE)
			{
				HighlightEdge(x, y);
			}
			else if (pdoc->GetSelectionMode() == SELECT_NODE)
			{
				HighlightNode(x, y);
			}
		}
		ev->accept();

		// we need to repaint if brush selection is on so the brush can be redrawn
		if (GetViewSettings().m_bselbrush)
		{
			m_x1 = x;
			m_y1 = y;
			repaint();
		}

		return;
	}

	AddRegionPoint(x, y);

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	int pivotMode = m_pivot.GetSelectionMode();
	if (pivotMode == PIVOT_SELECTION_MODE::SELECT_NONE)
	{
		if (but1 && !m_bsel)
		{
			if (GetViewSettings().m_bselbrush && (bshift || bctrl))
			{
				m_select.BrushSelectFaces(x, y, (bctrl == false), false);
				repaint();
			}
			else if (m_nview == VIEW_USER)
			{
				if (balt)
				{
					quatd qz = quatd((y - m_y1)*0.01f, vec3d(0, 0, 1));
					cam.Orbit(qz);
				}
				else
				{
					quatd qx = quatd((y - m_y1)*0.01f, vec3d(1, 0, 0));
					quatd qy = quatd((x - m_x1)*0.01f, vec3d(0, 1, 0));

					cam.Orbit(qx);
					cam.Orbit(qy);
				}

				repaint();
			}
			else SetViewMode(VIEW_USER);
		}
		else if ((but2 || (but3 && balt)) && !m_bsel)
		{
			vec3d r = vec3d(-(double)(x - m_x1), (double)(y - m_y1), 0.f);
			cam.PanView(r);
			repaint();
		}
		else if (but3 && !m_bsel)
		{
			if (bshift)
			{
				double D = (double) m_y1 - y;
				double s = cam.GetFinalTargetDistance()*1e-2;
				if (D < 0) s = -s;
				cam.Dolly(s);
			}
			else if (bctrl)
			{
				quatd qx = quatd((m_y1 - y)*0.001f, vec3d(1, 0, 0));
				quatd qy = quatd((m_x1 - x)*0.001f, vec3d(0, 1, 0));
				quatd q = qy*qx;

				cam.Pan(q);
			}
			else
			{
				if (m_y1 > y) cam.Zoom(0.95f);
				if (m_y1 < y) cam.Zoom(1.0f / 0.95f);
			}

			repaint();

			m_pWnd->UpdateGLControlBar();
		}
		// NOTE: Not sure why we would want to do an expensive update when we move the mouse.
		//       I think we only need to do a repaint
//		if (but1 && m_bsel) m_pWnd->Update();
		repaint();
	}
	else if (ntrans == TRANSFORM_MOVE)
	{
		if (but1)
		{
			if (m_bextrude)
			{
				GMeshObject* po = dynamic_cast<GMeshObject*>(pdoc->GetActiveObject());
				if (po)
				{
					FEExtrudeFaces mod;
					mod.SetExtrusionDistance(0.0);
					mdoc->ApplyFEModifier(mod, po, 0, false);
				}

				m_bextrude = false;
			}

			double f = 0.0012f*(double) cam.GetFinalTargetDistance();

			vec3d dr = vec3d(f*(x - m_x1), f*(m_y1 - y), 0);

			quatd q = cam.GetOrientation();

			q.Inverse().RotateVector(dr);
			FESelection* ps = nullptr;
			if (mdoc) ps = mdoc->GetCurrentSelection();
			else if (postDoc) ps = postDoc->GetCurrentSelection();
			if (ps)
			{
				if ((m_coord == COORD_LOCAL) || postDoc) ps->GetOrientation().Inverse().RotateVector(dr);

				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_X) dr.y = dr.z = 0;
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Y) dr.x = dr.z = 0;
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Z) dr.x = dr.y = 0;
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_XY) dr.z = 0;
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_YZ) dr.x = 0;
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_XZ) dr.y = 0;

				if ((m_coord == COORD_LOCAL) || postDoc) dr = ps->GetOrientation() * dr;

				m_rg += dr;
				if (bctrl)
				{
					double g = scene->GetGridScale();
					vec3d rt;
					rt.x = g * ((int)(m_rg.x / g));
					rt.y = g * ((int)(m_rg.y / g));
					rt.z = g * ((int)(m_rg.z / g));
					dr = rt - m_rt;
				}

				m_rt += dr;
				ps->Translate(dr);

				m_pWnd->OnSelectionTransformed();
			}
		}
	}
	else if (ntrans == TRANSFORM_ROTATE)
	{
		if (but1)
		{
			quatd q;

			double f = 0.002*((m_y1 - y) + (x - m_x1));
			if (fabs(f) < 1e-7) f = 0;

			m_wa += f;

			if (bctrl)
			{
				double da = 5 * DEG2RAD;
				int n = (int)(m_wa / da);
				f = n*da - m_wt;
			}
			if (fabs(f) < 1e-7) f = 0;

			m_wt += f;

			if (f != 0)
			{
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_X) q = quatd(f, vec3d(1, 0, 0));
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Y) q = quatd(f, vec3d(0, 1, 0));
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Z) q = quatd(f, vec3d(0, 0, 1));

				FESelection* ps = nullptr;
				if (mdoc) ps = mdoc->GetCurrentSelection();
				else if (postDoc) ps = postDoc->GetCurrentSelection();
				if (ps)
				{
					if ((m_coord == COORD_LOCAL) || postDoc)
					{
						quatd qs = ps->GetOrientation();
						q = qs * q * qs.Inverse();
					}

					q.MakeUnit();
					ps->Rotate(q, GetPivotPosition());
				}
			}

			m_pWnd->UpdateGLControlBar();
			repaint();
		}
	}
	else if (ntrans == TRANSFORM_SCALE)
	{
		if (but1)
		{
			double df = 1 + 0.002*((m_y1 - y) + (x - m_x1));

			m_sa *= df;
			if (bctrl)
			{
				double g = scene->GetGridScale();
				double st;
				st = g*((int)((m_sa - 1) / g)) + 1;

				df = st / m_st;
			}
			m_st *= df;
			FESelection* ps = mdoc->GetCurrentSelection();
			ps->Scale(df, m_ds, GetPivotPosition());

			m_pWnd->UpdateGLControlBar();
			repaint();
		}
	}

	m_x1 = x;
	m_y1 = y;

	cam.Update(true);

	m_pWnd->OnCameraChanged();

	ev->accept();
}

void CGLView::mouseDoubleClickEvent(QMouseEvent* ev)
{
	if (ev->button() == Qt::LeftButton)
	{
		m_pWnd->on_actionProperties_triggered();
	}
}

void CGLView::mouseReleaseEvent(QMouseEvent* ev)
{
	// get the active view
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	int x = (int)ev->position().x();
	int y = (int)ev->position().y();

	// let the widget manager handle it first
	if (m_Widget && (m_Widget->handle(x, y, CGLWidgetManager::RELEASE) == 1))
	{
		ev->accept();
		m_pWnd->UpdateFontToolbar();
		repaint();
		return;
	}
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = GetViewSettings();
	if (view.m_bselbrush)
	{
		m_select.Finish();
		ev->accept();
		return;
	}

	int ntrans = pdoc->GetTransformMode();
	int item = pdoc->GetItemMode();
	int nsel = pdoc->GetSelectionMode();
	Qt::MouseButton but = ev->button();

	if (GLHighlighter::IsTracking())
	{
		GLHighlighter::PickActiveItem();
		ev->accept();
		return;
	}

	// which mesh is active (surface or volume)
	int meshMode = m_pWnd->GetMeshMode();
	if (postDoc) meshMode = MESH_MODE_VOLUME;

	m_bextrude = false;

	AddRegionPoint(x, y);

	CGLCamera& cam = pdoc->GetView()->GetCamera();
	cam.SetMoving(false);

	int pivotMode = m_pivot.GetSelectionMode();
	if (pivotMode == PIVOT_SELECTION_MODE::SELECT_NONE)
	{
		if (but == Qt::LeftButton)
		{
			// if we are in selection mode, we need to see if 
			// there is an object under the cursor
			if (((m_x0==m_x1) && (m_y0==m_y1)) || m_bsel)
			{
				if ((m_x0 == m_x1) && (m_y0 == m_y1))
				{
					if (item == ITEM_MESH) 
					{
						switch (nsel)
						{
						case SELECT_OBJECT  : m_select.SelectObjects (m_x0, m_y0); break;
						case SELECT_PART    : m_select.SelectParts   (m_x0, m_y0); break;
						case SELECT_FACE    : m_select.SelectSurfaces(m_x0, m_y0); break;
						case SELECT_EDGE    : m_select.SelectEdges   (m_x0, m_y0); break;
						case SELECT_NODE    : m_select.SelectNodes   (m_x0, m_y0); break;
						case SELECT_DISCRETE: m_select.SelectDiscrete(m_x0, m_y0); break;
						default:
							ev->accept();
							return ;
						};
					}
					else
					{
						if (meshMode == MESH_MODE_VOLUME)
						{
							if      (item == ITEM_ELEM) m_select.SelectFEElements(m_x0, m_y0);
							else if (item == ITEM_FACE) m_select.SelectFEFaces(m_x0, m_y0);
							else if (item == ITEM_EDGE) m_select.SelectFEEdges(m_x0, m_y0);
							else if (item == ITEM_NODE) m_select.SelectFENodes(m_x0, m_y0);
						}
						else
						{
							if      (item == ITEM_FACE) m_select.SelectSurfaceFaces(m_x0, m_y0);
							else if (item == ITEM_EDGE) m_select.SelectSurfaceEdges(m_x0, m_y0);
							else if (item == ITEM_NODE) m_select.SelectSurfaceNodes(m_x0, m_y0);
						}
					}

					bool bok = false;
					vec3d r = PickPoint(m_x0, m_y0, &bok);
					if (bok)
					{
						m_bpick = true;
						Set3DCursor(r);

						emit pointPicked(r);
					}
					else m_bpick = false;
				}
				else
				{
					// allocate selection region
					int nregion = pdoc->GetSelectionStyle();
					SelectRegion* preg = 0;
					switch (nregion)
					{
					case REGION_SELECT_BOX: preg = new BoxRegion   (m_x0, m_x1, m_y0, m_y1); break;
					case REGION_SELECT_CIRCLE: preg = new CircleRegion(m_x0, m_x1, m_y0, m_y1); break;
					case REGION_SELECT_FREE: preg = new FreeRegion  (m_pl); break;
					default:
						assert(false);
					}

					if (item == ITEM_MESH)
					{
						switch (nsel)
						{
						case SELECT_OBJECT  : m_select.RegionSelectObjects (*preg); break;
						case SELECT_PART    : m_select.RegionSelectParts   (*preg); break;
						case SELECT_FACE    : m_select.RegionSelectSurfaces(*preg); break;
						case SELECT_EDGE    : m_select.RegionSelectEdges   (*preg); break;
						case SELECT_NODE    : m_select.RegionSelectNodes   (*preg); break;
						case SELECT_DISCRETE: m_select.RegionSelectDiscrete(*preg); break;
						default:
							ev->accept();
							return;
						};
					}
					else if (item == ITEM_ELEM) m_select.RegionSelectFEElems(*preg);
					else if (item == ITEM_FACE) m_select.RegionSelectFEFaces(*preg);
					else if (item == ITEM_EDGE) m_select.RegionSelectFEEdges(*preg);
					else if (item == ITEM_NODE) m_select.RegionSelectFENodes(*preg);

					delete preg;
				}

				CModelDocument* mdoc = dynamic_cast<CModelDocument*>(GetDocument());
				if (mdoc)
				{
					FESelection* psel = mdoc->GetCurrentSelection();
					if (psel)
					{
						if (psel->Size() && view.m_bhide)
						{
							pdoc->DoCommand(new CCmdHideSelection(mdoc));
						}
					}
					emit selectionChanged();
				}
				m_pWnd->Update(0, false);

				repaint();
			}
			else
			{
				CCmdChangeView* pcmd = new CCmdChangeView(pdoc->GetView(), cam);
				cam = m_oldCam;
				m_Cmd.DoCommand(pcmd);
				repaint();
			}
		}
		else if (but == Qt::MiddleButton)
		{
			if ((m_x0 == m_x1) && (m_y0 == m_y1))
			{
				if (view.m_apply)
				{
					CBuildPanel* build = m_pWnd->GetBuildPanel();
					if (build) build->Apply();
				}
			}
			else
			{
				CCmdChangeView* pcmd = new CCmdChangeView(pdoc->GetView(), cam);
				cam = m_oldCam;
				m_Cmd.DoCommand(pcmd);
				repaint();
			}
		}
		else if (but == Qt::RightButton)
		{
			if ((m_x0 == m_x1) && (m_y0 == m_y1))
			{
				if (m_showContextMenu)
				{
					QMenu menu(this);
					m_pWnd->BuildContextMenu(menu);
					menu.exec(ev->globalPos());
				}
			}
			else
			{
				CCmdChangeView* pcmd = new CCmdChangeView(pdoc->GetView(), cam);
				cam = m_oldCam;
				m_Cmd.DoCommand(pcmd);
				repaint();
			}
		}
		m_bsel = false;
	}
	else 
	{
		CModelDocument* mdoc = dynamic_cast<CModelDocument*>(GetDocument());
		if (mdoc == nullptr) return;
		FESelection* ps = mdoc->GetCurrentSelection();
		CCommand* cmd = 0;
		if ((ntrans == TRANSFORM_MOVE) && (but == Qt::LeftButton))
		{
			cmd = new CCmdTranslateSelection(mdoc, m_rt);
		}
		else if ((ntrans == TRANSFORM_ROTATE) && (but == Qt::LeftButton))
		{
			if (m_wt != 0)
			{
				quatd q;
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_X) q = quatd(m_wt, vec3d(1,0,0));
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Y) q = quatd(m_wt, vec3d(0,1,0));
				if (pivotMode == PIVOT_SELECTION_MODE::SELECT_Z) q = quatd(m_wt, vec3d(0,0,1));

				if ((m_coord == COORD_LOCAL) || postDoc)
				{
					quatd qs = ps->GetOrientation();
					q = qs*q*qs.Inverse();
				}

				q.MakeUnit();
				cmd = new CCmdRotateSelection(mdoc, q, GetPivotPosition());
				m_wt = 0;
			}
		}
		else if ((ntrans == TRANSFORM_SCALE) && (but == Qt::LeftButton))
		{
			cmd = new CCmdScaleSelection(mdoc, m_st, m_ds, GetPivotPosition());
			m_st = m_sa = 1;
		}

		if (cmd && ps)
		{
			string s = ps->GetName();
			pdoc->AddCommand(cmd, s);
			mdoc->GetGModel()->UpdateBoundingBox();
		}

		// TODO: Find a better way to update the GMesh when necessary. 
		//       When I move FE nodes, I need to rebuild the GMesh. 
		//       This still causes a delay between the GMesh update since we do this
		//       when the mouse is released, but I'm not sure how to do this better.
//		if (pdoc->GetActiveObject()) pdoc->GetActiveObject()->BuildGMesh();
	}

	ev->accept();
}

void CGLView::wheelEvent(QWheelEvent* ev)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	CGLCamera& cam = scene->GetView().GetCamera();

	int pivotMode = m_pivot.GetSelectionMode();

	Qt::KeyboardModifiers key = ev->modifiers();
	bool balt   = (key & Qt::AltModifier);
	Qt::MouseEventSource eventSource = ev->source();
	if (eventSource == Qt::MouseEventSource::MouseEventNotSynthesized)
	{
		int y = ev->angleDelta().y();
		if (y == 0) y = ev->angleDelta().x();
		if (balt && GetViewSettings().m_bselbrush)
		{
			float& R = GetViewSettings().m_brushSize;
			if (y < 0) R -= 2.f;
			if (y > 0) R += 2.f;
			if (R < 2.f) R = 1.f;
			if (R > 500.f) R = 500.f;
		}
		else
		{
			if (y > 0) cam.Zoom(0.95f);
			if (y < 0) cam.Zoom(1.0f / 0.95f);
		}
		repaint();
		m_pWnd->UpdateGLControlBar();
	}
	else
	{
		if (balt) {
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_NONE)
			{
				int y = ev->angleDelta().y();
				if (y > 0) cam.Zoom(0.95f);
				if (y < 0) cam.Zoom(1.0f / 0.95f);

				repaint();

				m_pWnd->UpdateGLControlBar();
			}
		}
		else {
			if (pivotMode == PIVOT_SELECTION_MODE::SELECT_NONE)
			{
				int dx = ev->pixelDelta().x();
				int dy = ev->pixelDelta().y();
				vec3d r = vec3d(-dx, dy, 0.f);
				cam.PanView(r);

				repaint();

				m_pWnd->UpdateGLControlBar();
			}
		}
	}

	cam.Update(true);
	ev->accept();
}

bool CGLView::gestureEvent(QNativeGestureEvent* ev)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return true;

	CGLCamera& cam = scene->GetView().GetCamera();

    if (ev->gestureType() == Qt::ZoomNativeGesture) {
        if (ev->value() < 0) {
            cam.Zoom(1./(1.0f+(float)ev->value()));
        }
        else {
            cam.Zoom(1.0f-(float)ev->value());
        }
    }
    else if (ev->gestureType() == Qt::RotateNativeGesture) {
        // rotate in-plane
        quatd qz = quatd(-2*ev->value()*0.01745329, vec3d(0, 0, 1));
        cam.Orbit(qz);
    }
    repaint();
    cam.Update(true);
    update();
    return true;
}

bool CGLView::event(QEvent* event)
{
    if (event->type() == QEvent::NativeGesture)
        return gestureEvent(static_cast<QNativeGestureEvent*>(event));
    return QOpenGLWidget::event(event);
}

void CGLView::initializeGL()
{
	CGLSceneView::initializeGL();

	glPolygonStipple(poly_mask);

	if (m_ballocDefaultWidgets)
	{
		m_Widget = CGLWidgetManager::GetInstance(); assert(m_Widget);
		m_Widget->AttachToView(this);

		int Y = 0;
		m_Widget->AddWidget(m_ptitle = new GLBox(20, 20, 300, 50, ""), 0);
		m_ptitle->set_font_size(30);
		m_ptitle->fit_to_size();
		m_ptitle->set_label("$(filename)");
		Y += m_ptitle->h();

		m_Widget->AddWidget(m_psubtitle = new GLBox(Y, 70, 300, 60, ""), 0);
		m_psubtitle->set_font_size(15);
		m_psubtitle->fit_to_size();
		m_psubtitle->set_label("$(datafield) $(units)\\nTime = $(time)");

		m_Widget->AddWidget(m_ptriad = new GLTriad(0, 0, 150, 150), 0);
		m_ptriad->align(GLW_ALIGN_LEFT | GLW_ALIGN_BOTTOM);
		m_Widget->AddWidget(m_pframe = new GLSafeFrame(0, 0, 800, 600));
		m_pframe->align(GLW_ALIGN_HCENTER | GLW_ALIGN_VCENTER);
		m_pframe->hide();
		m_pframe->set_layer(0); // permanent widget

		m_Widget->AddWidget(m_legend = new GLLegendBar(&m_colorMap, 0, 0, 120, 600), 0);
		m_legend->align(GLW_ALIGN_RIGHT | GLW_ALIGN_VCENTER);
		m_legend->hide();
	}

	const char* szv = (const char*) glGetString(GL_VERSION);
	m_oglVersionString = szv;

	// initialize clipping planes
	Post::CGLPlaneCutPlot::InitClipPlanes();
}

void CGLView::Reset()
{
	// default display properties
	int ntheme = m_pWnd->currentTheme();
	m_view.Defaults(ntheme);

	GLHighlighter::ClearHighlights();
	repaint();
}

void CGLView::UpdateWidgets(bool bposition)
{
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	if (postDoc && postDoc->IsValid())
	{
		int Y = 0;
		if (m_ptitle)
		{
			m_ptitle->fit_to_size();

			if (bposition)
				m_ptitle->resize(0, 0, m_ptitle->w(), m_ptitle->h());

			m_ptitle->fit_to_size();
			Y = m_ptitle->y() + m_ptitle->h();
		}

		if (m_psubtitle)
		{
			if (bposition)
				m_psubtitle->resize(0, Y, m_psubtitle->w(), m_psubtitle->h());

			m_psubtitle->fit_to_size();

			// set a min width for the subtitle otherwise the time values may get cropped
			if (m_psubtitle->w() < 150)
				m_psubtitle->resize(m_psubtitle->x(), m_psubtitle->y(), 150, m_psubtitle->h());
		}

		repaint();
	}
}

bool CGLView::isTitleVisible() const
{
	return (m_ptitle ? m_ptitle->visible() : false);
}

void CGLView::showTitle(bool b)
{
	if (m_ptitle)
	{
		if (b) m_ptitle->show(); else m_ptitle->hide();
		repaint();
	}
}

bool CGLView::isSubtitleVisible() const
{
	return (m_psubtitle ? m_psubtitle->visible() : false);
}

void CGLView::showSubtitle(bool b)
{
	if (m_psubtitle)
	{
		if (b) m_psubtitle->show(); else m_psubtitle->hide();
		repaint();
	}
}

QImage CGLView::CaptureScreen()
{
	if (m_pframe && m_pframe->visible())
	{
		QImage im = grabFramebuffer();

		// crop based on the capture frame
		double dpr = devicePixelRatio();
		return im.copy((int)(dpr*m_pframe->x()), (int)(dpr*m_pframe->y()), (int)(dpr*m_pframe->w()), (int)(dpr*m_pframe->h()));
	}
	else return grabFramebuffer();
}

void CGLView::repaintEvent()
{
	repaint();
}

void CGLView::RenderDecorations()
{
	if (m_deco.empty() == false)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glColor3ub(255, 255, 0);
		for (int i = 0; i < m_deco.size(); ++i)
		{
			m_deco[i]->render();
		}
		glPopAttrib();
	}
}

void CGLView::RenderScene()
{
	time_point<steady_clock> startTime = steady_clock::now();

	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	GLViewSettings& view = GetViewSettings();

	CGLCamera& cam = scene->GetView().GetCamera();
	cam.SetOrthoProjection(GetView()->OrhographicProjection());

	CGLContext& rc = m_rc;
	rc.m_view = this;
	rc.m_cam = &cam;
	rc.m_settings = view;

	PositionCamera();

	if (scene) scene->Render(rc);

	RenderPivot();

	if (m_bsel && (m_pivot.GetSelectionMode() == PIVOT_SELECTION_MODE::SELECT_NONE)) RenderRubberBand();

	if (view.m_bselbrush) RenderBrush();

	// set the projection Matrix to ortho2d so we can draw some stuff on the screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// update the triad
	if (m_ptriad) m_ptriad->setOrientation(cam.GetOrientation());

	// We must turn off culling before we use the QPainter, otherwise
	// drawing using QPainter doesn't work correctly.
	glDisable(GL_CULL_FACE);

	// render the GL widgets
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	CPostDocument* postDoc = m_pWnd->GetPostDocument();
	if (postDoc == nullptr)
	{
		CModelDocument* mdoc = m_pWnd->GetModelDocument();
		if (mdoc && m_Widget)
		{
			FSModel* ps = mdoc->GetFSModel();
			GModel& model = ps->GetModel();

			if (m_ptitle) m_ptitle->hide();
			if (m_psubtitle) m_psubtitle->hide();

			painter.setPen(QPen(QColor::fromRgb(164, 164, 164)));
			int activeLayer = model.GetActiveMeshLayer();
			const std::string& s = model.GetMeshLayerName(activeLayer);
			painter.drawText(0, 15, QString("  Mesh Layer > ") + QString::fromStdString(s));
			if (m_ptriad) m_Widget->DrawWidget(m_ptriad, &painter);
			if (m_pframe && m_pframe->visible()) m_Widget->DrawWidget(m_pframe, &painter);

			if (m_legend)
			{
				if (view.m_bcontour)
				{
					GObject* po = mdoc->GetActiveObject();
					FSMesh* pm = (po ? po->GetFEMesh() : nullptr);
					if (pm)
					{
						Mesh_Data& data = pm->GetMeshData();
						double vmin, vmax;
						data.GetValueRange(vmin, vmax);
						if (vmin == vmax) vmax++;
						m_legend->SetRange((float)vmin, (float)vmax);
						m_legend->show();
						m_Widget->DrawWidget(m_legend, &painter);
					}
				}
				else m_legend->hide();
			}

			// draw the other widgets
			int layer = mdoc->GetWidgetLayer();
			m_Widget->SetActiveLayer(layer);
			m_Widget->DrawWidgets(&painter);
		}
	}
	else
	{
		if (postDoc->IsValid() && m_Widget)
		{
			// make sure the model legend bar is hidden
			m_legend->hide();

			// make sure the titles are visible
			if (m_ptitle) m_ptitle->show();
			if (m_psubtitle) m_psubtitle->show();

			// draw the other widgets
			int layer = postDoc->GetWidgetLayer();
			m_Widget->SetActiveLayer(layer);
			m_Widget->DrawWidgets(&painter);
		}
	}

	painter.end();

	if (m_recorder.IsRecording())
	{
		glFlush();
		QImage im = CaptureScreen();
		if (m_recorder.AddFrame(im) == false)
		{
			m_recorder.Stop();
			QMessageBox::critical(this, "FEBio Studio", "An error occurred while writing frame to video stream.");
		}
	}

	if (m_recorder.IsPaused())
	{
		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
		QTextOption to;
		QFont font = painter.font();
		font.setPointSize(24);
		painter.setFont(font);
		painter.setPen(QPen(Qt::red));
		to.setAlignment(Qt::AlignRight | Qt::AlignTop);
		painter.drawText(rect(), "Recording paused", to);
		painter.end();
	}

	// stop time
	time_point<steady_clock> stopTime = steady_clock::now();

	double sec = duration_cast< duration<double> >(stopTime - startTime).count();
	if (m_showFPS)
	{
		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
		QTextOption to;
		QFont font = painter.font();
		font.setPointSize(12);
		painter.setFont(font);
		painter.setPen(QPen(Qt::red));
		to.setAlignment(Qt::AlignRight | Qt::AlignTop);
		painter.drawText(rect(), QString("FPS: %1").arg(1.0 / sec), to);
		painter.end();
	}
}

void CGLView::Render3DCursor()
{
	// only render if the 3D cursor is valid
	// (i.e. the user picked something on the screen)
	if (m_bpick == false) return;

	vec3d r = Get3DCursor();
	constexpr double R = 10.0;

	GLViewTransform transform(this);

	const int W = width();
	const int H = height();
	const int c = R*0.5;

	vec3d p = transform.WorldToScreen(r);
	p.y = H - p.y;
	p.z = 1;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
//	glEnable(GL_LINE_STIPPLE);
//	glLineStipple(1, 0xAAAA);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width(), 0, height());

	glColor3ub(255, 164, 164);
	glx::drawLine(p.x - R, p.y, p.x - R + c, p.y);
	glx::drawLine(p.x + R, p.y, p.x + R - c, p.y);
	glx::drawLine(p.x, p.y - R, p.x, p.y - R + c);
	glx::drawLine(p.x, p.y + R, p.x, p.y + R - c);
	glx::drawCircle(p, R, 36);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

QPoint CGLView::DeviceToPhysical(int x, int y)
{
	double dpr = devicePixelRatio();
	return QPoint((int)(dpr*x), m_viewport[3] - (int)(dpr * y));
}

inline vec3d mult_matrix(GLfloat m[4][4], vec3d r)
{
	vec3d a;
	a.x = m[0][0] * r.x + m[0][1] * r.y + m[0][2] * r.z;
	a.y = m[1][0] * r.x + m[1][1] * r.y + m[1][2] * r.z;
	a.z = m[2][0] * r.x + m[2][1] * r.y + m[2][2] * r.z;
	return a;
}

void CGLView::PositionCamera()
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	// position the camera
	CGLCamera& cam = scene->GetView().GetCamera();
	cam.Transform();

	CPostDocument* pdoc = m_pWnd->GetPostDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	// see if we need to track anything
	if (pdoc->IsValid() && m_btrack)
	{
		FSMeshBase* pm = pdoc->GetPostObject()->GetFEMesh();
		int NN = pm->Nodes();
		int* nt = m_ntrack;
		if ((nt[0] >= NN) || (nt[1] >= NN) || (nt[2] >= NN)) { m_btrack = false; return; }

		Post::FEPostModel& fem = *pdoc->GetFSModel();

		vec3d a = pm->Node(nt[0]).r;
		vec3d b = pm->Node(nt[1]).r;
		vec3d c = pm->Node(nt[2]).r;

		vec3d e1 = (b - a);
		vec3d e3 = e1 ^ (c - a);
		vec3d e2 = e3^e1;
		e1.Normalize();
		e2.Normalize();
		e3.Normalize();

		vec3d r0 = cam.GetPosition();
		vec3d r1 = a;

		// undo camera translation
		glTranslatef(r0.x, r0.y, r0.z);

		// set current orientation
		mat3d Q;
		Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
		Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
		Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

		// setup the rotation matrix that rotates back to the original
		// tracking orientation
		mat3d R = m_rot0*Q.inverse();

		// note that we need to pass the transpose to OGL
		GLfloat m[4][4] = { 0 };
		m[3][3] = 1.f;
		m[0][0] = R[0][0]; m[0][1] = R[1][0]; m[0][2] = R[2][0];
		m[1][0] = R[0][1]; m[1][1] = R[1][1]; m[1][2] = R[2][1];
		m[2][0] = R[0][2]; m[2][1] = R[1][2]; m[2][2] = R[2][2];
		glMultMatrixf(&m[0][0]);

		// center camera on track point
		glTranslatef(-r1.x, -r1.y, -r1.z);

		m_rc.m_btrack = true;
		m_rc.m_track_pos = r1;

		// This would make the plane cut relative to the element coordinate system
		m_rc.m_track_rot = quatd(R);
	}
	else m_rc.m_btrack = false;
}

void CGLView::SetTrackingData(int n[3])
{
	// store the nodes to track
	m_ntrack[0] = n[0];
	m_ntrack[1] = n[1];
	m_ntrack[2] = n[2];

	// get the current nodal positions
	CPostDocument* pdoc = m_pWnd->GetPostDocument();
	FSMeshBase* pm = pdoc->GetPostObject()->GetFEMesh();
	int NN = pm->Nodes();
	int* nt = m_ntrack;
	if ((nt[0] >= NN) || (nt[1] >= NN) || (nt[2] >= NN)) { assert(false); return; }

	Post::FEPostModel& fem = *pdoc->GetFSModel();
	vec3d a = pm->Node(nt[0]).r;
	vec3d b = pm->Node(nt[1]).r;
	vec3d c = pm->Node(nt[2]).r;

	// setup orthogonal basis
	vec3d e1 = (b - a);
	vec3d e3 = e1 ^ (c - a);
	vec3d e2 = e3^e1;
	e1.Normalize();
	e2.Normalize();
	e3.Normalize();

	// create matrix form
	mat3d Q;
	Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
	Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
	Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

	// store as quat
	m_rot0 = Q;
}

void CGLView::TrackSelection(bool b)
{
	if (b == false)
	{
		m_btrack = false;
	}
	else
	{
		m_btrack = false;
		CPostDocument* pdoc = m_pWnd->GetPostDocument();
		if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

		Post::CGLModel* model = pdoc->GetGLModel(); assert(model);

		int nmode = model->GetSelectionMode();
		FSMeshBase* pm = pdoc->GetPostObject()->GetFEMesh();
		if (nmode == Post::SELECT_ELEMS)
		{
			const vector<FEElement_*> selElems = pdoc->GetGLModel()->GetElementSelection();
			if (selElems.size() > 0)
			{
				FEElement_& el = *selElems[0];
				int* n = el.m_node;
				int m[3] = { n[0], n[1], n[2] };
				SetTrackingData(m);
				m_btrack = true;
			}
		}
		else if (nmode == Post::SELECT_NODES)
		{
			int ns = 0;
			int m[3];
			for (int i = 0; i<pm->Nodes(); ++i)
			{
				if (pm->Node(i).IsSelected()) m[ns++] = i;
				if (ns == 3)
				{
					SetTrackingData(m);
					m_btrack = true;
					break;
				}
			}
		}
	}
}

void CGLView::ShowMeshData(bool b)
{
	GetViewSettings().m_bcontour = b;
	m_planeCut.Clear();	// TODO: Why do we do this? 
}

void SetModelView(GObject* po)
{
	// get transform data
	vec3d r = po->GetTransform().GetPosition();
	vec3d s = po->GetTransform().GetScale();
	quatd q = po->GetTransform().GetRotation();

	// translate mesh
	glTranslated(r.x, r.y, r.z);

	// orient mesh
	double w = 180 * q.GetAngle() / PI;
	if (w != 0)
	{
		vec3d r = q.GetVector();
		glRotated(w, r.x, r.y, r.z);
	}

	// scale the mesh
	glScaled(s.x, s.y, s.z);
}

void CGLView::SetCoordinateSystem(int nmode)
{
	m_coord = nmode;
}

void CGLView::UndoViewChange()
{
	if (m_Cmd.CanUndo()) m_Cmd.UndoCommand();
	repaint();
}

void CGLView::RedoViewChange()
{
	if (m_Cmd.CanRedo()) m_Cmd.RedoCommand();
	repaint();
}

void CGLView::ClearCommandStack()
{
	m_Cmd.Clear();
}

int CGLView::GetMeshMode()
{
	return m_pWnd->GetMeshMode();
}

bool CGLView::TrackModeActive()
{
	return m_btrack;
}

void CGLView::RenderTrack()
{
	if (m_btrack == false) return;

	CPostDocument* pdoc = m_pWnd->GetPostDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	FSMeshBase* pm = pdoc->GetPostObject()->GetFEMesh();
	int* nt = m_ntrack;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	vec3d a = pm->Node(nt[0]).r;
	vec3d b = pm->Node(nt[1]).r;
	vec3d c = pm->Node(nt[2]).r;

	vec3d e1 = (b - a);
	vec3d e3 = e1 ^ (c - a);
	vec3d e2 = e3^e1;
	double l = e1.Length();
	e1.Normalize();
	e2.Normalize();
	e3.Normalize();

	vec3d A, B, C;
	A = a + e1*l;
	B = a + e2*l;
	C = a + e3*l;

	glColor3ub(255, 0, 255);
	glBegin(GL_LINES);
	{
		glVertex3f(a.x, a.y, a.z); glVertex3f(A.x, A.y, A.z);
		glVertex3f(a.x, a.y, a.z); glVertex3f(B.x, B.y, B.z);
		glVertex3f(a.x, a.y, a.z); glVertex3f(C.x, C.y, C.z);
	}
	glEnd();

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This function renders the manipulator at the current pivot
//
void CGLView::RenderPivot()
{
	CGLDocument* pdoc = dynamic_cast<CGLDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();

	// make there is something selected
	if (!ps || (ps->Size() == 0)) return;

	// get the global position of the pivot
	// this is where we place the manipulator
	vec3d rp = GetPivotPosition();

	CGLCamera& cam = *GetCamera();

	// determine the scale of the manipulator
	// we make it depend on the target distanceso that the 
	// manipulator will look about the same size regardless the zoom
	double d = 0.1*cam.GetTargetDistance();

	// push the modelview matrix
	glPushMatrix();

	// position the manipulator
	glTranslatef((float)rp.x, (float)rp.y, (float)rp.z);

	// orient the manipulator
	// (we always use local for post docs)
	int orient = m_coord;
	if (dynamic_cast<CPostDocument*>(pdoc)) orient = COORD_LOCAL;
	if (orient == COORD_LOCAL)
	{
		quatd q = ps->GetOrientation();
		double w = 180.0*q.GetAngle() / PI;
		vec3d r = q.GetVector();
		if (w != 0) glRotated(w, r.x, r.y, r.z);
	}

	// render the manipulator
	int nitem = pdoc->GetItemMode();
	int nsel = pdoc->GetSelectionMode();
	bool bact = true;
	if ((nitem == ITEM_MESH) && (nsel != SELECT_OBJECT)) bact = false;
	int ntrans = pdoc->GetTransformMode();
	m_pivot.Render(ntrans, d, bact);

	// restore the modelview matrix
	glPopMatrix();
}

void CGLView::RenderRubberBand()
{
	// Get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	int nstyle = pdoc->GetSelectionStyle();

	// set the ortho
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255, 255, 255);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineStipple(1, (GLushort)0xF0F0);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_STIPPLE);

	switch (nstyle)
	{
	case REGION_SELECT_BOX: glRecti(m_x0, m_y0, m_x1, m_y1); break;
	case REGION_SELECT_CIRCLE:
		{
			double dx = (m_x1 - m_x0);
			double dy = (m_y1 - m_y0);
			double R = sqrt(dx*dx + dy*dy);
			glx::drawCircle(vec3d(m_x0, m_y0, 0), R, 24);
		}
		break;
	case REGION_SELECT_FREE:
		{
			glBegin(GL_LINE_STRIP);
			{
				for (int i = 0; i<(int)m_pl.size(); ++i)
				{
					int x = m_pl[i].first;
					int y = m_pl[i].second;
					glVertex2i(x, y);
				}
			}
			glEnd();
		}
		break;
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
}

void CGLView::RenderBrush()
{
	// set the ortho
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255, 255, 255);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineStipple(1, (GLushort)0xF0F0);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_STIPPLE);

	double R = GetViewSettings().m_brushSize;
	int n = (int)(R / 2);
	if (n < 12) n = 12;
	glx::drawCircle(vec3d(m_x1, m_y1, 0), R, n);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
}

void CGLView::ShowSafeFrame(bool b)
{
	if (m_pframe)
	{
		if (b) m_pframe->show();
		else m_pframe->hide();
	}
}

void CGLView::SetViewMode(View_Mode n)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	GLViewSettings& view = GetViewSettings();
	int c = view.m_nconv;
	quatd q;

	switch (c) {
        case CONV_FR_XZ:
        {
            // set the plane orientation
            switch (n)
            {
                case VIEW_FRONT:
                case VIEW_BACK: q = quatd(90 * DEG2RAD, vec3d(1, 0, 0)); break;
                case VIEW_RIGHT:
                case VIEW_LEFT: q = quatd(90 * DEG2RAD, vec3d(1, 0, 0)); q *= quatd(90 * DEG2RAD, vec3d(0, 1, 0)); break;
                case VIEW_TOP:
                case VIEW_BOTTOM:
                case VIEW_ISOMETRIC:
                case VIEW_USER: q = quatd(0, vec3d(0, 0, 1)); break;
                default:
                    assert(false);
            }
            
            m_nview = n;
            scene->SetGridOrientation(q);
            
            // set the camera orientation
            switch (n)
            {
                case VIEW_TOP: q = quatd(0, vec3d(0, 0, 1)); break;
                case VIEW_BOTTOM: q = quatd(180 * DEG2RAD, vec3d(1, 0, 0)); break;
                case VIEW_LEFT: q = quatd(-90 * DEG2RAD, vec3d(1, 0, 0)); q *= quatd(-90 * DEG2RAD, vec3d(0, 0, 1)); break;
                case VIEW_RIGHT: q = quatd(-90 * DEG2RAD, vec3d(1, 0, 0)); q *= quatd(90 * DEG2RAD, vec3d(0, 0, 1)); break;
                case VIEW_FRONT: q = quatd(-90 * DEG2RAD, vec3d(1, 0, 0)); break;
                case VIEW_BACK: q = quatd(-90 * DEG2RAD, vec3d(1, 0, 0)); q *= quatd(180 * DEG2RAD, vec3d(0, 0, 1)); break;
                case VIEW_ISOMETRIC: q = quatd(56.6003 * DEG2RAD, vec3d(0.590284, -0.769274, -0.244504))*quatd(-90 * DEG2RAD, vec3d(1, 0, 0)); break;
                case VIEW_USER: repaint(); return;
            }
        }
            break;
        case CONV_FR_XY:
        {
            // set the plane orientation
            switch (n)
            {
                case VIEW_FRONT:
                case VIEW_BACK: q = quatd(0, vec3d(0,1,0)); break;
                case VIEW_RIGHT:
                case VIEW_LEFT: q = quatd( 90*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_TOP:
                case VIEW_BOTTOM: q = quatd(-90*DEG2RAD, vec3d(1,0,0)); break;
                case VIEW_ISOMETRIC:
                case VIEW_USER: q = quatd(0, vec3d(0, 0, 1)); break;
                default:
                    assert(false);
            }
            
            m_nview = n;
			scene->SetGridOrientation(q);
            
            // set the camera orientation
            switch (n)
            {
                case VIEW_FRONT : q = quatd(0, vec3d(1,0,0)); break;
                case VIEW_BACK  : q = quatd(180*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_LEFT  : q = quatd(-90*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_RIGHT : q = quatd( 90*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_TOP   : q = quatd(-90*DEG2RAD, vec3d(1,0,0)); break;
                case VIEW_BOTTOM: q = quatd( 90*DEG2RAD, vec3d(1,0,0)); break;
                case VIEW_ISOMETRIC: q = quatd(56.6003 * DEG2RAD, vec3d(0.590284, -0.769274, -0.244504)); break;
                case VIEW_USER: repaint(); return;
            }
        }
            break;
        case CONV_US_XY:
        {
            // set the plane orientation
            switch (n)
            {
                case VIEW_FRONT:
                case VIEW_BACK: q = quatd(0, vec3d(1,0,0)); break;
                case VIEW_RIGHT:
                case VIEW_LEFT: q = quatd(-90*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_TOP:
                case VIEW_BOTTOM: q = quatd( 90*DEG2RAD, vec3d(1,0,0)); break;
                case VIEW_ISOMETRIC:
                case VIEW_USER: q = quatd(0, vec3d(0, 0, 1)); break;
                default:
                    assert(false);
            }
            
            m_nview = n;
			scene->SetGridOrientation(q);
            
            // set the camera orientation
            switch (n)
            {
                case VIEW_FRONT : q = quatd(0, vec3d(1,0,0)); break;
                case VIEW_BACK  : q = quatd(180*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_LEFT  : q = quatd( 90*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_RIGHT : q = quatd(-90*DEG2RAD, vec3d(0,1,0)); break;
                case VIEW_TOP   : q = quatd( 90*DEG2RAD, vec3d(1,0,0)); break;
                case VIEW_BOTTOM: q = quatd(-90*DEG2RAD, vec3d(1,0,0)); break;
                case VIEW_ISOMETRIC: q = quatd(56.6003 * DEG2RAD, vec3d(0.590284, -0.769274, -0.244504)); break;
                case VIEW_USER: repaint(); return;
            }
        }
            break;
    }

	scene->GetCamera().SetOrientation(q);

	// set the camera target
	//	m_Cam.SetTarget(vec3d(0,0,0));

	repaint();
}

void CGLView::TogglePerspective(bool b)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	CGView& view = scene->GetView();
	view.m_bortho = b;
	repaint();
}

void CGLView::AddRegionPoint(int x, int y)
{
	if (m_pl.empty()) m_pl.push_back(pair<int, int>(x, y));
	else
	{
		pair<int, int>& p = m_pl[m_pl.size() - 1];
		if ((p.first != x) || (p.second != y)) m_pl.push_back(pair<int, int>(x, y));
	}
}

void CGLView::AddDecoration(GDecoration* deco)
{
	if (deco == nullptr) return;
	// make sure the deco is not defined
	for (int i = 0; i < m_deco.size(); ++i)
	{
		if (deco == m_deco[i]) return;
	}
	m_deco.push_back(deco);
}

void CGLView::RemoveDecoration(GDecoration* deco)
{
	if (deco == nullptr) return;
	for (int i = 0; i < m_deco.size(); ++i)
	{
		if (deco == m_deco[i])
		{
			m_deco.erase(m_deco.begin() + i);
			return;
		}
	}
}

void CGLView::ShowPlaneCut(bool b)
{
	m_showPlaneCut = b;
	UpdatePlaneCut(true);
	update();
}

bool CGLView::ShowPlaneCut() const
{
	return m_showPlaneCut;
}

void CGLView::SetPlaneCutMode(int nmode)
{
	m_planeCutMode = nmode;
	UpdatePlaneCut(true);
	update();
}

void CGLView::SetPlaneCut(double d[4])
{
	CModelDocument* doc = m_pWnd->GetModelDocument();
	if (doc == nullptr) return;

	BOX box = doc->GetGModel()->GetBoundingBox();

	double R = box.GetMaxExtent();
	if (R < 1e-12) R = 1.0;

	vec3d n(d[0], d[1], d[2]);

	vec3d a = box.r0();
	vec3d b = box.r1();
	vec3d r[8];
	r[0] = vec3d(a.x, a.y, a.z);
	r[1] = vec3d(b.x, a.y, a.z);
	r[2] = vec3d(b.x, b.y, a.z);
	r[3] = vec3d(a.x, b.y, a.z);
	r[4] = vec3d(a.x, a.y, b.z);
	r[5] = vec3d(b.x, a.y, b.z);
	r[6] = vec3d(b.x, b.y, b.z);
	r[7] = vec3d(a.x, b.y, b.z);
	double d0 = n * r[0];
	double d1 = d0;
	for (int i = 1; i < 8; ++i)
	{
		double d = n * r[i];
		if (d < d0) d0 = d;
		if (d > d1) d1 = d;
	}

	double d3 = d0 + 0.5*(d[3] + 1)*(d1 - d0);
	m_planeCut.SetPlaneCoordinates(d[0], d[1], d[2], -d3);
	update();
}

// Select an arm of the pivot manipulator
bool CGLView::SelectPivot(int x, int y)
{
	// store the old pivot mode
	int oldMode = m_pivot.GetSelectionMode();

	// get the transformation mode
	int ntrans = GetDocument()->GetTransformMode();

	makeCurrent();

	// get a new pivot mode
	int newMode = m_pivot.Pick(ntrans, x, y);
	return (newMode != oldMode);
}

//-----------------------------------------------------------------------------
// highlight edges
void CGLView::HighlightEdge(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// set up selection buffer
	int nsize = 5 * model.Edges();
	if (nsize == 0) return;

	makeCurrent();
	GLViewTransform transform(this);

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	int Objects = model.Objects();
	GEdge* closestEdge = 0;
	double zmin = 0.0;
	for (int i = 0; i<Objects; ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			GMesh* mesh = po->GetRenderMesh(); assert(mesh);
			if (mesh)
			{
				int edges = mesh->Edges();
				for (int j = 0; j<edges; ++j)
				{
					GMesh::EDGE& edge = mesh->Edge(j);

					if ((edge.n[0] != -1) && (edge.n[1] != -1))
					{
						vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(edge.n[0]).r);
						vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(edge.n[1]).r);

						vec3d p0 = transform.WorldToScreen(r0);
						vec3d p1 = transform.WorldToScreen(r1);

						if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
						{
							if ((closestEdge == 0) || (p0.z < zmin))
							{
								closestEdge = po->Edge(edge.pid);
								zmin = p0.z;
							}
						}
					}
				}
			}
		}
	}
	if (closestEdge != 0) GLHighlighter::SetActiveItem(closestEdge);
	else GLHighlighter::SetActiveItem(0);
}

//-----------------------------------------------------------------------------
// highlight nodes
void CGLView::HighlightNode(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// set up selection buffer
	int nsize = 5 * model.Nodes();
	if (nsize == 0) return;

	makeCurrent();
	GLViewTransform transform(this);

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	int Objects = model.Objects();
	GNode* closestNode = nullptr;
	double zmin = 0.0;
	for (int i = 0; i < Objects; ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			int nodes = po->Nodes();
			for (int j = 0; j < nodes; ++j)
			{
				GNode* pn = po->Node(j);

				vec3d r = pn->Position();

				vec3d p = transform.WorldToScreen(r);

				if (rt.contains(QPoint((int)p.x, (int)p.y)))
				{
					if ((closestNode == nullptr) || (p.z < zmin))
					{
						closestNode = pn;
						zmin = p.z;
					}
				}
			}
		}
	}
	if (closestNode != nullptr) GLHighlighter::SetActiveItem(closestNode);
	else GLHighlighter::SetActiveItem(nullptr);
}

CGLScene* CGLView::GetActiveScene()
{
	CGLDocument* doc = m_pWnd->GetGLDocument();
	if (doc) return doc->GetScene();
	return nullptr;
}

GObject* CGLView::GetActiveObject()
{
	return m_pWnd->GetActiveObject();
}

vec3d CGLView::PickPoint(int x, int y, bool* success)
{
	makeCurrent();
	GLViewTransform transform(this);

	if (success) *success = false;
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return vec3d(0,0,0);

	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return vec3d(0, 0, 0);

	GLViewSettings& view = GetViewSettings();

	// if a temp object is available, see if we can pick a point
	GObject* ptmp = m_pWnd->GetCreatePanel()->GetTempObject();
	if (ptmp)
	{
		int S = 4;
		QRect rt(x - S, y - S, 2 * S, 2 * S);

		int nodes = ptmp->Nodes();
		for (int i=0; i<nodes; ++i)
		{
			GNode* node = ptmp->Node(i);
			vec3d r = node->Position();

			vec3d p = transform.WorldToScreen(r);
			if (rt.contains((int)p.x, (int) p.y)) 
			{
				if (success) *success = true;
				return r;
			}
		}
	}

	// convert the point to a ray
	Ray ray = transform.PointToRay(x, y);

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po && po->GetEditableMesh())
	{
		// convert to local coordinates
		vec3d rl = po->GetTransform().GlobalToLocal(ray.origin);
		vec3d nl = po->GetTransform().GlobalToLocalNormal(ray.direction);

		FSMeshBase* mesh = po->GetEditableMesh();
		vec3d q;
		if (FindIntersection(*mesh, rl, nl, q, view.m_snapToNode))
		{
			if (success) *success = true;
			q = po->GetTransform().LocalToGlobal(q);
			return q;
		}
	}
	else
	{
		// pick a point on the grid
		GGrid& grid = scene->GetGrid();
		vec3d r = grid.Intersect(ray.origin, ray.direction, view.m_snapToGrid);
		if (success) *success = true;

		vec3d p = transform.WorldToScreen(r);

		return r;
	}

	return vec3d(0,0,0);
}

vec3d CGLView::GetPickPosition()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return vec3d(0, 0, 0);
	return Get3DCursor();
}

vec3d CGLView::GetPivotPosition()
{
	if (m_userPivot == false)
	{
		CGLDocument* pdoc = dynamic_cast<CGLDocument*>(GetDocument());
		if (pdoc == nullptr) return vec3d(0,0,0);

		FESelection* ps = pdoc->GetCurrentSelection();
		vec3d r(0, 0, 0);
		if (ps && ps->Size())
		{
			r = ps->GetPivot();
			if (fabs(r.x)<1e-7) r.x = 0;
			if (fabs(r.y)<1e-7) r.y = 0;
			if (fabs(r.z)<1e-7) r.z = 0;
		}

		m_pivot.SetPosition(r);
	}

	return m_pivot.GetPosition();
}

void CGLView::SetPivotPosition(const vec3d& r)
{ 
	m_pivot.SetPosition(r);
	repaint();
}

quatd CGLView::GetPivotRotation()
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if ((doc && (m_coord == COORD_LOCAL)) || dynamic_cast<CPostDocument*>(GetDocument()))
	{
		FESelection* ps = doc->GetCurrentSelection();
		if (ps) return ps->GetOrientation();
	}

	return quatd(0.0, 0.0, 0.0, 1.0);
}

bool CGLView::GetPivotUserMode() const { return m_userPivot; }
void CGLView::SetPivotUserMode(bool b) { m_userPivot = b; }

// this function will only adjust the camera if the currently
// selected object is too close.
void CGLView::ZoomSelection(bool forceZoom)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	// get the selection's bounding box
	BOX box = scene->GetSelectionBox();
	if (box.IsValid())
	{
		double f = box.GetMaxExtent();
		if (f < 1.0e-8) f = 1.0;

		CGLCamera& cam = scene->GetCamera();

		double g = cam.GetFinalTargetDistance();
		if ((forceZoom == true) || (g < 2.0*f))
		{
			cam.SetTarget(box.Center());
			cam.SetTargetDistance(2.0*f);
			repaint();
		}
	}
	else ZoomExtents();
}

void CGLView::ZoomToObject(GObject *po)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	BOX box = po->GetGlobalBox();

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = scene->GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0*f);
	cam.SetOrientation(po->GetTransform().GetRotationInverse());

	repaint();
}

//-----------------------------------------------------------------
//! zoom in on a box
void CGLView::ZoomTo(const BOX& box)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = scene->GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0*f);

	repaint();
}

void CGLView::ZoomExtents(bool banimate)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	BOX box = scene->GetBoundingBox();

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = scene->GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0*f);

	if (banimate == false) cam.Update(true);

	repaint();
}

//-----------------------------------------------------------------------------
//! Render the tags on the selected items.
void CGLView::RenderTags()
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetViewSettings();

	GObject* po = GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	FSMeshBase* pmb = pm;
	if (pm == nullptr)
	{
		GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
		if (pso) pmb = pso->GetSurfaceMesh();
		if (pmb == nullptr) return;
	}

	// create the tag array.
	// We add a tag for each selected item
	GLTAG tag;
	vector<GLTAG> vtag;

	// clear the node tags
	int NN = pmb->Nodes();
	for (int i = 0; i < NN; ++i) pmb->Node(i).m_ntag = 0;

	int mode = doc->GetItemMode();

	GLColor extcol(255, 255, 0);
	GLColor intcol(255, 0, 0);

	// process elements
	if (view.m_ntagInfo > TagInfoOption::NO_TAG_INFO)
	{
		if ((mode == ITEM_ELEM) && pm)
		{
			int NE = pm->Elements();
			for (int i = 0; i < NE; i++)
			{
				FEElement_& el = pm->Element(i);
				if (el.IsSelected())
				{
					tag.r = pm->LocalToGlobal(pm->ElementCenter(el));
					tag.c = extcol;
					int nid = el.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "E%d", nid);
					vtag.push_back(tag);

					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
				}
			}
		}

		// process faces
		if (mode == ITEM_FACE)
		{
			int NF = pmb->Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& f = pmb->Face(i);
				if (f.IsSelected())
				{
					tag.r = pmb->LocalToGlobal(pmb->FaceCenter(f));
					tag.c = (f.IsExternal() ? extcol : intcol);
					int nid = f.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "F%d", nid);
					vtag.push_back(tag);

					int nf = f.Nodes();
					for (int j = 0; j < nf; ++j) pmb->Node(f.n[j]).m_ntag = 1;
				}
			}
		}

		// process edges
		if (mode == ITEM_EDGE)
		{
			int NC = pmb->Edges();
			for (int i = 0; i < NC; i++)
			{
				FSEdge& edge = pmb->Edge(i);
				if (edge.IsSelected())
				{
					tag.r = pmb->LocalToGlobal(pmb->EdgeCenter(edge));
					tag.c = extcol;
					int nid = edge.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "L%d", nid);
					vtag.push_back(tag);

					int ne = edge.Nodes();
					for (int j = 0; j < ne; ++j) pmb->Node(edge.n[j]).m_ntag = 1;
				}
			}
		}

		// process nodes
		if (mode == ITEM_NODE)
		{
			for (int i = 0; i < NN; i++)
			{
				FSNode& node = pmb->Node(i);
				if (node.IsSelected())
				{
					tag.r = pmb->LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int nid = node.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", nid);
					vtag.push_back(tag);
				}
			}
		}

		// add additional nodes
		if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
		{
			for (int i = 0; i < NN; i++)
			{
				FSNode& node = pmb->Node(i);
				if (node.m_ntag == 1)
				{
					tag.r = pmb->LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int n = node.GetID();
					if (n < 0) n = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", n);
					vtag.push_back(tag);
				}
			}
		}
	}

	// render object labels
	CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc);
	if (postDoc && view.m_showRigidLabels)
	{
		bool renderRB = view.m_brigid;
		bool renderRJ = view.m_bjoint;
		Post::FEPostModel* fem = postDoc->GetFSModel();
		for (int i = 0; i < fem->PointObjects(); ++i)
		{
			Post::FEPostModel::PointObject& ob = *fem->GetPointObject(i);
			if (ob.IsActive())
			{
				if (((ob.m_tag == 1) && renderRB) ||
					((ob.m_tag  > 1) && renderRJ))
				{
					tag.r = ob.m_pos;
					tag.c = ob.Color();
					snprintf(tag.sztag, sizeof tag.sztag, ob.GetName().c_str());
					vtag.push_back(tag);
				}
			}
		}

		for (int i = 0; i < fem->LineObjects(); ++i)
		{
			Post::FEPostModel::LineObject& ob = *fem->GetLineObject(i);
			if (ob.IsActive() && renderRJ)
			{
				vec3d a = ob.m_r1;
				vec3d b = ob.m_r2;

				tag.r = (a + b) * 0.5;
				tag.c = ob.Color();
				snprintf(tag.sztag, sizeof tag.sztag, ob.GetName().c_str());
				vtag.push_back(tag);
			}
		}
	}

	// if we don't have any tags, just return
	if (vtag.empty()) return;

	// limit the number of tags to render
	const int MAX_TAGS = 100;
	int nsel = (int)vtag.size();
	if (nsel > MAX_TAGS) return; // nsel = MAX_TAGS;

	RenderTags(vtag);
}

void CGLView::RenderTags(std::vector<GLTAG>& vtag)
{
	int nsel = (int)vtag.size();

	// find out where the tags are on the screen
	GLViewTransform transform(this);
	for (int i = 0; i<nsel; i++)
	{
		vec3d p = transform.WorldToScreen(vtag[i].r);
		vtag[i].wx = p.x;
		vtag[i].wy = m_viewport[3] - p.y;
	}

	// render the tags
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(0, m_viewport[2], 0, m_viewport[3]);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	double dpr = devicePixelRatio();
	for (int i = 0; i<nsel; i++)
		{
			glBegin(GL_POINTS);
			{
				glColor3ub(0, 0, 0);
				int x = (int)(vtag[i].wx * dpr);
				int y = (int)(m_viewport[3] - dpr*(m_viewport[3] - vtag[i].wy));
				glVertex2f(x, y);
				glColor3ub(vtag[i].c.r, vtag[i].c.g, vtag[i].c.b);
				glVertex2f(x - 1, y + 1);
			}
			glEnd();
		}

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setFont(QFont("Helvetica", 10));
	for (int i = 0; i<nsel; ++i)
		{
            int x = vtag[i].wx;
            int y = height()*dpr - vtag[i].wy;
			painter.setPen(Qt::black);

			painter.drawText(x + 3, y - 2, vtag[i].sztag);

			GLColor c = vtag[i].c;
			painter.setPen(QColor::fromRgbF(c.r, c.g, c.b));

			painter.drawText(x + 2, y - 3, vtag[i].sztag);
		}

	painter.end();

	glPopAttrib();

	// QPainter messes this up so reset it
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

QSize CGLView::GetSafeFrameSize() const
{
	int cx = width();
	int cy = height();
	if (m_pframe && m_pframe->visible())
	{
		double dpr = devicePixelRatio();
		cx = (int)(dpr * m_pframe->w());
		cy = (int)(dpr * m_pframe->h());
	}
	return QSize(cx, cy);
}

void CGLView::LockSafeFrame()
{
	if (m_pframe) m_pframe->SetState(GLSafeFrame::LOCKED);
	repaint();
}

void CGLView::UnlockSafeFrame()
{
	if (m_pframe) m_pframe->SetState(GLSafeFrame::FREE);
	repaint();
}

void CGLView::UpdatePlaneCut(bool breset)
{
	m_planeCut.Clear();

	CModelDocument* doc = m_pWnd->GetModelDocument();
	if (doc == nullptr) return;

	FSModel& fem = *doc->GetFSModel();

	GModel& mdl = *doc->GetGModel();
	if (mdl.Objects() == 0) return;

	// set the plane normal
	double* d = m_planeCut.GetPlaneCoordinates();
	vec3d norm(d[0], d[1], d[2]);
	double ref = -d[3];

	GLViewSettings& vs = GetViewSettings();

	if (breset)
	{
		for (int n = 0; n < mdl.Objects(); ++n)
		{
			GObject* po = mdl.Object(n);
			if (po->GetFEMesh())
			{
				FSMesh* mesh = po->GetFEMesh();
				int NE = mesh->Elements();
				for (int i = 0; i < NE; ++i)
				{
					FSElement& el = mesh->Element(i);
					el.Show(); el.Unhide();
				}
				po->UpdateItemVisibility();
			}
		}
	}

	if ((m_planeCutMode == Planecut_Mode::PLANECUT) && (m_showPlaneCut))
	{
		m_planeCut.BuildPlaneCut(fem);
	}
	else
	{
		for (int n = 0; n < mdl.Objects(); ++n)
		{
			GObject* po = mdl.Object(n);
			if (po->GetFEMesh())
			{
				FSMesh* mesh = po->GetFEMesh();

				if (m_showPlaneCut)
				{
					int NN = mesh->Nodes();
					for (int i = 0; i < NN; ++i)
					{
						FSNode& node = mesh->Node(i);
						node.m_ntag = 0;

						vec3d ri = mesh->LocalToGlobal(node.pos());
						if (norm*ri < ref)
						{
							node.m_ntag = 1;
						}
					}

					int NE = mesh->Elements();
					for (int i = 0; i < NE; ++i)
					{
						FSElement& el = mesh->Element(i);
						el.Show(); el.Unhide();
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j)
						{
							if (mesh->Node(el.m_node[j]).m_ntag == 1)
							{
								el.Hide();
								break;
							}
						}
					}
				}
				else
				{
					int NE = mesh->Elements();
					for (int i = 0; i < NE; ++i)
					{
						FSElement& el = mesh->Element(i);
						el.Show(); el.Unhide();
					}
				}

				mesh->UpdateItemVisibility();
			}
		}
	}
}

bool CGLView::ShowPlaneCut()
{
	return m_showPlaneCut;
}

GLPlaneCut& CGLView::GetPlaneCut()
{
	return m_planeCut;
}

void CGLView::DeletePlaneCutMesh()
{
	m_planeCut.Clear();
}

int CGLView::PlaneCutMode()
{
	return m_planeCutMode;
}

double* CGLView::PlaneCoordinates()
{
	return m_planeCut.GetPlaneCoordinates();
}

void CGLView::RenderPlaneCut()
{
	CModelDocument* doc = m_pWnd->GetModelDocument();
	if (doc == nullptr) return;

	if (m_planeCut.IsValid() == false)
	{
		FSModel& fem = *doc->GetFSModel();
		m_planeCut.BuildPlaneCut(fem);
	}

	BOX box = doc->GetGModel()->GetBoundingBox();

	glColor3ub(200, 0, 200);
	glx::renderBox(box, false);

	m_planeCut.Render();
}

void CGLView::ToggleFPS()
{
	m_showFPS = !m_showFPS;
}
