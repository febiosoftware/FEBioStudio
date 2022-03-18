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
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include "GLHighlighter.h"
#include "GLCursor.h"
#include <math.h>
#include <QtCore/QTimer>
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/FECurveMesh.h>
#include "GLViewTransform.h"
#include <GLLib/glx.h>
#include <GLLib/GDecoration.h>
#include <PostLib/ColorMap.h>
#include <GLLib/GLCamera.h>
#include <GLLib/GLContext.h>
#include <MeshLib/FENodeEdgeList.h>
#include <MeshLib/FENodeFaceList.h>
#include <FEMLib/FEMultiMaterial.h>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <PostLib/ImageModel.h>
#include "PostDocument.h"
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLModel.h>
#include <MeshTools/GModel.h>
#include "Commands.h"
#include "PostObject.h"
#include <PostLib/ImageSlicer.h>
#include "ImageSliceView.h"

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

const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
const int PYR_NT[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };

// in MeshTools\lut.cpp
extern int LUT[256][15]; 
extern int ET_HEX[12][2];
extern int ET_TET[6][2];
extern int ET_PYR[8][2];

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

//-----------------------------------------------------------------------------

inline void render_triad(double x, double y, double z, double dx, double dy, double dz)
{
	glVertex3d(x, y, z); glVertex3d(x + dx, y, z);
	glVertex3d(x, y, z); glVertex3d(x, y + dy, z);
	glVertex3d(x, y, z); glVertex3d(x, y, z + dz);
}

void RenderBox(const BOX& bbox, bool partial = true, double scale = 1.0)
{
	// push attributes
	glPushAttrib(GL_ENABLE_BIT);

	// set attributes
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);

	BOX box = bbox;
	box.Scale(scale);

	if (partial)
	{
		double dx = box.Width()*0.3;
		double dy = box.Height()*0.3;
		double dz = box.Depth()*0.3;
		glBegin(GL_LINES);
		{
			render_triad(box.x0, box.y0, box.z0,  dx,  dy, dz);
			render_triad(box.x1, box.y0, box.z0, -dx,  dy, dz);
			render_triad(box.x1, box.y1, box.z0, -dx, -dy, dz);
			render_triad(box.x0, box.y1, box.z0,  dx, -dy, dz);

			render_triad(box.x0, box.y0, box.z1,  dx,  dy, -dz);
			render_triad(box.x1, box.y0, box.z1, -dx,  dy, -dz);
			render_triad(box.x1, box.y1, box.z1, -dx, -dy, -dz);
			render_triad(box.x0, box.y1, box.z1,  dx, -dy, -dz);
		}
		glEnd();
	}
	else
	{
		glBegin(GL_LINES);
		{
			glVertex3d(box.x0, box.y0, box.z0); glVertex3d(box.x1, box.y0, box.z0);
			glVertex3d(box.x1, box.y0, box.z0); glVertex3d(box.x1, box.y1, box.z0);
			glVertex3d(box.x1, box.y1, box.z0); glVertex3d(box.x0, box.y1, box.z0);
			glVertex3d(box.x0, box.y1, box.z0); glVertex3d(box.x0, box.y0, box.z0);

			glVertex3d(box.x0, box.y0, box.z1); glVertex3d(box.x1, box.y0, box.z1);
			glVertex3d(box.x1, box.y0, box.z1); glVertex3d(box.x1, box.y1, box.z1);
			glVertex3d(box.x1, box.y1, box.z1); glVertex3d(box.x0, box.y1, box.z1);
			glVertex3d(box.x0, box.y1, box.z1); glVertex3d(box.x0, box.y0, box.z1);

			glVertex3d(box.x0, box.y0, box.z0); glVertex3d(box.x0, box.y0, box.z1);
			glVertex3d(box.x1, box.y0, box.z0); glVertex3d(box.x1, box.y0, box.z1);
			glVertex3d(box.x0, box.y1, box.z0); glVertex3d(box.x0, box.y1, box.z1);
			glVertex3d(box.x1, box.y1, box.z0); glVertex3d(box.x1, box.y1, box.z1);
		}
		glEnd();
	}

	// restore attributes
	glPopAttrib();
}

CGLView::CGLView(CMainWindow* pwnd, QWidget* parent) : QOpenGLWidget(parent), m_pWnd(pwnd), m_Ttor(this), m_Rtor(this), m_Stor(this)
{
	QSurfaceFormat fmt = format();
//	fmt.setSamples(4);
//	setFormat(fmt);

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_AcceptTouchEvents, true);

	m_bsnap = false;
	m_grid.SetView(this);

	m_btrack = false;

	Reset();

	m_ox = m_oy = 1;

	m_wa = 0;
	m_wt = 0;

	m_light = vec3f(0.5, 0.5, 1);

	m_bsel = false;

	m_nview = VIEW_USER;

	m_nsnap = SNAP_NONE;

	m_pivot = PIVOT_NONE;
	m_bpivot = false;
	m_pv = vec3d(0, 0, 0);

	m_bshift = false;
	m_bctrl = false;

	m_btooltip = false;

	m_bpick = false;

	m_coord = COORD_GLOBAL;

	m_pmod = 0;

	setMouseTracking(true);

	m_showPlaneCut = false;
	m_planeCutMode = 0;
	m_plane[0] = 1.0;
	m_plane[1] = 0.0;
	m_plane[2] = 0.0;
	m_plane[3] = 0.0;
	m_planeCut = nullptr;

	// attach the highlighter to this view
	GLHighlighter::AttachToView(this);

	// attach the 3D cursor to this view
	GLCursor::AttachToView(this);

	m_Widget = CGLWidgetManager::GetInstance();
	m_Widget->AttachToView(this);

	m_video       = nullptr;
	m_videoMode   = VIDEO_STOPPED;
	m_videoFormat = GL_RGB;
}

CGLView::~CGLView()
{
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
	CPostDocument* doc = m_pWnd->GetPostDocument();
	if (doc && doc->IsValid())
	{
		CGLCamera& cam = doc->GetView()->GetCamera();
		cam.Update(hitCameraTarget);
	}
}

void CGLView::resizeGL(int w, int h)
{
	QOpenGLWidget::resizeGL(w, h);
	m_Widget->CheckWidgetBounds();
}

void CGLView::changeViewMode(View_Mode vm)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	SetViewMode(vm);

	// switch to ortho view if we're not in it
	bool bortho = doc->GetView()->OrhographicProjection();
	if (bortho == false)
	{
		m_pWnd->toggleOrtho();
	}
}

void CGLView::SetColorMap(Post::CColorMap& map)
{
	m_colorMap = map;
}

void CGLView::mousePressEvent(QMouseEvent* ev)
{
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	int ntrans = pdoc->GetTransformMode();

	int x = ev->x();
	int y = ev->y();

	// get the active view
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	// let the widget manager handle it first
	GLWidget* pw = GLWidget::get_focus();
	if (m_Widget->handle(x, y, CGLWidgetManager::PUSH) == 1)
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

	Qt::MouseButton but = ev->button();

	m_bextrude = false;

	if (but == Qt::LeftButton)
	{
		if ((m_bshift || m_bctrl) && (m_pivot == PIVOT_NONE)) m_bsel = true;
		if ((m_pivot != PIVOT_NONE) && m_bshift && (ntrans == TRANSFORM_MOVE))
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
			if (m_pivot == PIVOT_X) m_ds = vec3d(1, 0, 0);
			if (m_pivot == PIVOT_Y) m_ds = vec3d(0, 1, 0);
			if (m_pivot == PIVOT_Z) m_ds = vec3d(0, 0, 1);
			if (m_pivot == PIVOT_XY) m_ds = vec3d(1, 1, 0);
			if (m_pivot == PIVOT_YZ) m_ds = vec3d(0, 1, 1);
			if (m_pivot == PIVOT_XZ) m_ds = vec3d(1, 0, 1);
		}
		else m_ds = vec3d(1, 1, 1);

		CModelDocument* mdoc = dynamic_cast<CModelDocument*>(GetDocument());
		if (mdoc)
		{
			FESelection* ps = mdoc->GetCurrentSelection();
			if (ps && (m_coord == COORD_LOCAL))
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

	bool bshift = (ev->modifiers() & Qt::ShiftModifier   ? true : false);
	bool bctrl  = (ev->modifiers() & Qt::ControlModifier ? true : false);
	bool balt   = (ev->modifiers() & Qt::AltModifier     ? true : false);

	int ntrans = pdoc->GetTransformMode();

	bool but1 = (ev->buttons() & Qt::LeftButton);
	bool but2 = (ev->buttons() & Qt::MiddleButton);
	bool but3 = (ev->buttons() & Qt::RightButton);

	// get the mouse position
	int x = ev->pos().x();
	int y = ev->pos().y();

	// get the active view
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	// let the widget manager handle it first
	if (but1 && (m_Widget->handle(x, y, CGLWidgetManager::DRAG) == 1))
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
//			if (GLHighlighter::IsTracking())
			{
				HighlightEdge(x, y);
			}
		}
		ev->accept();
		return;
	}

	AddRegionPoint(x, y);

	CGLCamera& cam = pdoc->GetView()->GetCamera();


	if (m_pivot == PIVOT_NONE)
	{
		if (but1 && !m_bsel)
		{
			if (m_nview == VIEW_USER)
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
			PanView(r);
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
		if (but1 && m_bsel) m_pWnd->Update();
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
			FESelection* ps = mdoc->GetCurrentSelection();
			if (m_coord == COORD_LOCAL) ps->GetOrientation().Inverse().RotateVector(dr);

			if (m_pivot == PIVOT_X) dr.y = dr.z = 0;
			if (m_pivot == PIVOT_Y) dr.x = dr.z = 0;
			if (m_pivot == PIVOT_Z) dr.x = dr.y = 0;
			if (m_pivot == PIVOT_XY) dr.z = 0;
			if (m_pivot == PIVOT_YZ) dr.x = 0;
			if (m_pivot == PIVOT_XZ) dr.y = 0;

			if (m_coord == COORD_LOCAL) dr = ps->GetOrientation()*dr;

			m_rg += dr;
			if (bctrl)
			{
				double g = GetGridScale();
				vec3d rt;
				rt.x = g*((int)(m_rg.x / g));
				rt.y = g*((int)(m_rg.y / g));
				rt.z = g*((int)(m_rg.z / g));
				dr = rt - m_rt;
			}

			m_rt += dr;
			ps->Translate(dr);

			m_pWnd->UpdateGLControlBar();
			repaint();
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
				if (m_pivot == PIVOT_X) q = quatd(f, vec3d(1, 0, 0));
				if (m_pivot == PIVOT_Y) q = quatd(f, vec3d(0, 1, 0));
				if (m_pivot == PIVOT_Z) q = quatd(f, vec3d(0, 0, 1));

				FESelection* ps = mdoc->GetCurrentSelection();
				assert(ps);

				if (m_coord == COORD_LOCAL)
				{
					quatd qs = ps->GetOrientation();
					q = qs*q*qs.Inverse();
				}

				q.MakeUnit();
				ps->Rotate(q, GetPivotPosition());
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
				double g = GetGridScale();
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

	int x = ev->x();
	int y = ev->y();

	// let the widget manager handle it first
	if (m_Widget->handle(x, y, CGLWidgetManager::RELEASE) == 1)
	{
		ev->accept();
		m_pWnd->UpdateFontToolbar();
		repaint();
		return;
	}
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

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

	m_bextrude = false;

	AddRegionPoint(x, y);

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	if (m_pivot == PIVOT_NONE)
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
						case SELECT_OBJECT  : SelectObjects (m_x0, m_y0); break;
						case SELECT_PART    : SelectParts   (m_x0, m_y0); break;
						case SELECT_FACE    : SelectSurfaces(m_x0, m_y0); break;
						case SELECT_EDGE    : SelectEdges   (m_x0, m_y0); break;
						case SELECT_NODE    : SelectNodes   (m_x0, m_y0); break;
						case SELECT_DISCRETE: SelectDiscrete(m_x0, m_y0); break;
						default:
							ev->accept();
							return ;
						};
					}
					else
					{
						if (meshMode == MESH_MODE_VOLUME)
						{
							if      (item == ITEM_ELEM) SelectFEElements(m_x0, m_y0);
							else if (item == ITEM_FACE) SelectFEFaces(m_x0, m_y0);
							else if (item == ITEM_EDGE) SelectFEEdges(m_x0, m_y0);
							else if (item == ITEM_NODE) SelectFENodes(m_x0, m_y0);
						}
						else
						{
							if      (item == ITEM_FACE) SelectSurfaceFaces(m_x0, m_y0);
							else if (item == ITEM_EDGE) SelectSurfaceEdges(m_x0, m_y0);
							else if (item == ITEM_NODE) SelectSurfaceNodes(m_x0, m_y0);
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
						case SELECT_OBJECT  : RegionSelectObjects (*preg); break;
						case SELECT_PART    : RegionSelectParts   (*preg); break;
						case SELECT_FACE    : RegionSelectSurfaces(*preg); break;
						case SELECT_EDGE    : RegionSelectEdges   (*preg); break;
						case SELECT_NODE    : RegionSelectNodes   (*preg); break;
						case SELECT_DISCRETE: RegionSelectDiscrete(*preg); break;
						default:
							ev->accept();
							return;
						};
					}
					else if (item == ITEM_ELEM) RegionSelectFEElems(*preg);
					else if (item == ITEM_FACE) RegionSelectFEFaces(*preg);
					else if (item == ITEM_EDGE) RegionSelectFEEdges(*preg);
					else if (item == ITEM_NODE) RegionSelectFENodes(*preg);

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
				QMenu menu(this);
				m_pWnd->BuildContextMenu(menu);
				menu.exec(ev->globalPos());
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
			if (!m_pmod)
			{
				cmd = new CCmdTranslateSelection(mdoc, m_rt);
			}
			else
			{
//				m_pmod->m_dist = m_rt.Length();
				m_pmod = 0;
			}
		}
		else if ((ntrans == TRANSFORM_ROTATE) && (but == Qt::LeftButton))
		{
			if (m_wt != 0)
			{
				quatd q;
				if (m_pivot == PIVOT_X) q = quatd(m_wt, vec3d(1,0,0));
				if (m_pivot == PIVOT_Y) q = quatd(m_wt, vec3d(0,1,0));
				if (m_pivot == PIVOT_Z) q = quatd(m_wt, vec3d(0,0,1));

				if (m_coord == COORD_LOCAL)
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
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	CGLCamera& cam = doc->GetView()->GetCamera();

    Qt::KeyboardModifiers key = ev->modifiers();
    bool balt   = (key & Qt::AltModifier);
	Qt::MouseEventSource eventSource = ev->source();
	if (eventSource == Qt::MouseEventSource::MouseEventNotSynthesized)
	{
		int y = ev->angleDelta().y();
		if (y > 0) cam.Zoom(0.95f);
		if (y < 0) cam.Zoom(1.0f / 0.95f);
		repaint();
		m_pWnd->UpdateGLControlBar();
	}
	else
	{
		if (balt) {
			if (m_pivot == PIVOT_NONE)
			{
				int y = ev->angleDelta().y();
				if (y > 0) cam.Zoom(0.95f);
				if (y < 0) cam.Zoom(1.0f / 0.95f);

				repaint();

				m_pWnd->UpdateGLControlBar();
			}
		}
		else {
			if (m_pivot == PIVOT_NONE)
			{
				int dx = ev->pixelDelta().x();
				int dy = ev->pixelDelta().y();
				vec3d r = vec3d(-dx, dy, 0.f);
				PanView(r);

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
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return true;

	CGLCamera& cam = doc->GetView()->GetCamera();

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
	GLfloat amb1[] = { .09f, .09f, .09f, 1.f };
	GLfloat dif1[] = { .8f, .8f, .8f, 1.f };

	//	GLfloat amb2[] = {.0f, .0f, .0f, 1.f};
	//	GLfloat dif2[] = {.3f, .3f, .4f, 1.f};

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glShadeModel(GL_FLAT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.5f);

	// enable lighting and set default options
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif1);

	glEnable(GL_POLYGON_OFFSET_FILL);

	//	glEnable(GL_LIGHT1);
	//	glLightfv(GL_LIGHT1, GL_AMBIENT, amb2);
	//	glLightfv(GL_LIGHT1, GL_DIFFUSE, dif2);

	// enable color tracking for diffuse color of materials
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glPolygonStipple(poly_mask);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glPointSize(7.0f);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	int Y = 0;
	m_Widget->AddWidget(m_ptitle = new GLBox(20, 20, 300, 50, ""), 0);
	m_ptitle->set_font_size(30);
	m_ptitle->fit_to_size();
	m_ptitle->set_label("$(filename)");
	Y += m_ptitle->h();

	m_Widget->AddWidget(m_psubtitle = new GLBox(Y, 70, 300, 60, ""), 0);
	m_psubtitle->set_font_size(15);
	m_psubtitle->fit_to_size();
	m_psubtitle->set_label("$(datafield)\\nTime = $(time)");

	m_Widget->AddWidget(m_ptriad = new GLTriad(0, 0, 150, 150), 0);
	m_ptriad->align(GLW_ALIGN_LEFT | GLW_ALIGN_BOTTOM);
	m_Widget->AddWidget(m_pframe = new GLSafeFrame(0, 0, 800, 600));
	m_pframe->align(GLW_ALIGN_HCENTER | GLW_ALIGN_VCENTER);
	m_pframe->hide();

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

//-----------------------------------------------------------------------------
void CGLView::UpdateWidgets(bool bposition)
{
	CPostDocument* postDoc = m_pWnd->GetPostDocument();

	if (postDoc && postDoc->IsValid())
	{
		m_ptitle->fit_to_size();

		int Y = 0;
		if (bposition)
			m_ptitle->resize(0, 0, m_ptitle->w(), m_ptitle->h());

		m_ptitle->fit_to_size();
		Y = m_ptitle->y() + m_ptitle->h();

		if (bposition)
			m_psubtitle->resize(0, Y, m_psubtitle->w(), m_psubtitle->h());

		m_psubtitle->fit_to_size();

		// set a min width for the subtitle otherwise the time values may get cropped
		if (m_psubtitle->w() < 150)
			m_psubtitle->resize(m_psubtitle->x(), m_psubtitle->y(), 150, m_psubtitle->h());

		repaint();
	}
}

//-----------------------------------------------------------------------------
bool CGLView::isTitleVisible() const
{
	return m_ptitle->visible();
}

void CGLView::showTitle(bool b)
{
	if (b) m_ptitle->show(); else m_ptitle->hide();
	repaint();
}

bool CGLView::isSubtitleVisible() const
{
	return m_psubtitle->visible();
}

void CGLView::showSubtitle(bool b)
{
	if (b) m_psubtitle->show(); else m_psubtitle->hide();
	repaint();
}

//-----------------------------------------------------------------------------
QImage CGLView::CaptureScreen()
{
	if (m_pframe && m_pframe->visible())
	{
		QImage im = grabFramebuffer();

		// crop based on the capture frame
		double dpr = m_pWnd->devicePixelRatio();
		return im.copy((int)(dpr*m_pframe->x()), (int)(dpr*m_pframe->y()), (int)(dpr*m_pframe->w()), (int)(dpr*m_pframe->h()));
	}
	else return grabFramebuffer();
}


bool CGLView::NewAnimation(const char* szfile, CAnimation* video, GLenum fmt)
{
	m_video = video;
	SetVideoFormat(fmt);

	// get the width/height of the animation
	int cx = width();
	int cy = height();
	if (m_pframe && m_pframe->visible())
	{
		int dpr = m_pWnd->devicePixelRatio();
		cx = dpr*m_pframe->w();
		cy = dpr*m_pframe->h();
	}

	// get the frame rate
	float fps = 10.f;
	if (m_pWnd->GetPostDocument()) fps = m_pWnd->GetPostDocument()->GetTimeSettings().m_fps;
	if (fps == 0.f) fps = 10.f;

	// create the animation
	if (m_video->Create(szfile, cx, cy, fps) == false)
	{
		delete m_video;
		m_video = nullptr;
		m_videoMode = VIDEO_STOPPED;
	}
	else
	{
		// lock the frame
		m_pframe->SetState(GLSafeFrame::FIXED_SIZE);

		// set the animation mode to paused
		m_videoMode = VIDEO_STOPPED;
	}

	return (m_video != 0);
}

bool CGLView::HasRecording() const
{
	return (m_video != 0);
}

VIDEO_MODE CGLView::RecordingMode() const
{
	return m_videoMode;
}

void CGLView::StartAnimation()
{
	if (m_video)
	{
		// set the animation mode to recording
		m_videoMode = VIDEO_RECORDING;

		// lock the frame
		m_pframe->SetState(GLSafeFrame::LOCKED);
		repaint();
	}
}

void CGLView::StopAnimation()
{
	if (m_video)
	{
		// stop the animation
		m_videoMode = VIDEO_STOPPED;

		// get the nr of frames before we close
		int nframes = m_video->Frames();

		// close the stream
		m_video->Close();

		// delete the object
		delete m_video;
		m_video = nullptr;

		// say something if frames is 0. 
		if (nframes == 0)
		{
			QMessageBox::warning(this, "FEBio Studio", "This animation contains no frames. Only an empty video file was saved.");
		}

		// unlock the frame
		m_pframe->SetState(GLSafeFrame::FREE);

		repaint();
	}
}

void CGLView::PauseAnimation()
{
	if (m_video)
	{
		// pause the recording
		m_videoMode = VIDEO_PAUSED;
		m_pframe->SetState(GLSafeFrame::FIXED_SIZE);
		repaint();
	}
}

//-----------------------------------------------------------------------------
void CGLView::repaintEvent()
{
	repaint();
}

void CGLView::paintGL()
{
	// Get the current document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr)
	{
		glClearColor(.2f, .2f, .2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		return;
	}

	VIEW_SETTINGS& view = GetViewSettings();

	int nitem = pdoc->GetItemMode();

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	CGLContext& rc = m_rc;
	rc.m_cam = &cam;
	rc.m_showOutline = view.m_bfeat;
	rc.m_showMesh = view.m_bmesh;
	rc.m_q = cam.GetOrientation();

	// prepare for rendering
	PrepModel();

	// render the backgound
	RenderBackground();

	// get the active view
	CPostDocument* postDoc = m_pWnd->GetPostDocument();
    if(postDoc) RenderPostView(postDoc);

    CModelDocument* mDoc = m_pWnd->GetModelDocument();
	if (mDoc) RenderModelView();

	// render the grid
	if (view.m_bgrid && (mDoc)) m_grid.Render(m_rc);

	// render the image data
	RenderImageData();

	// render the decorations
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

	// render the 3D cursor
	if (mDoc)
	{
		// render the highlights
		GLHighlighter::draw();

		if (m_bpick && (nitem == ITEM_MESH))
		{
			Render3DCursor(Get3DCursor(), 10.0);
		}

		// render the pivot
		RenderPivot();
	}

	// render the tooltip
	if (m_btooltip) RenderTooltip(m_xp, m_yp);

	// render selection
	if (m_bsel && (m_pivot == PIVOT_NONE)) RenderRubberBand();

	// show the labels on rigid bodies
	if (view.m_showRigidLabels) RenderRigidLabels();

	// set the projection Matrix to ortho2d so we can draw some stuff on the screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Update GLWidget string table for post rendering
	if (postDoc)
	{
		if (postDoc && postDoc->IsValid())
		{
			GLWidget::addToStringTable("$(filename)", postDoc->GetDocFileName());
			GLWidget::addToStringTable("$(datafield)", postDoc->GetFieldString());
			GLWidget::addToStringTable("$(time)", postDoc->GetTimeValue());
		}
	}

	// update the triad
	m_ptriad->setOrientation(cam.GetOrientation());

	// We must turn off culling before we use the QPainter, otherwise
	// drawing using QPainter doesn't work correctly.
	glDisable(GL_CULL_FACE);

	// render the GL widgets
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	if (postDoc == nullptr)
	{
		CModelDocument* mdoc = dynamic_cast<CModelDocument*>(pdoc);
		if (mdoc)
		{
			FSModel* ps = mdoc->GetFSModel();
			GModel& model = ps->GetModel();

			painter.setPen(QPen(QColor::fromRgb(164, 164, 164)));
			int activeLayer = model.GetActiveMeshLayer();
			const std::string& s = model.GetMeshLayerName(activeLayer);
			painter.drawText(0, 15, QString("  Mesh Layer > ") + QString::fromStdString(s));
			m_Widget->DrawWidget(m_ptriad, &painter);
			if (m_pframe->visible()) m_Widget->DrawWidget(m_pframe, &painter);
		}
	}
	else
	{
		if (postDoc->IsValid())
		{
			int layer = postDoc->GetGLModel()->m_layer;
			m_Widget->SetActiveLayer(layer);
			m_Widget->DrawWidgets(&painter);
		}
	}

	painter.end();

	if (m_videoMode != VIDEO_STOPPED)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		int x = width() - 200;
		int y = height() - 40;
		glPopAttrib();
	}

	if ((m_videoMode == VIDEO_RECORDING) && (m_video != 0))
	{
		glFlush();
		QImage im = CaptureScreen();
		if (m_video->Write(im) == false)
		{
			StopAnimation();
			QMessageBox::critical(this, "FEBio Studio", "An error occurred while writing frame to video stream.");
		}
	}

	if ((m_videoMode == VIDEO_PAUSED) && (m_video != 0))
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

	// if the camera is animating, we need to redraw
	if (cam.IsAnimating())
	{
		cam.Update();
		QTimer::singleShot(50, this, SLOT(repaintEvent()));
	}
}

//-----------------------------------------------------------------------------
void CGLView::RenderModelView()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	VIEW_SETTINGS& view = GetViewSettings();
	int nitem = pdoc->GetItemMode();

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	if (m_showPlaneCut)
	{
		if (m_planeCut == nullptr) UpdatePlaneCut();
		if (m_planeCutMode == 0)
		{
			// render the plane cut first
			if (m_planeCut) RenderPlaneCut();

			// then turn on the clipping plane before rendering the other geometry
			glClipPlane(GL_CLIP_PLANE0, m_plane);
			glEnable(GL_CLIP_PLANE0);
		}
	}

	// render the model
	if (pdoc->IsValid())
	{
		// render the (solid) model
		if ((view.m_nrender == RENDER_SOLID) || (nitem != ITEM_MESH)) RenderModel();

		// render discrete objects
		if (view.m_showDiscrete)
		{
			RenderDiscrete();
		}

		// render selected box
		RenderSelectionBox();

		cam.LineDrawMode(true);
		cam.Transform();

		// Render mesh lines
		//	if ((view.m_nrender == RENDER_SOLID) && (view.m_bmesh || (nitem != ITEM_MESH)))
		if (view.m_bmesh) RenderMeshLines();

		if (view.m_bfeat || (view.m_nrender == RENDER_WIREFRAME))
		{
			// don't draw feature edges in edge mode, since the edges are the feature edges
			// (Don't draw feature edges when we are rendering FE edges)
			int nselect = pdoc->GetSelectionMode();
			if (((nitem != ITEM_MESH) || (nselect != SELECT_EDGE)) && (nitem != ITEM_EDGE)) RenderFeatureEdges();
		}

		cam.LineDrawMode(false);
		cam.Transform();
	}

	// render the temp object
	CCreatePanel* cp = m_pWnd->GetCreatePanel();
	if (cp)
	{
		GObject* po = cp->GetTempObject();
		if (po)
		{
			RenderObject(po);
			RenderEdges(po);
		}
	}

	// render physics
	if (pdoc->IsValid())
	{
		if (view.m_brigid) RenderRigidBodies();
		if (view.m_bjoint) { RenderRigidJoints(); RenderRigidConnectors(); }
		if (view.m_bwall) RenderRigidWalls();
		if (view.m_bfiber) RenderMaterialFibers();
		if (view.m_blma) RenderLocalMaterialAxes();
	}

	// render the command window gizmo's
	/*	CCommandPanel* pcw = m_pWnd->GetCommandWindow()->GetActivePanel();
	if (pcw)
	{
	GLCanvas glc(this);
	pcw->Render(&glc);
	}
	*/
	// render the selected parts
	if (pdoc->IsValid())
	{
		GModel& model = *pdoc->GetGModel();
		int nsel = pdoc->GetSelectionMode();
		if (nitem == ITEM_MESH)
		{
			for (int i = 0; i<model.Objects(); ++i)
			{
				GObject* po = model.Object(i);
				if (po->IsVisible() && po->IsValid())
				{
					glPushMatrix();
					SetModelView(po);
					switch (nsel)
					{
					case SELECT_PART: RenderSelectedParts(po); break;
					case SELECT_FACE: RenderSelectedSurfaces(po); break;
					case SELECT_EDGE: RenderSelectedEdges(po); break;
					case SELECT_NODE: RenderSelectedNodes(po); break;
					}
					glPopMatrix();
				}
			}
		}
	}

	glDisable(GL_CLIP_PLANE0);

	// render the tags
	if (view.m_bTags) RenderTags();
}

//-----------------------------------------------------------------------------
void CGLView::RenderPostView(CPostDocument* postDoc)
{
	if (postDoc && postDoc->IsValid())
	{
		Post::CGLModel* glm = postDoc->GetGLModel();

		CGLCamera& cam = postDoc->GetView()->GetCamera();

		VIEW_SETTINGS& vs = GetViewSettings();

		glm->m_nrender = vs.m_nrender + 1;
		glm->m_bnorm = vs.m_bnorm;
		glm->m_scaleNormals = vs.m_scaleNormals;
		glm->m_brenderPlotObjects = vs.m_bjoint;
		glm->m_doZSorting = vs.m_bzsorting;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		PositionCamera();

		glDisable(GL_CULL_FACE);

		// match the selection mode
		int selectionMode = Post::SELECT_ELEMS;
		switch (GetDocument()->GetItemMode())
		{
		case ITEM_MESH:
		case ITEM_ELEM: selectionMode = Post::SELECT_ELEMS; break;
		case ITEM_FACE: selectionMode = Post::SELECT_FACES; break;
		case ITEM_EDGE: selectionMode = Post::SELECT_EDGES; break;
		case ITEM_NODE: selectionMode = Post::SELECT_NODES; break;
		}
		glm->SetSelectionMode(selectionMode);


		if (vs.m_bShadows)
		{
			BOX box = postDoc->GetBoundingBox();

			float a = vs.m_shadow_intensity;
			GLfloat shadow[] = { a, a, a, 1 };
			GLfloat zero[] = { 0, 0, 0, 1 };
			GLfloat ones[] = { 1,1,1,1 };
			GLfloat lp[4] = { 0 };

			glEnable(GL_STENCIL_TEST);

			float inf = box.Radius()*100.f;

			vec3d lpv = to_vec3d(GetLightPosition());

			quatd q = cam.GetOrientation();
			q.Inverse().RotateVector(lpv);

			lp[0] = lpv.x;
			lp[1] = lpv.y;
			lp[2] = lpv.z;

			// set coloring for shadows
			glLightfv(GL_LIGHT0, GL_DIFFUSE, shadow);
			glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

			glStencilFunc(GL_ALWAYS, 0x00, 0xff);
			glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

			// render the scene
			glm->Render(m_rc);

			// Create mask in stencil buffer
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

			Post::FEPostModel* fem = glm->GetFSModel();
			glm->RenderShadows(fem, lpv, inf);

			glCullFace(GL_BACK);
			glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);

			glm->RenderShadows(fem, lpv, inf);

			// Render the scene in light
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);

			GLfloat d = vs.m_diffuse;
			GLfloat dv[4] = { d, d, d, 1.f };

			glLightfv(GL_LIGHT0, GL_DIFFUSE, dv);
			glLightfv(GL_LIGHT0, GL_SPECULAR, ones);

			glStencilFunc(GL_EQUAL, 0, 0xff);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glDisable(GL_CULL_FACE);

			glClear(GL_DEPTH_BUFFER_BIT);
		}

		glm->Render(m_rc);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		// render the tracking
		if (m_btrack) RenderTrack();

		// render the tags
		VIEW_SETTINGS& view = GetViewSettings();
		if (view.m_bTags) RenderTags();
	}

	Post::CGLPlaneCutPlot::DisableClipPlanes();
}

//-----------------------------------------------------------------------------
void CGLView::Render3DCursor(const vec3d& r, double R)
{
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

//-----------------------------------------------------------------------------
// get device pixel ration
double CGLView::GetDevicePixelRatio() { return m_pWnd->devicePixelRatio(); }

QPoint CGLView::DeviceToPhysical(int x, int y)
{
	double dpr = m_pWnd->devicePixelRatio();
	return QPoint((int)(dpr*x), m_viewport[3] - (int)(dpr * y));
}

//-----------------------------------------------------------------------------
void CGLView::SetMatProps(GMaterial* pm)
{
	if (pm == 0) SetDefaultMatProps();
	else
	{
		GMaterial& m = *pm;
		GLfloat f = 1.f / 255.f;
		//			GLfloat dif[4] = {m.m_diffuse.r*f, m.m_diffuse.g*f, m.m_diffuse.b*f, 1.f}; 
		GLColor a = pm->Ambient();
		GLColor s = pm->Specular();
		GLColor e = pm->Emission();
		GLfloat amb[4] = { a.r*f, a.g*f, a.b*f, 1.f };
		GLfloat spc[4] = { s.r*f, s.g*f, s.b*f, 1.f };
		GLfloat emi[4] = { e.r*f, e.g*f, e.b*f, 1.f };
		//			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , dif);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128 * (GLfloat)m.m_shininess);
	}
}

//-----------------------------------------------------------------------------
void CGLView::SetDefaultMatProps()
{
	//		GLfloat dif[] = {0.8f, 0.8f, 0.8f, 1.f};
	GLfloat amb[] = { 0.8f, 0.8f, 0.8f, 1.f };
	GLfloat spc[] = { 0.0f, 0.0f, 0.0f, 1.f };
	GLfloat emi[] = { 0.0f, 0.0f, 0.0f, 1.f };

	//		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
}

//-----------------------------------------------------------------------------
// setup the projection matrix
void CGLView::SetupProjection()
{
	// set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	BOX box;

    CModelDocument* mdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if(mdoc)
    {
        box = mdoc->GetModelBox();
    }

	CPostDocument* postDoc = dynamic_cast<CPostDocument*>(GetDocument());
    if (postDoc && postDoc->IsValid())
	{
		box = postDoc->GetPostObject()->GetBoundingBox();
	}

	CGView& view = *doc->GetView();
	CGLCamera& cam = view.GetCamera();

	double R = box.Radius();
	VIEW_SETTINGS& vs = GetViewSettings();

	// NOTE: For some reason the camera position is coming back with the opposite sign
	//       so I need to add p and c, instead of subtract. Need to look into this closer. 
	vec3d p = cam.GlobalPosition();
	vec3d c = box.Center();
	double L = (c + p).Length();

	view.m_ffar = (L + R) * 2;
	view.m_fnear = 0.01f*view.m_ffar;

	double D = 0.5*cam.GetFinalTargetDistance();
	if ((D > 0) && (D < view.m_fnear)) view.m_fnear = D;

	if (height() == 0) view.m_ar = 1; view.m_ar = (GLfloat)width() / (GLfloat)height();

	// set up projection matrix
	if (view.m_bortho)
	{
		GLdouble f = 0.35*cam.GetTargetDistance();
		m_ox = f*view.m_ar;
		m_oy = f;
		glOrtho(-m_ox, m_ox, -m_oy, m_oy, view.m_fnear, view.m_ffar);
	}
	else
	{
		gluPerspective(view.m_fov, view.m_ar, view.m_fnear, view.m_ffar);
	}
}

//-----------------------------------------------------------------------------
inline vec3d mult_matrix(GLfloat m[4][4], vec3d r)
{
	vec3d a;
	a.x = m[0][0] * r.x + m[0][1] * r.y + m[0][2] * r.z;
	a.y = m[1][0] * r.x + m[1][1] * r.y + m[1][2] * r.z;
	a.z = m[2][0] * r.x + m[2][1] * r.y + m[2][2] * r.z;
	return a;
}

//-----------------------------------------------------------------------------
void CGLView::PositionCamera()
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	// position the camera
	CGLCamera& cam = doc->GetView()->GetCamera();
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void CGLView::PrepModel()
{
	GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };

	// store the viewport dimensions
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// setup projection
	SetupProjection();

	// reset the modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// clear the model
	glClearColor(.0f, .0f, .0f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// set material properties
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

	// set the diffuse lighting intensity
	CDocument* pdoc = GetDocument();
	if (pdoc && pdoc->IsValid())
	{
		VIEW_SETTINGS& view = GetViewSettings();

		// set the line width
		glLineWidth(view.m_line_size);

		// turn on/off lighting
		if (view.m_bLighting)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);

		GLfloat d = view.m_diffuse;
		GLfloat dv[4] = { d, d, d, 1.f };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dv);

		// set the ambient lighting intensity
		GLfloat f = view.m_ambient;
		GLfloat av[4] = { f, f, f, 1.f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, av);

		// position the light
		vec3f lp = GetLightPosition(); lp.Normalize();
		GLfloat fv[4] = { 0 };
		fv[0] = lp.x; fv[1] = lp.y; fv[2] = lp.z;
		glLightfv(GL_LIGHT0, GL_POSITION, fv);
	}

	// position the camera
	PositionCamera();
}

CGView* CGLView::GetView()
{
	CGLDocument* doc = GetDocument();
	if (doc) return doc->GetView();
	return nullptr;
}

CGLCamera* CGLView::GetCamera()
{
	CGLDocument* doc = GetDocument();
	if (doc) return &doc->GetView()->GetCamera();
	return nullptr;
}

void CGLView::ShowMeshData(bool b)
{
	GetViewSettings().m_bcontour = b;
	delete m_planeCut; m_planeCut = nullptr;
}

void CGLView::RenderTooltip(int x, int y)
{
/*	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	char sz[] = "Hello, world";

	gl_font(FL_HELVETICA, 12);

	int nw = (int)fl_width(sz) + 10;
	int nh = (int)fl_height() + 10;

	glColor3ub(255, 255, 128);
	gl_rectf(x, y, nw, nh);

	glColor3ub(0, 0, 0);
	gl_rect(x, y, nw, nh);


	gl_color(FL_BLACK);
	gl_draw("Hello, world", x, y, nw, nh, FL_ALIGN_CENTER);

	glPopAttrib();

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
*/
}

void CGLView::SetModelView(GObject* po)
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

void CGLView::RenderNormals(GObject* po, double scale)
{
	if (po->IsVisible() == false) return;

	FSMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return;

	double R = 0.05*pm->GetBoundingBox().GetMaxExtent()*scale;

	glPushAttrib(GL_LIGHTING);
	glDisable(GL_LIGHTING);

	int NS = po->Faces();
	vector<bool> vis(NS);
	for (int n = 0; n<NS; ++n)
	{
		GFace* gface = po->Face(n);
		vis[n] = po->IsFaceVisible(gface);
	}

	glBegin(GL_LINES);
	{
		int N = pm->Faces();
		for (int i = 0; i<N; ++i)
		{
			FSFace& face = pm->Face(i);
			bool bvis = ((face.m_gid >= 0) && (face.m_gid < NS) ? vis[face.m_gid] : true);
			if (face.IsVisible() && bvis)
			{
				vec3d fn = to_vec3d(face.m_fn);

				int n = face.Nodes();
				vec3d p = vec3d(0, 0, 0);
				for (int j = 0; j<n; ++j) p += pm->Node(face.n[j]).r;
				p /= (double)n;

				vec3d q = p + fn*R;

				float r = (float) fabs(fn.x);
				float g = (float) fabs(fn.y);
				float b = (float) fabs(fn.z);
	
				glx::drawLine_(p, q, GLColor::White(), GLColor::FromRGBf(r, g, b));
			}
		}
	}
	glEnd();
	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLView::RenderModel()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// Get the item mode
	int item = pdoc->GetItemMode();

	// get the mesh mode
	int meshMode = m_pWnd->GetMeshMode();

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();

	GObject* poa = pdoc->GetActiveObject();

	bool bnorm = view.m_bnorm;
	double scale = view.m_scaleNormals;

	// we don't use backface culling when drawing
	//	if (view.m_bcull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	if (item == ITEM_MESH)
	{
		for (int i = 0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible() && po->IsValid())
			{
				glPushMatrix();
				SetModelView(po);
				switch (nsel)
				{
				case SELECT_OBJECT:
				{
					if (view.m_bcontour && (poa == po) && po->GetFEMesh()) RenderFEElements(po);
					else if (m_showPlaneCut && (m_planeCutMode == 1))
					{
						RenderFEElements(po);

						GLColor c = view.m_mcol;
						glColor3ub(c.r, c.g, c.b);
						RenderMeshLines(po);
					}
					else RenderObject(po);
				}
				break;
				case SELECT_PART: RenderParts(po); break;
				case SELECT_FACE: RenderSurfaces(po); break;
				case SELECT_EDGE:
					{
						RenderObject(po);
						cam.LineDrawMode(true);
						cam.Transform();
						SetModelView(po);
						RenderEdges(po);
						cam.LineDrawMode(false);
						cam.Transform();
						SetModelView(po);
					}
					break;
				case SELECT_NODE:
					{
						RenderObject(po);
						cam.LineDrawMode(true);
						cam.Transform();
						SetModelView(po);
						RenderNodes(po);
						cam.LineDrawMode(false);
						cam.Transform();
						SetModelView(po);
					}
					break;
				case SELECT_DISCRETE:
				{
					RenderObject(po);
				}
				break;
				}
				if (bnorm) RenderNormals(po, scale);
				glPopMatrix();
			}
		}
	}
	else
	{
		for (int i = 0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible() && po->IsValid())
			{
				glPushMatrix();
				SetModelView(po);
				if (po == poa)
				{
					if (meshMode == MESH_MODE_VOLUME)
					{
						if (item == ITEM_ELEM)
						{
							RenderFEElements(po);
						}
						else if (item == ITEM_FACE)
						{
							RenderFEFaces(po);
						}
						else if (item == ITEM_EDGE)
						{
							RenderFEFaces(po);
							cam.LineDrawMode(true);
							cam.Transform();
							SetModelView(po);
							RenderFEEdges(po);
							cam.LineDrawMode(false);
							cam.Transform();
						}
						else if (item == ITEM_NODE)
						{
							RenderFEFaces(po);
							RenderFENodes(po);
						}
					}
					else
					{
						if (item == ITEM_FACE)
						{
							RenderSurfaceMeshFaces(po);
						}
						else if (item == ITEM_EDGE)
						{
							RenderSurfaceMeshFaces(po);
							cam.LineDrawMode(true);
							cam.Transform();
							SetModelView(po);
							RenderSurfaceMeshEdges(po);
							cam.LineDrawMode(false);
							cam.Transform();
						}
						else if (item == ITEM_NODE)
						{
							RenderSurfaceMeshFaces(po);
							RenderSurfaceMeshNodes(po);
						}
					}
				}
				else RenderObject(po);
				if (bnorm) RenderNormals(po, scale);
				glPopMatrix();
			}
		}
	}
}

void CGLView::RenderSelectionBox()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// Get the item mode
	int item = pdoc->GetItemMode();

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();

	GObject* poa = pdoc->GetActiveObject();

	bool bnorm = view.m_bnorm;
	double scale = view.m_scaleNormals;

	if (item == ITEM_MESH)
	{
		for (int i = 0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible())
			{
				glPushMatrix();
				SetModelView(po);

				if (nsel == SELECT_OBJECT)
				{
					glColor3ub(255, 255, 255);
					if (po->IsSelected())
					{
						RenderBox(po->GetLocalBox(), true, 1.025);
					}
				}
				else if (po == poa)
				{
					glColor3ub(164, 0, 164);
					assert(po->IsSelected());
					RenderBox(po->GetLocalBox(), true, 1.025);
				}
				glPopMatrix();
			}
		}
	}
	else if (poa)
	{
		glPushMatrix();
		SetModelView(poa);
		glColor3ub(255, 255, 0);
		RenderBox(poa->GetLocalBox(), true, 1.025);
		glPopMatrix();
	}
}

void RenderLine(GNode& n0, GNode& n1)
{
	vec3d r0 = n0.Position();
	vec3d r1 = n1.Position();

	glx::drawPoint(r0);
	glx::drawPoint(r1);

	glx::drawLine(r0, r1);
}

void CGLView::RenderDiscrete()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();
	bool bsel = (nsel == SELECT_DISCRETE);

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// render the discrete objects
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	int ND = model.DiscreteObjects();
	for (int i = 0; i<model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (po->IsVisible())
		{
			GLColor c = po->GetColor();

			if (bsel && po->IsSelected()) glColor3ub(255, 255, 0);
			else glColor3ub(c.r, c.g, c.b);

			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);
			if (ps)
			{
				GNode* pn0 = model.FindNode(ps->m_node[0]);
				GNode* pn1 = model.FindNode(ps->m_node[1]);
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}

			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(po);
			if (pg)
			{
				GNode* pn0 = model.FindNode(pg->m_node[0]);
				GNode* pn1 = model.FindNode(pg->m_node[1]);
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}

			GDiscreteElementSet* pd = dynamic_cast<GDiscreteElementSet*>(po);
			if (pd)
			{
				int N = pd->size();
				for (int n = 0; n < N; ++n)
				{
					GDiscreteElement& el = pd->element(n);

					if (bsel && el.IsSelected()) glColor3ub(255, 255, 0);
					else glColor3ub(c.r, c.g, c.b);

					GNode* pn0 = model.FindNode(el.Node(0));
					GNode* pn1 = model.FindNode(el.Node(1));
					if (pn0 && pn1) RenderLine(*pn0, *pn1);
				}
			}

			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(po);
			if (ds)
			{
				GNode* pn0 = model.FindNode(ds->NodeID(0));
				GNode* pn1 = model.FindNode(ds->NodeID(1));
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}
		}
	}
	glPopAttrib();
}

void CGLView::RenderBackground()
{
	// Get the document
	CDocument* pdoc = GetDocument();

	// set up the viewport
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	VIEW_SETTINGS& view = GetViewSettings();

	GLColor c[4];

	switch (view.m_nbgstyle)
	{
	case BG_COLOR1:
		c[0] = c[1] = c[2] = c[3] = view.m_col1; break;
	case BG_COLOR2:
		c[0] = c[1] = c[2] = c[3] = view.m_col2; break;
	case BG_HORIZONTAL:
		c[0] = c[1] = view.m_col2;
		c[2] = c[3] = view.m_col1;
		break;
	case BG_VERTICAL:
		c[0] = c[3] = view.m_col1;
		c[1] = c[2] = view.m_col2;
		break;
	}

	glBegin(GL_QUADS);
	{
		glColor3ub(c[0].r, c[0].g, c[0].b); glVertex2f(-1, -1);
		glColor3ub(c[1].r, c[1].g, c[1].b); glVertex2f(1, -1);
		glColor3ub(c[2].r, c[2].g, c[2].b); glVertex2f(1, 1);
		glColor3ub(c[3].r, c[3].g, c[3].b); glVertex2f(-1, 1);
	}
	glEnd();

	glPopAttrib();

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
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

void CGLView::RenderImageData()
{
	CGLDocument* doc = GetDocument();
	if (doc->IsValid() == false) return;

	CGLCamera& cam = doc->GetView()->GetCamera();

	VIEW_SETTINGS& vs = GetViewSettings();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	cam.Transform();

    if(doc->GetView()->imgView == CGView::MODEL_VIEW)
    {
        for (int i = 0; i < doc->ImageModels(); ++i)
        {
            Post::CImageModel* img = doc->GetImageModel(i);
            BOX box = img->GetBoundingBox();
    		// GLColor c = img->GetColor();
            GLColor c(255, 128, 128);
            glColor3ub(c.r, c.g, c.b);
            if (img->ShowBox()) RenderBox(box, false);
            img->Render(m_rc);
        }
    }
    else if(doc->GetView()->imgView == CGView::SLICE_VIEW)
    {
        CImageSliceView* sliceView = m_pWnd->GetImageSliceView();

        Post::CImageModel* img =  sliceView->GetImageModel();
        if(img)
        {
            BOX box = img->GetBoundingBox();
            GLColor c(255, 128, 128);
            glColor3ub(c.r, c.g, c.b);
            if (img->ShowBox()) RenderBox(box, false);
            img->Render(m_rc);

            sliceView->RenderSlicers(m_rc);
        }
    }

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void RenderFiber(GObject* po, FSMaterial* pmat, FEElementRef& rel, const vec3d& c, double h)
{
	if (pmat->HasFibers())
	{
		vec3d q = pmat->GetFiber(rel);

		// This vector is defined in global coordinates, except for user-defined fibers, which
		// are assumed to be in local coordinates
		FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(pmat);
		if (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
		{
			q = po->GetTransform().LocalToGlobalNormal(q);
		}

		double r = fabs(q.x);
		double g = fabs(q.y);
		double b = fabs(q.z);

		glColor3d(r, g, b);

		vec3d p0 = c - q * (h*0.5);
		vec3d p1 = c + q * (h*0.5);
		glVertex3d(p0.x, p0.y, p0.z);
		glVertex3d(p1.x, p1.y, p1.z);
	}

	for (int i = 0; i < pmat->Properties(); ++i)
	{
		FSProperty& prop = pmat->GetProperty(i);
		for (int j = 0; j < prop.Size(); ++j)
		{
			FSMaterial* matj = pmat->GetMaterialProperty(i, j);
			if (matj) RenderFiber(po, matj, rel, c, h);
		}
	}
}

void CGLView::RenderMaterialFibers()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	FEElementRef rel;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	BOX box = model.GetBoundingBox();
	double h = 0.05*box.GetMaxExtent()*view.m_fiber_scale;

	glBegin(GL_LINES);
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				rel.m_pmesh = pm;
				for (int j = 0; j<pm->Elements(); ++j)
				{
					FSElement& el = pm->Element(j);
					GPart* pg = po->Part(el.m_gid);

					bool showFiber = (pg->IsVisible() && el.IsVisible()) || view.m_showHiddenFibers;

					if (showFiber)
					{
						GMaterial* pgm = ps->GetMaterialFromID(po->Part(el.m_gid)->GetMaterialID());
						FSMaterial* pmat = 0;
						if (pgm) pmat = pgm->GetMaterialProperties();

						rel.m_nelem = j;
						if (pmat)
						{
							// element center
							vec3d c(0, 0, 0);
							for (int k = 0; k < el.Nodes(); ++k) c += pm->Node(el.m_node[k]).r;
							c /= el.Nodes();

							// to global coordinates
							c = po->GetTransform().LocalToGlobal(c);

							// render the fiber
							RenderFiber(po, pmat, rel, c, h);
						}
					}
				}
			}
		}
	}
	glEnd(); // GL_LINES

	glPopAttrib();
}

void CGLView::RenderLocalMaterialAxes()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	FEElementRef rel;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	VIEW_SETTINGS& view = GetViewSettings();
	BOX box = model.GetBoundingBox();
	double h = 0.05*box.GetMaxExtent()*view.m_fiber_scale;

	double rgb[3][3] = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };

	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				Transform& T = po->GetTransform();
				rel.m_pmesh = pm;
				for (int j = 0; j<pm->Elements(); ++j)
				{
					FSElement& el = pm->Element(j);

					GPart* pg = po->Part(el.m_gid);

					bool showAxes = (pg->IsVisible() && el.IsVisible()) || view.m_showHiddenFibers;

					if (showAxes)
					{
						GMaterial* pgm = ps->GetMaterialFromID(po->Part(el.m_gid)->GetMaterialID());
						FSMaterial* pmat = 0;
						if (pgm) pmat = pgm->GetMaterialProperties();

						rel.m_nelem = j;
						if (el.m_Qactive)
						{
							vec3d c(0, 0, 0);
							for (int k = 0; k<el.Nodes(); ++k) c += pm->NodePosition(el.m_node[k]);
							c /= el.Nodes();

							mat3d Q = el.m_Q;
							vec3d q;
							for (int k = 0; k<3; ++k) {
								q = vec3d(Q[0][k], Q[1][k], Q[2][k]);

								q = T.LocalToGlobalNormal(q);

								glColor3ub(rgb[0][k], rgb[1][k], rgb[2][k]);

								glx::drawLine(c, c + q*h);
							}
						}
						else if (pmat)
						{
							vec3d c(0, 0, 0);
							for (int k = 0; k<el.Nodes(); ++k) c += pm->NodePosition(el.m_node[k]);
							c /= el.Nodes();

							mat3d Q = pmat->GetMatAxes(rel);
							vec3d q;
							for (int k = 0; k<3; ++k) {
								q = vec3d(Q[0][k], Q[1][k], Q[2][k]);

								glColor3ub(rgb[0][k], rgb[1][k], rgb[2][k]);

								glx::drawLine(c, c + q*h);
							}
						}
					}
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This function renders the manipulator at the current pivot
//
void CGLView::RenderPivot(bool bpick)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();

	// make there is something selected
	if (!ps || (ps->Size() == 0)) return;

	// get the global position of the pivot
	// this is where we place the manipulator
	vec3d rp = GetPivotPosition();

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	// determine the scale of the manipulator
	// we make it depend on the target distanceso that the 
	// manipulator will look about the same size regardless the zoom
	double d = 0.1*cam.GetTargetDistance();

	// push the modelview matrix
	glPushMatrix();

	// position the manipulator
	glTranslatef((float)rp.x, (float)rp.y, (float)rp.z);

	// orient the manipulator
	if (m_coord == COORD_LOCAL)
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
	switch (ntrans)
	{
	case TRANSFORM_MOVE  : m_Ttor.SetScale(d); m_Ttor.Render(m_pivot, bact); break;
	case TRANSFORM_ROTATE: m_Rtor.SetScale(d); m_Rtor.Render(m_pivot, bact); break;
	case TRANSFORM_SCALE : m_Stor.SetScale(d); m_Stor.Render(m_pivot, bact); break;
	}

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


void CGLView::RenderRigidWalls()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	FSModel* ps = pdoc->GetFSModel();
	BOX box = ps->GetModel().GetBoundingBox();
	double R = box.GetMaxExtent();
	vec3d c = box.Center();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	for (int n = 0; n<ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i<s.Interfaces(); ++i)
		{
			FSRigidWallInterface* pi = dynamic_cast<FSRigidWallInterface*>(s.Interface(i));
			if (pi)
			{
				// get the plane equation
				double a[4];
				pi->GetPlaneEquation(a);
				vec3d n(a[0], a[1], a[2]);
				double D = a[3];
				vec3d r0 = n*(D / (n*n));

				// project the center of the box onto the plane
				n.Normalize();
				vec3d p = c - n*(n*(c - r0));

				quatd q(vec3d(0, 0, 1), n);
				glPushMatrix();
				{
					glTranslated(p.x, p.y, p.z);
					glx::rotate(q);
					glColor4ub(128, 96, 0, 96);
					glRectd(-R, -R, R, R);

					glColor3ub(164, 128, 0);
					glBegin(GL_LINE_LOOP);
					{
						glVertex3d(-R, -R, 0);
						glVertex3d(R, -R, 0);
						glVertex3d(R, R, 0);
						glVertex3d(-R, R, 0);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(0, 0, 0); glVertex3d(0, 0, R / 2);
						glVertex3d(0, 0, R / 2); glVertex3d(-R*0.1, 0, R*0.4);
						glVertex3d(0, 0, R / 2); glVertex3d(R*0.1, 0, R*0.4);
					}
					glEnd();
				}
				glPopMatrix();
			}
			FSRigidSphereInterface* prs = dynamic_cast<FSRigidSphereInterface*>(s.Interface(i));
			if (prs)
			{
				vec3d c = prs->Center();
				double R = prs->Radius();

				GLUquadricObj* pobj = gluNewQuadric();

				glColor4ub(128, 96, 0, 96);
				glPushMatrix();
				{
					glTranslated(c.x, c.y, c.z);
					gluSphere(pobj, R, 32, 32);
				}
				glPopMatrix();

				gluDeleteQuadric(pobj);
			}
		}
	}

	glPopAttrib();
}

void CGLView::RenderRigidJoints()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	FSModel* ps = pdoc->GetFSModel();

	double scale = 0.05*(double)cam.GetTargetDistance();
	double R = 0.5*scale;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255, 0, 0);

	for (int n = 0; n<ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i<s.Interfaces(); ++i)
		{
			FSRigidJoint* pj = dynamic_cast<FSRigidJoint*> (s.Interface(i));
			if (pj)
			{
				vec3d r = pj->GetVecValue(FSRigidJoint::RJ);

				glColor3ub(255, 0, 0);
				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glx::renderJoint(R);
				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}

void CGLView::RenderRigidConnectors()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	FSModel* ps = pdoc->GetFSModel();

	double scale = 0.05*(double)cam.GetTargetDistance();
	double R = 0.5*scale;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(0, 0, 255);

	for (int n = 0; n<ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i<s.RigidConnectors(); ++i)
		{
			FSRigidConnector* rci = s.RigidConnector(i);
			if (dynamic_cast<FSRigidSphericalJoint*> (rci))
			{
				FSRigidSphericalJoint* pj = dynamic_cast<FSRigidSphericalJoint*> (rci);
				vec3d r = pj->GetVecValue(FSRigidSphericalJoint::J_ORIG);

				glColor3ub(255, 0, 0);
				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glx::renderJoint(R);
				glPopMatrix();
			}
			else if (dynamic_cast<FSRigidRevoluteJoint*> (rci))
			{
				FSRigidRevoluteJoint* pj = dynamic_cast<FSRigidRevoluteJoint*> (rci);
				vec3d r = pj->GetVecValue(FSRigidRevoluteJoint::J_ORIG);
				vec3d c = pj->GetVecValue(FSRigidRevoluteJoint::J_AXIS); c.Normalize();
				vec3d a = pj->GetVecValue(FSRigidRevoluteJoint::T_AXIS); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				glColor3ub(0, 0, 255);
				glx::renderRevoluteJoint(R);

				glPopMatrix();
			}
			else if (dynamic_cast<FSRigidPrismaticJoint*> (rci))
			{
				FSRigidPrismaticJoint* pj = dynamic_cast<FSRigidPrismaticJoint*> (rci);
				vec3d r = pj->GetVecValue(FSRigidPrismaticJoint::J_ORIG);
				vec3d a = pj->GetVecValue(FSRigidPrismaticJoint::J_AXIS); a.Normalize();
				vec3d b = pj->GetVecValue(FSRigidPrismaticJoint::T_AXIS); b.Normalize();
				vec3d c = a ^ b; c.Normalize();
				b = c ^ a; b.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				glColor3ub(0, 255, 0);
				glx::renderPrismaticJoint(R);

				glPopMatrix();
			}
			else if (dynamic_cast<FSRigidCylindricalJoint*> (rci))
			{
				FSRigidCylindricalJoint* pj = dynamic_cast<FSRigidCylindricalJoint*> (rci);
				vec3d r = pj->GetVecValue(FSRigidCylindricalJoint::J_ORIG);
				vec3d c = pj->GetVecValue(FSRigidCylindricalJoint::J_AXIS); c.Normalize();
				vec3d a = pj->GetVecValue(FSRigidCylindricalJoint::T_AXIS); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				glColor3ub(255, 0, 255);
				glx::renderCylindricalJoint(R);

				glPopMatrix();
			}
			else if (dynamic_cast<FSRigidPlanarJoint*> (rci))
			{
				FSRigidPlanarJoint* pj = dynamic_cast<FSRigidPlanarJoint*> (rci);
				vec3d r = pj->GetVecValue(FSRigidPlanarJoint::J_ORIG);
				vec3d c = pj->GetVecValue(FSRigidPlanarJoint::J_AXIS); c.Normalize();
				vec3d a = pj->GetVecValue(FSRigidPlanarJoint::T_AXIS); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				glColor3ub(0, 255, 255);
				glx::renderPlanarJoint(R);

				glPopMatrix();
			}
            else if (dynamic_cast<FSRigidLock*> (rci))
            {
                FSRigidLock* pj = dynamic_cast<FSRigidLock*> (rci);
                vec3d r = pj->GetVecValue(FSRigidLock::J_ORIG);
                vec3d c = pj->GetVecValue(FSRigidLock::J_AXIS); c.Normalize();
                vec3d a = pj->GetVecValue(FSRigidLock::T_AXIS); a.Normalize();
                vec3d b = c ^ a; b.Normalize();
                a = b ^ c; a.Normalize();
                GLfloat Q4[16] = {
                    (GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
                    (GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
                    (GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
                    0.f, 0.f, 0.f, 1.f };
                
                glPushMatrix();
                glTranslatef((float)r.x, (float)r.y, (float)r.z);
                glMultMatrixf(Q4);
                
				glColor3ub(255, 127, 0);

				glx::renderRigidLock(R);
                
                glPopMatrix();
            }
			else if (dynamic_cast<FSRigidSpring*> (rci))
			{
				FSRigidSpring* pj = dynamic_cast<FSRigidSpring*> (rci);
				vec3d xa = pj->GetVecValue(FSRigidSpring::XA);
				vec3d xb = pj->GetVecValue(FSRigidSpring::XB);

				glPushMatrix();
				glColor3ub(255, 0, 0);
				glx::renderSpring(xa, xb, R);
				glPopMatrix();
			}
			else if (dynamic_cast<FSRigidDamper*> (rci))
			{
				FSRigidDamper* pj = dynamic_cast<FSRigidDamper*> (rci);
				vec3d xa = pj->GetVecValue(FSRigidDamper::XA);
				vec3d xb = pj->GetVecValue(FSRigidDamper::XB);

				glPushMatrix();

				glColor3ub(255, 0, 0);
				glx::renderDamper(xa, xb, R);

				glPopMatrix();
			}
			else if (dynamic_cast<FSRigidContractileForce*> (rci))
			{
				FSRigidContractileForce* pj = dynamic_cast<FSRigidContractileForce*> (rci);
				vec3d xa = pj->GetVecValue(FSRigidContractileForce::XA);
				vec3d xb = pj->GetVecValue(FSRigidContractileForce::XB);

				glPushMatrix();

				glColor3ub(255, 0, 0);
				glx::renderContractileForce(xa, xb, R);

				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}


void CGLView::RenderRigidBodies()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	CGLCamera& cam = pdoc->GetView()->GetCamera();

	FSModel* ps = pdoc->GetFSModel();

	double scale = 0.03*(double)cam.GetTargetDistance();
	double R = 0.5*scale;

	quatd qi = cam.GetOrientation().Inverse();

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i<ps->Materials(); ++i)
	{
		GMaterial* pm = ps->GetMaterial(i);
		FSRigidMaterial* pb = dynamic_cast<FSRigidMaterial*> (pm->GetMaterialProperties());
		if (pb)
		{
			GLColor c = pm->Diffuse();

			glColor3ub(c.r, c.g, c.b);

			vec3d r = pb->GetVecValue(FSRigidMaterial::MP_RC);

			glPushMatrix();
			glTranslatef((float)r.x, (float)r.y, (float)r.z);

			glx::renderRigidBody(R);

			glPopMatrix();

			// get the parent
			if (pb->m_pid != -1)
			{
				FSRigidMaterial* pp = dynamic_cast<FSRigidMaterial*>(ps->GetMaterialFromID(pb->m_pid)->GetMaterialProperties());
				assert(pp);

				glColor3ub(50, 50, 255);
				vec3d r0 = pb->GetVecValue(FSRigidMaterial::MP_RC);
				vec3d r1 = pp->GetVecValue(FSRigidMaterial::MP_RC);

				double l = (r1 - r0).Length();
				vec3d el = r0 - r1; el.Normalize();

				quatd q(vec3d(0, 0, 1), el);
				glPushMatrix();
				{
					glTranslated(r1.x, r1.y, r1.z);
					glx::rotate(q);

					vec3d e2 = q*vec3d(0, 0, 1);

					double a = l*0.25;
					double b = a*0.25;
					glBegin(GL_LINES);
					{
						glVertex3d(0, 0, 0); glVertex3d(b, b, a); glVertex3d(b, b, a); glVertex3d(0, 0, l);
						glVertex3d(0, 0, 0); glVertex3d(-b, b, a); glVertex3d(-b, b, a); glVertex3d(0, 0, l);
						glVertex3d(0, 0, 0); glVertex3d(-b, -b, a); glVertex3d(-b, -b, a); glVertex3d(0, 0, l);
						glVertex3d(0, 0, 0); glVertex3d(b, -b, a); glVertex3d(b, -b, a); glVertex3d(0, 0, l);
						glVertex3d(b, b, a); glVertex3d(-b, b, a);
						glVertex3d(-b, b, a); glVertex3d(-b, -b, a);
						glVertex3d(-b, -b, a); glVertex3d(b, -b, a);
						glVertex3d(b, -b, a); glVertex3d(b, b, a);
					}
					glEnd();
				}
				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}

void CGLView::ScreenToView(int x, int y, double& fx, double& fy)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	double W = (double)width();
	double H = (double)height();

	if (H == 0.f) H = 0.001f;

	CGView& view = *doc->GetView();

	double ar = W / H;

	double fh = 2.f*view.m_fnear*(double)tan(0.5*view.m_fov*PI / 180);
	double fw = fh * ar;

	fx = -fw / 2 + x*fw / W;
	fy = fh / 2 - y*fh / H;
}

vec3d CGLView::WorldToPlane(vec3d r)
{
	return m_grid.m_q.Inverse()*(r - m_grid.m_o);
}

void CGLView::showSafeFrame(bool b)
{
	if (b) m_pframe->show();
	else m_pframe->hide();
}

void CGLView::SetViewMode(View_Mode n)
{
    // Get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

    VIEW_SETTINGS& view = GetViewSettings();
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
            m_grid.m_q = q;
            
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
            m_grid.m_q = q;
            
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
            m_grid.m_q = q;
            
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

	pdoc->GetView()->GetCamera().SetOrientation(q);

	// set the camera target
	//	m_Cam.SetTarget(vec3d(0,0,0));

	repaint();
}

void CGLView::TogglePerspective(bool b)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	CGView& view = *doc->GetView();
	view.m_bortho = b;
	repaint();
}

void CGLView::ToggleDisplayNormals()
{
	VIEW_SETTINGS& view = GetViewSettings();
	view.m_bnorm = !view.m_bnorm;
	repaint();
}

//-----------------------------------------------------------------------------
void CGLView::AddRegionPoint(int x, int y)
{
	if (m_pl.empty()) m_pl.push_back(pair<int, int>(x, y));
	else
	{
		pair<int, int>& p = m_pl[m_pl.size() - 1];
		if ((p.first != x) || (p.second != y)) m_pl.push_back(pair<int, int>(x, y));
	}
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void CGLView::ShowPlaneCut(bool b)
{
	m_showPlaneCut = b;
	UpdatePlaneCut(true);
	update();
}

//-----------------------------------------------------------------------------
bool CGLView::ShowPlaneCut() const
{
	return m_showPlaneCut;
}

//-----------------------------------------------------------------------------
void CGLView::SetPlaneCutMode(int nmode)
{
	m_planeCutMode = nmode;
	UpdatePlaneCut(true);
	update();
}

//-----------------------------------------------------------------------------
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

	m_plane[0] = d[0];
	m_plane[1] = d[1];
	m_plane[2] = d[2];
	m_plane[3] = -d3;
	delete m_planeCut; m_planeCut = nullptr;
	update();
}

//-----------------------------------------------------------------------------
void CGLView::PanView(vec3d r)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	CGLCamera& cam = doc->GetView()->GetCamera();

	double f = 0.001f*(double)cam.GetFinalTargetDistance();
	r.x *= f;
	r.y *= f;

	cam.Truck(r);
}


//-----------------------------------------------------------------------------
// Select an arm of the pivot manipulator
bool CGLView::SelectPivot(int x, int y)
{
	// store the old pivot mode
	int old_mode = m_pivot;

	// get the transformation mode
	int ntrans = GetDocument()->GetTransformMode();

	makeCurrent();

	// get a new pivot mode
	switch (ntrans)
	{
	case TRANSFORM_MOVE  : m_pivot = m_Ttor.Pick(x, y); break;
	case TRANSFORM_ROTATE: m_pivot = m_Rtor.Pick(x, y); break;
	case TRANSFORM_SCALE : m_pivot = m_Stor.Pick(x, y); break;
	}
	return (m_pivot != old_mode);
}

//-----------------------------------------------------------------------------
bool IntersectObject(GObject* po, const Ray& ray, Intersection& q)
{
	GLMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return false;

	Intersection qtmp;
	double distance = 0.0, minDist = 1e34;
	int NF = mesh->Faces();
	bool intersect = false;
	for (int j = 0; j<NF; ++j)
	{
		GMesh::FACE& face = mesh->Face(j);

		if (po->Face(face.pid)->IsVisible())
		{
			vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
			vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
			vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

			Triangle tri = { r0, r1, r2 };
			if (IntersectTriangle(ray, tri, qtmp))
			{
				double distance = ray.direction*(qtmp.point - ray.origin);
				if ((distance >= 0.0) && (distance < minDist))
				{
					minDist = distance;
					q = qtmp;
					intersect = true;
				}
			}
		}
	}

	return intersect;
}

//-----------------------------------------------------------------------------
// Select Objects
void CGLView::SelectObjects(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	makeCurrent();

	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// convert the point to a ray
	GLViewTransform transform(this);
	Ray ray = transform.PointToRay(x, y);

	GObject* closestObject = 0;
	Intersection q;
	double minDist = 0;
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && IntersectObject(po, ray, q))
		{
			double distance = ray.direction*(q.point - ray.origin);
			if ((closestObject == 0) || ((distance >= 0.0) && (distance < minDist)))
			{
				closestObject = po;
				minDist = distance;
			}
		}
	}

	// parse the selection buffer
	CCommand* pcmd = 0;
	string objName;
	if (closestObject != 0)
	{
		if (m_bctrl) pcmd = new CCmdUnselectObject(&model, closestObject);
		else pcmd = new CCmdSelectObject(&model, closestObject, m_bshift);
		objName = closestObject->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) 
	{
		// this clears the selection, but we only do this when there is an object currently selected
		FESelection* sel = pdoc->GetCurrentSelection();
		if (sel && sel->Size()) pcmd = new CCmdSelectObject(&model, 0, false);
		objName = "<Empty>";
	}

	// (un)select the mesh(es)
	if (pcmd) pdoc->DoCommand(pcmd, objName);
}

//-----------------------------------------------------------------------------
// Select parts
void CGLView::SelectParts(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	if (model.Parts() == 0) return;

	// convert the point to a ray
	makeCurrent();
	GLViewTransform transform(this);
	Ray ray = transform.PointToRay(x, y);

	GPart* closestPart = 0;
	Intersection q;
	double minDist = 0;
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			GLMesh* mesh = po->GetRenderMesh();
			if (mesh)
			{
				int NF = mesh->Faces();
				for (int j = 0; j<NF; ++j)
				{
					GMesh::FACE& face = mesh->Face(j);

					vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
					vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
					vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

					Triangle tri = { r0, r1, r2 };
					if (IntersectTriangle(ray, tri, q))
					{
						double distance = ray.direction*(q.point - ray.origin);
						if ((closestPart == 0) || ((distance >= 0.0) && (distance < minDist)))
						{
							GFace* gface = po->Face(face.pid);
							int pid = gface->m_nPID[0];
							GPart* part = po->Part(pid);
							if (part->IsVisible() && ((part->IsSelected() == false) || (m_bctrl)))
							{
								closestPart = part;
								minDist = distance;
							}
							else if (gface->m_nPID[1] >= 0)
							{
								pid = gface->m_nPID[1];
								part = po->Part(pid);
								if (part->IsVisible() && ((part->IsSelected() == false) || (m_bctrl)))
								{
									closestPart = part;
									minDist = distance;
								}
							}
							else if (gface->m_nPID[2] >= 0)
							{
								pid = gface->m_nPID[2];
								part = po->Part(pid);
								if (part->IsVisible() && ((part->IsSelected() == false) || (m_bctrl)))
								{
									closestPart = part;
									minDist = distance;
								}
							}
						}
					}
				}
			}
		}
	}
	CCommand* pcmd = 0;
	string partName;
	if (closestPart != 0)
	{
		int index = closestPart->GetID();
		if (m_bctrl) pcmd = new CCmdUnSelectPart(&model, &index, 1);
		else pcmd = new CCmdSelectPart(&model, &index, 1, m_bshift);
		partName = closestPart->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false))
	{
		pcmd = new CCmdSelectPart(&model, 0, 0, false);
		partName = "<Empty>";
	}

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd, partName);
}

//-----------------------------------------------------------------------------
// select faces
void CGLView::SelectSurfaces(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	if (model.Surfaces() == 0) return;

	// convert the point to a ray
	makeCurrent();
	GLViewTransform transform(this);
	Ray ray = transform.PointToRay(x, y);

	GFace* closestSurface = 0;
	Intersection q;
	double minDist = 0;
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			GLMesh* mesh = po->GetRenderMesh();
			if (mesh)
			{
				int NF = mesh->Faces();
				for (int j=0; j<NF; ++j)
				{
					GMesh::FACE& face = mesh->Face(j);
					GFace* gface = po->Face(face.pid);
					if (po->IsFaceVisible(gface))
					{
						// NOTE: Note sure why I have a scale factor here. It was originally to 0.99, but I
						//       had to increase it. I suspect it is to overcome some z-fighting for overlapping surfaces, but not sure. 
						vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r*0.99999);
						vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r*0.99999);
						vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r*0.99999);

						Triangle tri = {r0, r1, r2};
						if (IntersectTriangle(ray, tri, q))
						{
							double distance = ray.direction*(q.point - ray.origin);
							if ((closestSurface == 0) || ((distance >= 0.0) && (distance < minDist)))
							{
								if ((gface->IsSelected() == false) || (m_bctrl))
								{
									closestSurface = po->Face(face.pid);
									minDist = distance;
								}
							}					
						}
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	string surfName = "<Empty>";
	if (closestSurface != 0)
	{
		int index = closestSurface->GetID();
		if (m_bctrl) pcmd = new CCmdUnSelectSurface(&model, &index, 1);
		else pcmd = new CCmdSelectSurface(&model, &index, 1, m_bshift);
		surfName = ps->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectSurface(&model, 0, 0, false);

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd, surfName);
}

//-----------------------------------------------------------------------------
// select edges
void CGLView::SelectEdges(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int NE = model.Edges();
	if (NE == 0) return;

	makeCurrent();
	GLViewTransform transform(this);

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	int Objects = model.Objects();
	GEdge* closestEdge = 0;
	double zmin = 0.0;
	for (int i=0; i<Objects; ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			GLMesh* mesh = po->GetRenderMesh(); assert(mesh);
			if (mesh)
			{
				int edges = mesh->Edges();
				for (int j=0; j<edges; ++j)
				{
					GMesh::EDGE& edge = mesh->Edge(j);

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

	CCommand* pcmd = 0;
	string edgeName = "<Empty>";
	if (closestEdge != 0)
	{
		int index = closestEdge->GetID();
		if (m_bctrl) pcmd = new CCmdUnSelectEdge(&model, &index, 1);
		else pcmd = new CCmdSelectEdge(&model, &index, 1, m_bshift);
		edgeName = ps->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectEdge(&model, 0, 0, false);

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd, edgeName);
}

//-----------------------------------------------------------------------------
// highlight edges
void CGLView::HighlightEdge(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

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
			GLMesh* mesh = po->GetRenderMesh(); assert(mesh);
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
// select nodes
void CGLView::SelectNodes(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	makeCurrent();
	GLViewTransform transform(this);

	int NN = model.Nodes();
	if (NN == 0) return;
	GNode* closestNode = 0;
	double zmin = 0.0;
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			int nodes = po->Nodes();
			for (int j=0; j<nodes; ++j)
			{
				GNode& node = *po->Node(j);

				// don't select shape nodes
				if (node.Type() != NODE_SHAPE)
				{
					vec3d r = node.Position();

					vec3d p = transform.WorldToScreen(r);

					if (rt.contains(QPoint((int)p.x, (int)p.y)))
					{
						if ((closestNode == 0) || (p.z < zmin))
						{
							closestNode = &node;
							zmin = p.z;
						}
					}
				}
			}
		}
	}
	 
	CCommand* pcmd = 0;
	string nodeName = "<Empty>";
	if (closestNode != 0)
	{
		int index = closestNode->GetID();
		assert(closestNode->Type() != NODE_SHAPE);
		if (m_bctrl) pcmd = new CCmdUnSelectNode(&model, &index, 1);
		else pcmd = new CCmdSelectNode(&model, &index, 1, m_bshift);
		nodeName = ps->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectNode(&model, 0, 0, false);

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd, nodeName);
}

//-----------------------------------------------------------------------------
// select nodes
void CGLView::SelectDiscrete(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int ND = model.DiscreteObjects();
	if (ND == 0) return;


	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	makeCurrent();
	GLViewTransform transform(this);

	int index = -1;
	int comp = -1;
	float zmin = 0.f;
	for (int i = 0; i<ND; ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);

		if (dynamic_cast<GLinearSpring*>(po))
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);
			GNode* node0 = model.FindNode(ps->m_node[0]);
			GNode* node1 = model.FindNode(ps->m_node[1]);
			if (node0 && node1)
			{
				vec3d r0 = node0->Position();
				vec3d r1 = node1->Position();

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
				{
					if ((index == -1) || (p0.z < zmin))
					{
						index = i;
						zmin = p0.z;
					}
				}
			}
		}
		else if (dynamic_cast<GGeneralSpring*>(po))
		{
			GGeneralSpring* ps = dynamic_cast<GGeneralSpring*>(po);
			GNode* node0 = model.FindNode(ps->m_node[0]);
			GNode* node1 = model.FindNode(ps->m_node[1]);
			if (node0 && node1)
			{
				vec3d r0 = node0->Position();
				vec3d r1 = node1->Position();

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
				{
					if ((index == -1) || (p0.z < zmin))
					{
						index = i;
						zmin = p0.z;
					}
				}
			}
		}
		else if (dynamic_cast<GDiscreteElementSet*>(po))
		{
			GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(po);
			int NE = ps->size();
			for (int j=0; j<NE; ++j)
			{
				GDiscreteElement& el = ps->element(j);

				GNode* node0 = model.FindNode(el.Node(0));
				GNode* node1 = model.FindNode(el.Node(1));
				if (node0 && node1)
				{
					vec3d r0 = node0->Position();
					vec3d r1 = node1->Position();

					vec3d p0 = transform.WorldToScreen(r0);
					vec3d p1 = transform.WorldToScreen(r1);

					if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
					{
						if ((index == -1) || (p0.z < zmin))
						{
							index = i;
							zmin = p0.z;
							comp = j;
						}
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(model.DiscreteObject(index));
		if (pds)
		{
			// TODO: Turn this into a command
			if (m_bctrl) pds->UnselectComponent(comp);
			else 
			{
				if (m_bshift == false) pds->UnSelect();
				pds->SelectComponent(comp);
			}
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnSelectDiscrete(&model, &index, 1);
			else pcmd = new CCmdSelectDiscrete(&model, &index, 1, m_bshift);
		}
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectDiscrete(&model, 0, 0, false);

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
GObject* CGLView::GetActiveObject()
{
	return m_pWnd->GetActiveObject();
}

//-----------------------------------------------------------------------------
void CGLView::SelectFEElements(int x, int y)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	VIEW_SETTINGS& view = GetViewSettings();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMesh* pm = po->GetFEMesh();

	// convert the point to a ray
	makeCurrent();
	GLViewTransform transform(this);
	Ray ray = transform.PointToRay(x, y);

	// convert ray to local coordinates
	Ray localRay;
	localRay.origin = po->GetTransform().GlobalToLocal(ray.origin);
	localRay.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	CCommand* pcmd = 0;
	bool bfound = FindElementIntersection(localRay, *pm, q, m_bctrl);

	if (bfound && m_planeCut)
	{
		vec3d p = po->GetTransform().LocalToGlobal(q.point);

		// see if the intersection lies behind the plane cut. 
		double* a = m_plane;
		double d = p.x*a[0] + p.y*a[1] + p.z*a[2] + a[3];
		if (d < 0)
		{
			// find the intersection with the plane cut
			bfound = FindFaceIntersection(ray, *m_planeCut, q);

			if (bfound)
			{
				// conver the index from a face index into an element index
				int nface = q.m_index;
				if ((nface >= 0) && (nface < m_planeCut->Faces()))
				{
					GMesh::FACE& face = m_planeCut->Face(nface);
					q.m_index = face.eid;
					if (q.m_index < 0) bfound = false;
				}
				else bfound = false;
			}
		}
	}

	if (bfound)
	{
		int index = q.m_index;
		if (view.m_bconn)
		{
			FEElement_* pe, *pe2;
			int elems = pm->Elements();
			vector<int> pint(elems);
			int m = 0;

			for (int i = 0; i<pm->Elements(); ++i) pm->Element(i).m_ntag = i;
			std::stack<FEElement_*> stack;

			// push the first element to the stack
			pe = pm->ElementPtr(index);
			pe->m_ntag = -1;
			pint[m++] = index;
			stack.push(pe);

			double tr = -2;
			vec3d t(0, 0, 0);
			if (pe->IsShell())
			{
				assert(pe->m_face[0] >= 0);
				t = to_vec3d(pm->Face(pe->m_face[0]).m_fn); tr = cos(PI*view.m_fconn / 180.0);
			}

			// get the respect partition boundary flag
			bool bpart = view.m_bpart;
			int gid = pe->m_gid;

			// now push the rest
			int n;
			while (!stack.empty())
			{
				pe = stack.top(); stack.pop();

				// solid elements
				n = pe->Faces();
				for (int i = 0; i<n; ++i)
				if (pe->m_nbr[i] >= 0)
				{
					pe2 = pm->ElementPtr(pe->m_nbr[i]);
					if (pe2->m_ntag >= 0 && pe2->IsVisible())
					{
						if ((view.m_bext == false) || pe2->IsExterior())
						{
							int fid2 = -1;
							if (pe->m_face[i] >= 0)
							{
								FSFace& f2 = pm->Face(pe->m_face[i]);
								fid2 = f2.m_gid;
							}

							if ((bpart == false) || ((pe2->m_gid == gid) && (fid2 == -1)))
							{
								pint[m++] = pe2->m_ntag;
								pe2->m_ntag = -1;
								stack.push(pe2);
							}
						}
					}
				}

				// shell elements
				n = pe->Edges();
				for (int i = 0; i<n; ++i)
				if (pe->m_nbr[i] >= 0)
				{
					pe2 = pm->ElementPtr(pe->m_nbr[i]);
					if (pe2->m_ntag >= 0 && pe2->IsVisible())
					{
						int eface = pe2->m_face[0]; assert(eface >= 0);
						if (eface >= 0)
						{
							if ((view.m_bmax == false) || (pm->Face(eface).m_fn*to_vec3f(t) >= tr))
							{
								if ((bpart == false) || (pe2->m_gid == gid))
								{
									pint[m++] = pe2->m_ntag;
									pe2->m_ntag = -1;
									stack.push(pe2);
								}
							}
						}
					}
				}
			}

			if (m_bctrl) pcmd = new CCmdUnselectElements(pm, &pint[0], m);
			else pcmd = new CCmdSelectElements(pm, &pint[0], m, m_bshift);
		}
		else
		{
			int num = (int)index;
			if (m_bctrl)
				pcmd = new CCmdUnselectElements(pm, &num, 1);
			else
			{
				pcmd = new CCmdSelectElements(pm, &num, 1, m_bshift);

				// print value of currently selected element
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						Post::FEPostModel* fem = postDoc->GetFSModel();
						Post::FEState* state = fem->CurrentState();
						double val = state->m_ELEM[num].m_val;
						FSElement& el = pm->Element(num);
						QString txt = QString("Element %1 : %2\n").arg(el.m_nid).arg(val);
						m_pWnd->AddLogEntry(txt);
					}
				}
			}
		}
	}
	else
	{
		int X = x;
		int Y = y;
		int S = 6;
		QRect rt(X - S, Y - S, 2 * S, 2 * S);

		// try to select discrete elements
		vec3d o(0, 0, 0);
		vec3d O = transform.WorldToScreen(o);

		int index = -1;
		float zmin = 0.f;
		int NE = pm->Elements();
		for (int i = 0; i < NE; ++i)
		{
			FSElement& del = pm->Element(i);
			if (del.IsBeam() && del.IsVisible())
			{
				vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(del.m_node[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(del.m_node[1]).r);

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				// make sure p0, p1 are in front of the camera
				if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) &&
					(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
				{
					// see if the edge intersects
					if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
					{
						if ((index == -1) || (p0.z < zmin))
						{
							index = i;
							zmin = p0.z;
						}
					}
				}
			}
		}

		if (index >= 0)
		{
			pcmd = new CCmdSelectElements(pm, &index, 1, m_bshift);
		}
		else if (!m_bshift)
		{
			int nsel = pm->CountSelectedElements();
			if (nsel > 0)
			{
				pcmd = new CCmdSelectElements(pm, 0, 0, false);
			}
		}
	}

	delete m_planeCut; m_planeCut = nullptr;
	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::SelectFEFaces(int x, int y)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	VIEW_SETTINGS& view = GetViewSettings();

	// Get the active object
	GObject* po = GetActiveObject();
	if (po == 0) return;

	// get the FE mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	// convert the point to a ray
	makeCurrent();
	GLViewTransform transform(this);
	Ray ray = transform.PointToRay(x, y);

	// convert ray to local coordinates
	ray.origin = po->GetTransform().GlobalToLocal(ray.origin);
	ray.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	CCommand* pcmd = 0;
	if (FindFaceIntersection(ray, *pm, q))
	{
		int index = q.m_index;
		if (view.m_bconn)
		{
			// get the list of connected faces
			vector<int> faceList = MeshTools::GetConnectedFaces(pm, index, (view.m_bmax ? view.m_fconn : 0.0), view.m_bpart);

			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, faceList);
			else pcmd = new CCmdSelectFaces(pm, faceList, m_bshift);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, &index, 1);
			else
			{
				pcmd = new CCmdSelectFaces(pm, &index, 1, m_bshift);

				// print value of currently selected face
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						Post::FEPostModel* fem = postDoc->GetFSModel();
						Post::FEState* state = fem->CurrentState();
						double val = state->m_FACE[index].m_val;
						FSFace& face = pm->Face(index);
						QString txt = QString("Face %1 : %2\n").arg(face.m_nid).arg(val);
						m_pWnd->AddLogEntry(txt);
					}
				}
			}
		}
	}
	else if (!m_bshift)
	{
		int nsel = pm->CountSelectedFaces();
		if (nsel > 0)
		{
			pcmd = new CCmdSelectFaces(pm, 0, 0, false);
		}
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::SelectFEEdges(int x, int y)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	makeCurrent();
	GLViewTransform transform(this);

	vec3d o(0,0,0);
	vec3d O = transform.WorldToScreen(o);

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[0]).r);
		vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[1]).r);

		vec3d p0 = transform.WorldToScreen(r0);
		vec3d p1 = transform.WorldToScreen(r1);

		// make sure p0, p1 are in front of the camers
		if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) && 
			(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
			{
				// see if the edge intersects
				if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
				{
					if ((index == -1) || (p0.z < zmin))
					{
						index = i;
						zmin = p0.z;
					}
				}
			}
	}

	// parse the selection buffer
	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn)
		{
			vector<int> pint(pm->Edges());
			int m = 0;

			for (int i = 0; i<pm->Edges(); ++i) pm->Edge(i).m_ntag = i;
			std::stack<FSEdge*> stack;

			FSNodeEdgeList NEL(pm);

			// push the first face to the stack
			FSEdge* pe = pm->EdgePtr(index);
			pint[m++] = index;
			pe->m_ntag = -1;
			stack.push(pe);

			int gid = pe->m_gid;

			// setup the direction vector
			vec3d& r0 = pm->Node(pe->n[0]).r;
			vec3d& r1 = pm->Node(pe->n[1]).r;
			vec3d t1 = r1 - r0; t1.Normalize();

			// angle tolerance
			double wtol = 1.000001*cos(PI*view.m_fconn / 180.0); // scale factor to address some numerical round-off issue when selecting 180 degrees

			// now push the rest
			while (!stack.empty())
			{
				pe = stack.top(); stack.pop();

				for (int i = 0; i<2; ++i)
				{
					int n = NEL.Edges(pe->n[i]);
					for (int j=0; j<n; ++j)
					{
						int edgeID = NEL.Edge(pe->n[i], j)->m_ntag;
						if (edgeID >= 0)
						{
							FSEdge* pe2 = pm->EdgePtr(edgeID);
							vec3d& r0 = pm->Node(pe2->n[0]).r;
							vec3d& r1 = pm->Node(pe2->n[1]).r;
							vec3d t2 = r1 - r0; t2.Normalize();
							if (pe2->IsVisible() && ((view.m_bmax == false) || (fabs(t1*t2) >= wtol)) && ((gid == -1) || (pe2->m_gid == gid)))
							{
								pint[m++] = pe2->m_ntag;
								pe2->m_ntag = -1;
								stack.push(pe2);
							}
						}
					}
				}
			}

			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &pint[0], m);
			else pcmd = new CCmdSelectFEEdges(pm, &pint[0], m, m_bshift);
		}
		else
		{
			int num = (int)index;
			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &num, 1);
			else
			{
				pcmd = new CCmdSelectFEEdges(pm, &num, 1, m_bshift);

				// print value of currently selected edge
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						Post::FEPostModel* fem = postDoc->GetFSModel();
						Post::FEState* state = fem->CurrentState();
						double val = state->m_EDGE[num].m_val;
						FSEdge& ed = pm->Edge(num);
						QString txt = QString("Edge %1 : %2\n").arg(ed.m_nid).arg(val);
						m_pWnd->AddLogEntry(txt);
					}
				}
			}
		}
	}
	else if (!m_bshift)
	{
		int nsel = pm->CountSelectedEdges();
		if (nsel)
		{
			pcmd = new CCmdSelectFEEdges(pm, 0, 0, false);
		}
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::SelectSurfaceFaces(int x, int y)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// Get the active object
	GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(GetActiveObject());
	if (po == 0) return;

	// get the surface mesh
	FSMeshBase* pm = po->GetSurfaceMesh();
	if (pm == 0) return;

	// convert the point to a ray
	makeCurrent();
	GLViewTransform transform(this);
	Ray ray = transform.PointToRay(x, y);

	// convert ray to local coordinates
	ray.origin = po->GetTransform().GlobalToLocal(ray.origin);
	ray.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	CCommand* pcmd = 0;
	if (FindFaceIntersection(ray, *pm, q))
	{
		int index = q.m_index;
		if (view.m_bconn)
		{
			// get the list of connected faces
			vector<int> faceList = MeshTools::GetConnectedFaces(pm, index, (view.m_bmax ? view.m_fconn : 0.0), view.m_bpart);

			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, faceList);
			else pcmd = new CCmdSelectFaces(pm, faceList, m_bshift);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, &index, 1);
			else pcmd = new CCmdSelectFaces(pm, &index, 1, m_bshift);
		}
	}
	else if (!m_bshift) pcmd = new CCmdSelectFaces(pm, 0, 0, false);

	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::SelectSurfaceEdges(int x, int y)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSLineMesh* pm = po->GetEditableLineMesh();

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	makeCurrent();
	GLViewTransform transform(this);

	vec3d o(0, 0, 0);
	vec3d O = transform.WorldToScreen(o);

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[0]).r);
		vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[1]).r);

		vec3d p0 = transform.WorldToScreen(r0);
		vec3d p1 = transform.WorldToScreen(r1);

		// make sure p0, p1 are in front of the camers
		if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) &&
			(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
		{
			// see if the edge intersects
			if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
			{
				if ((index == -1) || (p0.z < zmin))
				{
					index = i;
					zmin = p0.z;
				}
			}
		}
	}

	// parse the selection buffer
	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn)
		{
			vector<int> pint(pm->Edges());
			int m = 0;

			for (int i = 0; i<pm->Edges(); ++i) pm->Edge(i).m_ntag = i;
			std::stack<FSEdge*> stack;

			FSNodeEdgeList NEL(pm);

			// push the first face to the stack
			FSEdge* pe = pm->EdgePtr(index);
			pint[m++] = index;
			pe->m_ntag = -1;
			stack.push(pe);

			int gid = pe->m_gid;

			// setup the direction vector
			vec3d& r0 = pm->Node(pe->n[0]).r;
			vec3d& r1 = pm->Node(pe->n[1]).r;
			vec3d t1 = r1 - r0; t1.Normalize();

			// angle tolerance
			double wtol = 1.000001*cos(PI*view.m_fconn / 180.0); // scale factor to address some numerical round-off issue when selecting 180 degrees

																 // now push the rest
			while (!stack.empty())
			{
				pe = stack.top(); stack.pop();

				for (int i = 0; i<2; ++i)
				{
					int n = NEL.Edges(pe->n[i]);
					for (int j = 0; j<n; ++j)
					{
						int edgeID = NEL.Edge(pe->n[i], j)->m_ntag;
						if (edgeID >= 0)
						{
							FSEdge* pe2 = pm->EdgePtr(edgeID);
							vec3d& r0 = pm->Node(pe2->n[0]).r;
							vec3d& r1 = pm->Node(pe2->n[1]).r;
							vec3d t2 = r1 - r0; t2.Normalize();
							if (pe2->IsVisible() && ((view.m_bmax == false) || (fabs(t1*t2) >= wtol)) && ((gid == -1) || (pe2->m_gid == gid)))
							{
								pint[m++] = pe2->m_ntag;
								pe2->m_ntag = -1;
								stack.push(pe2);
							}
						}
					}
				}
			}

			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &pint[0], m);
			else pcmd = new CCmdSelectFEEdges(pm, &pint[0], m, m_bshift);
		}
		else
		{
			int num = (int)index;
			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &num, 1);
			else pcmd = new CCmdSelectFEEdges(pm, &num, 1, m_bshift);
		}
	}
	else if (!m_bshift) pcmd = new CCmdSelectFEEdges(pm, 0, 0, false);

	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::SelectSurfaceNodes(int x, int y)
{
	static int lastIndex = -1;

	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMeshBase* pm = po->GetEditableMesh();
	FSLineMesh* lineMesh = po->GetEditableLineMesh();
	if (lineMesh == 0) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	makeCurrent();
	GLViewTransform transform(this);

	int index = -1;
	float zmin = 0.f;
	int NN = lineMesh->Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = lineMesh->Node(i);
		if (node.IsVisible() && ((view.m_bext == false) || node.IsExterior()))
		{
			vec3d r = po->GetTransform().LocalToGlobal(lineMesh->Node(i).r);

			vec3d p = transform.WorldToScreen(r);

			if (rt.contains(QPoint((int)p.x, (int)p.y)))
			{
				if ((index == -1) || (p.z < zmin))
				{
					index = i;
					zmin = p.z;
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn && pm)
		{
			vector<int> pint(pm->Nodes(), 0);

			if (view.m_bselpath == false)
			{
				TagConnectedNodes(pm, index);
				lastIndex = -1;
			}
			else
			{
				if ((lastIndex != -1) && (lastIndex != index))
				{
					TagNodesByShortestPath(pm, lastIndex, index);
					lastIndex = index;
				}
				else
				{
					pm->TagAllNodes(0);
					pm->Node(index).m_ntag = 1;
					lastIndex = index;
				}
			}

			// fill the pint array
			int m = 0;
			for (int i = 0; i<pm->Nodes(); ++i)
				if (pm->Node(i).m_ntag == 1) pint[m++] = i;

			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &pint[0], m);
			else pcmd = new CCmdSelectFENodes(pm, &pint[0], m, m_bshift);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectNodes(lineMesh, &index, 1);
			else pcmd = new CCmdSelectFENodes(lineMesh, &index, 1, m_bshift);
			lastIndex = -1;
		}
	}
	else if (!m_bshift)
	{
		pcmd = new CCmdSelectFENodes(lineMesh, 0, 0, false);
		lastIndex = -1;
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

vec3d CGLView::PickPoint(int x, int y, bool* success)
{
	makeCurrent();
	GLViewTransform transform(this);

	if (success) *success = false;
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return vec3d(0,0,0);

	VIEW_SETTINGS& view = GetViewSettings();

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
		vec3d r = m_grid.Intersect(ray.origin, ray.direction, view.m_snapToGrid);
		if (success) *success = true;

		vec3d p = transform.WorldToScreen(r);

		return r;
	}

	return vec3d(0,0,0);
}

void CGLView::RegionSelectObjects(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the document
	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();
	if (model.Objects() == 0) return;

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	vector<GObject*> selectedObjects;
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GLMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			bool intersect = false;
			for (int j = 0; j<mesh->Faces(); ++j)
			{
				GMesh::FACE& face = mesh->Face(j);

				vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
				vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);
				vec3d p2 = transform.WorldToScreen(r2);

				if (region.TriangleIntersect((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y))
				{
					selectedObjects.push_back(po);
					intersect = true;
					break;
				}
			}

			// check nodes too
			if (intersect == false)
			{
				for (int j=0; j<mesh->Nodes(); ++j)
				{
					GMesh::NODE& node = mesh->Node(j);

					vec3d r = po->GetTransform().LocalToGlobal(node.r);
					vec3d p = transform.WorldToScreen(r);
					if (region.IsInside((int) p.x, (int) p.y))
					{
						selectedObjects.push_back(po);
						intersect = true;
						break;
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectObject(&model, selectedObjects);
	else pcmd = new CCmdSelectObject(&model, selectedObjects, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::RegionSelectParts(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the document
	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	if (model.Parts() == 0) return;

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	vector<int> selectedParts;
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GLMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			for (int j = 0; j<mesh->Faces(); ++j)
			{
				GMesh::FACE& face = mesh->Face(j);

				vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
				vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);
				vec3d p2 = transform.WorldToScreen(r2);

				if (region.TriangleIntersect((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y))
				{
					GFace* gface = po->Face(face.pid);
					GPart* part = po->Part(gface->m_nPID[0]);

					int pid = part->GetID();

					// make sure that this surface is not added yet
					bool bfound = false;
					for (int k = 0; k<selectedParts.size(); ++k)
					{
						if (selectedParts[k] == pid)
						{
							bfound = true;
							break;
						}
					}

					if (bfound == false) selectedParts.push_back(pid);
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectPart(&model, selectedParts);
	else pcmd = new CCmdSelectPart(&model, selectedParts, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::RegionSelectSurfaces(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	// get the document
	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int nSurfaces = model.Surfaces();
	if (nSurfaces == 0) return;

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	vector<int> selectedSurfaces;
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GLMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			for (int j=0; j<mesh->Faces(); ++j)
			{
				GMesh::FACE& face = mesh->Face(j);

				vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
				vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);
				vec3d p2 = transform.WorldToScreen(r2);

				if (region.TriangleIntersect((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y))
				{
					int pid = po->Face(face.pid)->GetID();

					// make sure that this surface is not added yet
					bool bfound = false;
					for (int k=0; k<selectedSurfaces.size(); ++k)
					{
						if (selectedSurfaces[k] == pid)
						{
							bfound = true;
							break;
						}
					}

					if (bfound == false) selectedSurfaces.push_back(pid);
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectSurface(&model, selectedSurfaces);
	else pcmd = new CCmdSelectSurface(&model, selectedSurfaces, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}


void CGLView::RegionSelectEdges(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	vector<int> selectedEdges;
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			for (int j=0; j<po->Edges(); ++j)
			{
				GEdge* edge = po->Edge(j);
				int* n = edge->m_node;
				
				if ((n[0] >= 0) && (n[1] >= 0))
				{
					vec3d r0 = po->Node(n[0])->Position();
					vec3d r1 = po->Node(n[1])->Position();

					vec3d p0 = transform.WorldToScreen(r0);
					vec3d p1 = transform.WorldToScreen(r1);

					int x0 = (int)p0.x;
					int y0 = (int)p0.y;
					int x1 = (int)p1.x;
					int y1 = (int)p1.y;

					if (region.LineIntersects(x0, y0, x1, y1))
					{
						selectedEdges.push_back(edge->GetID());
					}
				}
			}
		}
	}


	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectEdge(&model, selectedEdges);
	else pcmd = new CCmdSelectEdge(&model, selectedEdges, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::RegionSelectNodes(const SelectRegion& region)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the document
	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = doc->GetSelectionStyle();

	// Get the model
	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	vector<int> selectedNodes;
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			for (int j=0; j<po->Nodes(); ++j)
			{
				GNode* node = po->Node(j);

				// don't select shape nodes
				if (node->Type() != NODE_SHAPE)
				{
					vec3d r = node->Position();
					vec3d p = transform.WorldToScreen(r);

					if (region.IsInside((int)p.x, (int)p.y))
					{
						selectedNodes.push_back(node->GetID());
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectNode(&model, selectedNodes);
	else pcmd = new CCmdSelectNode(&model, selectedNodes, m_bshift);
	if (pcmd) doc->DoCommand(pcmd);
}

void CGLView::RegionSelectDiscrete(const SelectRegion& region)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the document
	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = doc->GetSelectionStyle();

	// Get the model
	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	vector<int> selectedObjects;

	for (int i=0; i<model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);

		if (dynamic_cast<GLinearSpring*>(po))
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);

			vec3d r0 = model.FindNode(ps->m_node[0])->Position();
			vec3d r1 = model.FindNode(ps->m_node[1])->Position();

			vec3d p0 = transform.WorldToScreen(r0);
			vec3d p1 = transform.WorldToScreen(r1);

			int x0 = (int)p0.x;
			int y0 = (int)p0.y;
			int x1 = (int)p1.x;
			int y1 = (int)p1.y;

			if (region.LineIntersects(x0, y0, x1, y1))
			{
				selectedObjects.push_back(i);
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectDiscrete(&model, selectedObjects);
	else pcmd = new CCmdSelectDiscrete(&model, selectedObjects, m_bshift);
	if (pcmd) doc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
void CGLView::TagBackfacingNodes(FSMeshBase& mesh)
{
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i) mesh.Node(i).m_ntag = 1;

	// assigns 1 to back-facing faces, and 0 to front-facing
	TagBackfacingFaces(mesh);

	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.m_ntag == 0)
		{
			int nn = f.Nodes();
			for (int i = 0; i<nn; ++i) mesh.Node(f.n[i]).m_ntag = 0;
		}
	}
}

void CGLView::RegionSelectFENodes(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMeshBase* pm = po->GetEditableMesh();
	FSLineMesh* lineMesh = po->GetEditableLineMesh();
	if (lineMesh == 0) return;

	makeCurrent();
	GLViewTransform transform(this);

	if (pm)
	{
		// ignore exterior option for surface meshes
		if (view.m_bext || (dynamic_cast<FSSurfaceMesh*>(pm)))
		{
			if (view.m_bcullSel)
			{
				// NOTE: This tags front facing nodes. Should rename function. 
				TagBackfacingNodes(*pm);
			}
			else
			{
				// tag all exterior nodes
				for (int i = 0; i < pm->Nodes(); ++i)
				{
					FSNode& node = pm->Node(i);
					if (node.IsExterior()) node.m_ntag = 0;
					else node.m_ntag = -1;
				}
			}
		}
		else
			pm->TagAllNodes(0);
	}
	else lineMesh->TagAllNodes(0);

	vector<int> selectedNodes;
	for (int i = 0; i<lineMesh->Nodes(); ++i)
	{
		FSNode& node = lineMesh->Node(i);
		if (node.IsVisible() && (node.m_ntag == 0))
		{
			vec3d r = po->GetTransform().LocalToGlobal(node.r);

			vec3d p = transform.WorldToScreen(r);

			if (region.IsInside((int)p.x, (int)p.y))
			{
				selectedNodes.push_back(i);
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, selectedNodes);
	else pcmd = new CCmdSelectFENodes(pm, selectedNodes, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

bool IsBackfacing(const vec3d r[3])
{
	bool b = ((r[1].x - r[0].x)*(r[2].y - r[0].y) - (r[1].y - r[0].y)*(r[2].x - r[0].x)) >= 0.f;
	return b;
}

//-----------------------------------------------------------------------------
void CGLView::TagBackfacingElements(FSMesh& mesh)
{
	GLViewTransform transform(this);
	vec3d r[4], p1[3], p2[3];
	int NE = mesh.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		el.m_ntag = 0;

		// make sure the element is visible
		if (el.IsExterior())
		{
			// get the number of faces
			// Note that NF = 0 for shells so shells are never considered back facing
			int NF = el.Faces();

			// check each face
			// an element is backfacing if all its visible faces are back facing
			bool backFacing = true;
			el.m_ntag = 1;
			for (int j = 0; j<NF; ++j)
			{
				FSElement* pj = (el.m_nbr[j] != -1 ? &mesh.Element(el.m_nbr[j]) : 0);
				if ((pj == 0) || (pj->IsVisible() == false))
				{
					FSFace f = el.GetFace(j);
					switch (f.Type())
					{
					case FE_FACE_TRI3:
					case FE_FACE_TRI6:
					case FE_FACE_TRI7:
					case FE_FACE_TRI10:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					case FE_FACE_QUAD4:
					case FE_FACE_QUAD8:
					case FE_FACE_QUAD9:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;
						r[3] = mesh.Node(f.n[3]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						p2[0] = p1[2];
						p2[1] = transform.WorldToScreen(r[3]);
						p2[2] = p1[0];

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					}
				}

				if (backFacing == false)
				{
					el.m_ntag = 0;
					break;
				}
			}

			// shells 
			if (el.IsShell())
			{
				FSFace* pf = mesh.FacePtr(el.m_face[0]);
				if (pf)
				{
					FSFace& f = *pf;
					switch (f.Type())
					{
					case FE_FACE_TRI3:
					case FE_FACE_TRI6:
					case FE_FACE_TRI7:
					case FE_FACE_TRI10:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					case FE_FACE_QUAD4:
					case FE_FACE_QUAD8:
					case FE_FACE_QUAD9:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;
						r[3] = mesh.Node(f.n[3]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						p2[0] = p1[2];
						p2[1] = transform.WorldToScreen(r[3]);
						p2[2] = p1[0];

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					}
				}

				if (backFacing == false)
				{
					el.m_ntag = 0;
				}
			}
		}
	}
}

void CGLView::RegionSelectFEElems(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMesh* pm = po->GetFEMesh();

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	if (view.m_bcullSel)
	{
		TagBackfacingElements(*pm);
	}
	else pm->TagAllElements(0);

	vector<int> selectedElements;
	int NE = pm->Elements();
	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = pm->Element(i);

		// if the exterior-only flag is off, make sure all solids are selectable
		if ((view.m_bext == false) && el.IsSolid()) el.m_ntag = 0;

		if ((el.m_ntag == 0) && el.IsVisible() && po->Part(el.m_gid)->IsVisible())
		{
			if ((view.m_bext == false) || el.IsExterior())
			{
				int ne = el.Nodes();
				bool binside = false;

				for (int j = 0; j<ne; ++j)
				{
					vec3d r = po->GetTransform().LocalToGlobal(pm->Node(el.m_node[j]).r);
					vec3d p = transform.WorldToScreen(r);
					if (region.IsInside((int)p.x, (int)p.y))
					{
						binside = true;
						break;
					}
				}

				if (binside)
				{
					selectedElements.push_back(i);
				}
			}
		}
	}


	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectElements(pm, selectedElements);
	else pcmd = new CCmdSelectElements(pm, selectedElements, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}


//-----------------------------------------------------------------------------
bool regionFaceIntersect(GLViewTransform& transform, const SelectRegion& region, FSFace& face, FSMeshBase* pm)
{
	if (pm == 0) return false;

	vec3d r[4], p[4];
	bool binside = false;
	switch (face.Type())
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		r[0] = pm->NodePosition(face.n[0]);
		r[1] = pm->NodePosition(face.n[1]);
		r[2] = pm->NodePosition(face.n[2]);

		p[0] = transform.WorldToScreen(r[0]);
		p[1] = transform.WorldToScreen(r[1]);
		p[2] = transform.WorldToScreen(r[2]);

		if (region.TriangleIntersect((int)p[0].x, (int)p[0].y, (int)p[1].x, (int)p[1].y, (int)p[2].x, (int)p[2].y))
		{
			binside = true;
		}
		break;

	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		r[0] = pm->NodePosition(face.n[0]);
		r[1] = pm->NodePosition(face.n[1]);
		r[2] = pm->NodePosition(face.n[2]);
		r[3] = pm->NodePosition(face.n[3]);

		p[0] = transform.WorldToScreen(r[0]);
		p[1] = transform.WorldToScreen(r[1]);
		p[2] = transform.WorldToScreen(r[2]);
		p[3] = transform.WorldToScreen(r[3]);

		if ((region.TriangleIntersect((int)p[0].x, (int)p[0].y, (int)p[1].x, (int)p[1].y, (int)p[2].x, (int)p[2].y)) ||
			(region.TriangleIntersect((int)p[2].x, (int)p[2].y, (int)p[3].x, (int)p[3].y, (int)p[0].x, (int)p[0].y)))
		{
			binside = true;
		}
		break;
	}
	return binside;
}

void CGLView::TagBackfacingFaces(FSMeshBase& mesh)
{
	GLViewTransform transform(this);

	vec3d r[4], p1[3], p2[3];
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);

		if (f.IsExterior())
		{
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
			{
				r[0] = mesh.Node(f.n[0]).r;
				r[1] = mesh.Node(f.n[1]).r;
				r[2] = mesh.Node(f.n[2]).r;

				p1[0] = transform.WorldToScreen(r[0]);
				p1[1] = transform.WorldToScreen(r[1]);
				p1[2] = transform.WorldToScreen(r[2]);

				if (IsBackfacing(p1)) f.m_ntag = 1;
				else f.m_ntag = 0;
			}
			break;
			case FE_FACE_QUAD4:
			case FE_FACE_QUAD8:
			case FE_FACE_QUAD9:
			{
				r[0] = mesh.Node(f.n[0]).r;
				r[1] = mesh.Node(f.n[1]).r;
				r[2] = mesh.Node(f.n[2]).r;
				r[3] = mesh.Node(f.n[3]).r;

				p1[0] = transform.WorldToScreen(r[0]);
				p1[1] = transform.WorldToScreen(r[1]);
				p1[2] = transform.WorldToScreen(r[2]);

				p2[0] = p1[2];
				p2[1] = transform.WorldToScreen(r[3]);
				p2[2] = p1[0];

				if (IsBackfacing(p1) && IsBackfacing(p2)) f.m_ntag = 1;
				else f.m_ntag = 0;
			}
			break;
			}
		}
		else f.m_ntag = 1;
	}
}

void CGLView::RegionSelectFEFaces(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMeshBase* pm = po->GetEditableMesh();

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	// tag back facing items so they won't get selected.
	if (view.m_bcullSel)
	{
		// NOTE: This actually tags front-facing faces. Should rename function.
		TagBackfacingFaces(*pm);
	}
	else if (view.m_bext)
	{
		// tag exterior faces only 
		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& f = pm->Face(i);
			if (f.IsExterior()) f.m_ntag = 0;
			else f.m_ntag = -1;
		}
	}
	else
		pm->TagAllFaces(0);

	int NS = po->Faces();
	vector<bool> vis(NS);
	for (int i=0; i<NS; ++i)
	{
		vis[i] = po->IsFaceVisible(po->Face(i));
	}

	vector<int> selectedFaces;
	int NF = pm->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.IsVisible() && vis[face.m_gid] && (face.m_ntag == 0))
		{
			if (regionFaceIntersect(transform, region, face, pm))
			{
				selectedFaces.push_back(i);
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, selectedFaces);
	else pcmd = new CCmdSelectFaces(pm, selectedFaces, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
void CGLView::TagBackfacingEdges(FSMeshBase& mesh)
{
	int NE = mesh.Edges();
	for (int i = 0; i<NE; ++i) mesh.Edge(i).m_ntag = 1;

	TagBackfacingNodes(mesh);

	for (int i = 0; i<NE; ++i)
	{
		FSEdge& e = mesh.Edge(i);
		if ((mesh.Node(e.n[0]).m_ntag == 0) && (mesh.Node(e.n[1]).m_ntag == 0))
			e.m_ntag = 0;
	}
}

void CGLView::RegionSelectFEEdges(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMeshBase* pm = po->GetEditableMesh();

	// activate the gl rendercontext
	makeCurrent();
	GLViewTransform transform(this);

	if (view.m_bcullSel)
		TagBackfacingEdges(*pm);
	else
		pm->TagAllEdges(0);

	vector<int> selectedEdges;
	int NE = pm->Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsVisible() && (edge.m_ntag == 0))
		{
			vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[0]).r);
			vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[1]).r);

			vec3d p0 = transform.WorldToScreen(r0);
			vec3d p1 = transform.WorldToScreen(r1);

			int x0 = (int)p0.x;
			int y0 = (int)p0.y;
			int x1 = (int)p1.x;
			int y1 = (int)p1.y;

			if (region.LineIntersects(x0, y0, x1, y1))
			{
				selectedEdges.push_back(i);
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, selectedEdges);
	else pcmd = new CCmdSelectFEEdges(pm, selectedEdges, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

void CGLView::SelectFENodes(int x, int y)
{
	static int lastIndex = -1;

	// get the document
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = GetActiveObject();
	if (po == 0) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	makeCurrent();
	GLViewTransform transform(this);

	int index = -1;
	float zmin = 0.f;
	int NN = pm->Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsVisible() && ((view.m_bext == false) || node.IsExterior()))
		{
			vec3d r = po->GetTransform().LocalToGlobal(pm->Node(i).r);

			vec3d p = transform.WorldToScreen(r);

			if (rt.contains(QPoint((int)p.x, (int)p.y)))
			{
				if ((index == -1) || (p.z < zmin))
				{
					index = i;
					zmin = p.z;
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn && pm)
		{
			vector<int> pint(pm->Nodes(), 0);

			if (view.m_bselpath == false)
			{
				TagConnectedNodes(pm, index);
				lastIndex = -1;
			}
			else
			{
				if ((lastIndex != -1) && (lastIndex != index))
				{
					TagNodesByShortestPath(pm, lastIndex, index);
					lastIndex = index;
				}
				else
				{
					pm->TagAllNodes(0);
					pm->Node(index).m_ntag = 1;
					lastIndex = index;
				}
			}

			// fill the pint array
			int m = 0;
			for (int i = 0; i<pm->Nodes(); ++i)
				if (pm->Node(i).m_ntag == 1) pint[m++] = i;

			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &pint[0], m);
			else pcmd = new CCmdSelectFENodes(pm, &pint[0], m, m_bshift);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &index, 1);
			else
			{
				pcmd = new CCmdSelectFENodes(pm, &index, 1, m_bshift);

				// print value of currently selected node
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::FEPostModel* fem = postDoc->GetFSModel();
					Post::FEState* state = fem->CurrentState();
					FSNode& node = pm->Node(index);
					vec3f r = state->m_NODE[index].m_rt;
					QString txt = QString("Node %1 : position = (%2, %3, %4)").arg(node.m_nid).arg(r.x).arg(r.y).arg(r.z);

					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						double val = state->m_NODE[index].m_val;
						txt += QString(", value = %1").arg(val);
					}

					m_pWnd->AddLogEntry(txt + QString("\n"));
				}
			}
			lastIndex = -1;
		}
	}
	else if (!m_bshift)
	{
		int nsel = pm->CountSelectedNodes();
		if (nsel > 0)
		{
			pcmd = new CCmdSelectFENodes(pm, 0, 0, false);
		}
		lastIndex = -1;
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------

void CGLView::TagConnectedNodes(FSMeshBase* pm, int num)
{
	// get the document
	CDocument* pdoc = GetDocument();
	VIEW_SETTINGS& view = GetViewSettings();

	// clear all tags
	for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;

	// first see if this node is a corner node
	if (pm->Node(num).m_gid >= 0)
	{
		pm->Node(num).m_ntag = 1;
	}
	else
	{
		pm->Node(num).m_ntag = 1;

		// see if this node belongs to an edge
		std::stack<FSEdge*> stack;

		// find all edges that have this node as a node
		for (int i = 0; i<pm->Edges(); ++i)
		{
			FSEdge* pe = pm->EdgePtr(i);
			if (pe->m_gid >= 0)
			{
				pe->m_ntag = 0;
				if (pe->FindNodeIndex(num) >= 0)
				{
					stack.push(pe);
					pe->m_ntag = -1;
				}
			}
		}

		if (!stack.empty())
		{
			// now push the rest
			while (!stack.empty())
			{
				FSEdge* pe = stack.top(); stack.pop();

				// mark all nodes
				int nn = pe->Nodes();
				for (int i = 0; i<nn; ++i)
				{
					pm->Node(pe->n[i]).m_ntag = 1;
				}

				// push neighbours
				for (int i = 0; i<2; ++i)
				if (pe->m_nbr[i] >= 0)
				{
					FSEdge* pe2 = pm->EdgePtr(pe->m_nbr[i]);
					if (pe2->m_ntag >= 0 && pe2->IsVisible() && (pe2->m_gid == pe->m_gid))
					{
						pe2->m_ntag = -1;
						stack.push(pe2);
					}
				}
			}
		}
		else
		{
			// create a stack of face pointers
			std::stack<FSFace*> stack;

			// find all faces that have this node as a node
			vec3d t(0, 0, 0);
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FSFace* pf = pm->FacePtr(i);
				pf->m_ntag = 0;
				int m = pf->FindNode(num);
				if (m >= 0)
				{
					t += to_vec3d(pf->m_fn);
					stack.push(pf);
					pf->m_ntag = -1;
				}
			}
			t.Normalize();
			double tr = cos(PI*view.m_fconn / 180.0);
			bool bangle = view.m_bmax;

			// now push the rest
			int m = 0;
			while (!stack.empty())
			{
				FSFace* pf = stack.top(); stack.pop();
				int nn = pf->Nodes();
				int ne = pf->Edges();

				// mark all nodes
				for (int i = 0; i<nn; ++i)
				{
					pm->Node(pf->n[i]).m_ntag = 1;
				}

				// push neighbours
				for (int i = 0; i<ne; ++i)
				if (pf->m_nbr[i] >= 0)
				{
					FSFace* pf2 = pm->FacePtr(pf->m_nbr[i]);
					if (pf2->m_ntag >= 0 && pf2->IsVisible() && (pf2->m_gid == pf->m_gid) && ((pf2->m_fn*to_vec3f(t) >= tr) || (bangle == false)))
					{
						pf2->m_ntag = -1;
						stack.push(pf2);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLView::TagNodesByShortestPath(FSMeshBase* pm, int n0, int n1)
{
	if (n1 == n0) return;

	pm->TagAllNodes(0);
	pm->Node(n0).m_ntag = 1;

	vec3d r0 = pm->Node(n0).r;
	vec3d r1 = pm->Node(n1).r;

	// see if the start and end node lie on an edge
	bool b0 = false, b1 = false;
	int NE = pm->Edges();
	for (int i=0; i<NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if ((edge.n[0] == n0) || (edge.n[1] == n0)) b0 = true;
		if ((edge.n[0] == n1) || (edge.n[1] == n1)) b1 = true;

		if (b0 && b1) break;
	}

	if (b0 && b1)
	{
		// see if we can connect the nodes by staying on edges
		FSNodeEdgeList NEL(pm);

		int n = n0;
		double Lmin = (r1 - r0).SqrLength();
		do
		{
			int minNode = -1;
			int nval = NEL.Edges(n);
			for (int i = 0; i<nval; ++i)
			{
				const FSEdge* pe = NEL.Edge(n, i);
				int ne = pe->Nodes();
				for (int j = 0; j<ne; ++j)
				{
					int nj = pe->n[j];
					if (pm->Node(nj).m_ntag == 0)
					{
						vec3d rj = pm->Node(nj).r;
						double L = (r1 - rj).SqrLength();
						if (L < Lmin)
						{
							Lmin = L;
							minNode = nj;
						}
					}
				}
			}

			if (minNode != -1)
			{
				pm->Node(minNode).m_ntag = 1;
				n = minNode;
				if (minNode == n1) break;
			}
			else break;
		}
		while (1);
	}
	else
	{
		FSNodeFaceList NFL;
		NFL.Build(pm);

		int n = n0;
		do
		{
			// this is the line to project on
			vec3d e = (r1 - r0); e.Normalize();

			double Lmin = 1e99;
			int minNode = -1;
			int nval = NFL.Valence(n);
			for (int i=0; i<nval; ++i)
			{
				FSFace* pf = NFL.Face(n, i);
				int nf = pf->Nodes();
				for (int j=0; j<nf; ++j)
				{
					int nj = pf->n[j];
					if (pm->Node(nj).m_ntag == 0)
					{
						vec3d pj = pm->Node(nj).r - r0;

						// see if the projects on the positive side of the line
						double l = pj*e;
						if (l > 0)
						{
							// calculate distance to line
							double L = (pj - e*l).Length();
							if (L < Lmin)
							{
								Lmin = L;
								minNode = nj;
							}
						}
					}
				}
			}

			if (minNode != -1)
			{
				pm->Node(minNode).m_ntag = 1;
				n = minNode;
				if (minNode == n1) break;

				r0 = pm->Node(minNode).r;
			}
			else break;
		}
		while (1);
	}
}

//-----------------------------------------------------------------------------
void CGLView::RenderFeatureEdges()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 0);

	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();

	for (int k = 0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		if (po->IsVisible())
		{
			glPushMatrix();
			SetModelView(po);

			GLMesh& m = *po->GetRenderMesh();
			m_renderer.RenderGLEdges(&m);

			glPopMatrix();
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
vec3d CGLView::GetPickPosition()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return vec3d(0, 0, 0);
	return Get3DCursor();
}

//-----------------------------------------------------------------------------
vec3d CGLView::GetPivotPosition()
{
	if (m_bpivot) return m_pv;
	else
	{
		CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
		if (pdoc == nullptr) return vec3d(0,0,0);

		FESelection* ps = pdoc->GetCurrentSelection();
		vec3d r(0, 0, 0);
		if (ps && ps->Size())
		{
			r = pdoc->GetCurrentSelection()->GetPivot();
			if (fabs(r.x)<1e-7) r.x = 0;
			if (fabs(r.y)<1e-7) r.y = 0;
			if (fabs(r.z)<1e-7) r.z = 0;
		}
		m_pv = r;
		return r;
	}
}

//-----------------------------------------------------------------------------
void CGLView::SetPivot(const vec3d& r)
{ 
	m_pv = r;
	repaint();
}

//-----------------------------------------------------------------------------
quatd CGLView::GetPivotRotation()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc && (m_coord == COORD_LOCAL))
	{
		FESelection* ps = doc->GetCurrentSelection();
		if (ps) return ps->GetOrientation();
	}

	return quatd(0.0, 0.0, 0.0, 1.0);
}

//=============================================================================
//					Rendering functions for GObjects
//=============================================================================

//-----------------------------------------------------------------------------
// Render non-selected nodes
void CGLView::RenderNodes(GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 255);
	for (int i = 0; i<po->Nodes(); ++i)
	{
		// only render nodes that are not selected
		// and are not shape-nodes
		GNode& n = *po->Node(i);
		if (!n.IsSelected() && (n.Type() != NODE_SHAPE))
		{
			vec3d r = n.LocalPosition();
			glBegin(GL_POINTS);
			{
				glVertex3d(r.x, r.y, r.z);
			}
			glEnd();
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render selected nodes
void CGLView::RenderSelectedNodes(GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255, 255, 0);
	for (int i = 0; i<po->Nodes(); ++i)
	{
		GNode& n = *po->Node(i);
		if (n.IsSelected())
		{
			assert(n.Type() != NODE_SHAPE);
			vec3d r = n.LocalPosition();
			glBegin(GL_POINTS);
			{
				glVertex3d(r.x, r.y, r.z);
			}
			glEnd();
		}
	}

#ifdef _DEBUG
	// Draw FE nodes on top of GMesh nodes to make sure they match
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		glColor3ub(255, 0, 0);
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FSNode& n = pm->Node(i);
			if (n.m_gid > -1)
			{
				GNode& gn = *po->Node(n.m_gid);
				if (gn.IsSelected())
				{
					vec3d r = n.r;
					glBegin(GL_POINTS);
					{
						glVertex3d(r.x, r.y, r.z);
					}
					glEnd();
				}
			}
		}
	}
#endif

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render non-selected edges
void CGLView::RenderEdges(GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 255);

	GLMesh& m = *po->GetRenderMesh();
	int N = po->Edges();
	for (int i = 0; i<N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected() == false)
		{
			m_renderer.RenderGLEdges(&m, i);
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render selected edges
void CGLView::RenderSelectedEdges(GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	vec3d r1, r2;

	GLMesh& m = *po->GetRenderMesh();
	int N = po->Edges();
	for (int i = 0; i<N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected())
		{
			m_renderer.RenderGLEdges(&m, i);

			GNode* n0 = po->Node(e.m_node[0]);
			GNode* n1 = po->Node(e.m_node[1]);

			if (n0 && n1)
			{
				glBegin(GL_POINTS);
				{
					vec3d r0 = n0->LocalPosition();
					vec3d r1 = n1->LocalPosition();
					glVertex3d(r0.x, r0.y, r0.z);
					glVertex3d(r1.x, r1.y, r1.z);
				}
				glEnd();
			}
		}
	}

#ifdef _DEBUG
	// Render FE edges onto of GMesh edges to make sure they are consistent
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		glColor3ub(255, 0, 0);
		for (int i = 0; i<pm->Edges(); ++i)
		{
			FSEdge& e = pm->Edge(i);
			if (e.m_gid > -1)
			{
				GEdge& ge = *po->Edge(e.m_gid);
				if (ge.IsSelected())
				{
					vec3d r0 = pm->Node(e.n[0]).r;
					vec3d r1 = pm->Node(e.n[1]).r;
					glBegin(GL_LINES);
					{
						glVertex3d(r0.x, r0.y, r0.z);
						glVertex3d(r1.x, r1.y, r1.z);
					}
					glEnd();
				}
			}
		}
	}
#endif
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render non-selected surfaces
void CGLView::RenderSurfaces(GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& vs = GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	assert(pm);

	// render non-selected faces
	GPart* pgmat = 0; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n<NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// make sure this face is not selected
		if (f.IsSelected() == false)
		{
			// get the part IDs
			int* pid = f.m_nPID;

			// get the part (that is visible)
			GPart* pg = po->Part(pid[0]);
			if (pg && pg->IsVisible() == false)
			{
				if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = 0;
				if (pg && (pg->IsVisible() == false)) pg = 0;
			}

			// make sure we have a part
			if (pg)
			{
				// if this part is not the current part defining the 
				// material, we need to change the mat props
				if (pg != pgmat)
				{
					if (vs.m_objectColor == 0)
					{
						GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
						SetMatProps(pmat);
						GLColor c = po->GetColor();
						if (pmat) c = pmat->Diffuse();
						glColor3ub(c.r, c.g, c.b);
						pgmat = pg;
					}
					else
					{
						SetMatProps(0);
						GLColor c = po->GetColor();
						glColor3ub(c.r, c.g, c.b);
					}
				}

				if (vs.m_transparencyMode != 0)
				{
					switch (vs.m_transparencyMode)
					{
					case 1: if (po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					case 2: if (!po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					}
				}

				// render the face
				m_renderer.RenderGLMesh(pm, n);

				if (vs.m_transparencyMode != 0) glDisable(GL_POLYGON_STIPPLE);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Render selected surfaces
void CGLView::RenderSelectedSurfaces(GObject* po)
{
	if (!po->IsVisible()) return;

	GLMesh* pm = po->GetRenderMesh();
	assert(pm);

	// render the selected faces
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	{
		glColor3ub(0, 0, 255);
		glEnable(GL_POLYGON_STIPPLE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		int NF = po->Faces();
		for (int i = 0; i<NF; ++i)
		{
			GFace& f = *po->Face(i);
			if (f.IsSelected())
			{
				m_renderer.RenderGLMesh(pm, i);
			}
		}

#ifdef _DEBUG
		// Render the GFace nodes and the FE surfaces to make sure the 
		// GMesh and the FE mesh are consisten

		// render GNodes
		// TODO: This causes a crash after a primitive was converted to editable mesh and auto partition was applied.
/*		for (int i = 0; i<NF; ++i)
		{
			GFace& f = *po->Face(i);
			if (f.IsSelected())
			{
				glBegin(GL_POINTS);
				{
					int nf = f.Nodes();
					for (int j = 0; j<nf; ++j)
					if (f.m_node[j] != -1)
					{
						vec3d r = po->Node(f.m_node[j])->LocalPosition();
						int c = 255 * j / (nf - 1);
						glColor3ub((GLubyte)c, (GLubyte)c, (GLubyte)c);
						glVertex3d(r.x, r.y, r.z);
					}
				}
				glEnd();
			}
		}
*/
		// render FE surfaces
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			glColor3ub(255, 0, 0);
			vec3d rf[FSElement::MAX_NODES];
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FSFace& f = pm->Face(i);
				if (f.m_gid > -1)
				{
					GFace& gf = *po->Face(f.m_gid);
					if (gf.IsSelected())
					{
						int nf = f.Nodes();
						for (int j = 0; j<nf; ++j) rf[j] = pm->Node(f.n[j]).r;
						switch (nf)
						{
						case 3:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
							}
							glEnd();
							break;
						case 4:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
							}
							glEnd();
							break;
						case 6:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);

								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);

								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);

								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
							}
							glEnd();
							break;
						case 8:
						case 9:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[7].x, rf[7].y, rf[7].z);

								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);

								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);

								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[7].x, rf[7].y, rf[7].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);

								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);
								glVertex3d(rf[7].x, rf[7].y, rf[7].z);

								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);
							}
							glEnd();
							break;
						}
					}
				}
			}
		}
#endif
		glDisable(GL_POLYGON_STIPPLE);
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render non-selected parts
void CGLView::RenderParts(GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& vs = GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	assert(pm);

	// render non-selected parts
	GPart* pgmat = 0; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n<NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// get the part IDs
		int* pid = f.m_nPID;

		// get the part (that is visible)
		GPart* pg = po->Part(pid[0]); assert(pg);
		if (pg && ((pg->IsVisible() == false) || (pg->IsSelected())))
		{
			if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = 0;
			if (pg && ((pg->IsVisible() == false) || pg->IsSelected())) pg = 0;

			if (pg == nullptr)
			{
				if (pid[2] >= 0) pg = po->Part(pid[2]); else pg = 0;
				if (pg && ((pg->IsVisible() == false) || pg->IsSelected())) pg = 0;
			}
		}

		// make sure we have a part
		if (pg)
		{
			// if this part is not the current part defining the 
			// material, we need to change the mat props
			if (vs.m_objectColor == 0)
			{
				if (pg != pgmat)
				{
					GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
					SetMatProps(pmat);
					GLColor c = po->GetColor();
					if (pmat) c = pmat->Diffuse();
					glColor3ub(c.r, c.g, c.b);
					pgmat = pg;
				}
			}
			else
			{
				SetMatProps(0);
				GLColor c = po->GetColor();
				glColor3ub(c.r, c.g, c.b);
			}

			if (vs.m_transparencyMode != 0)
			{
				switch (vs.m_transparencyMode)
				{
				case 1: if (po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
				case 2: if (!po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
				}
			}

			// render the face
			int nid = pg->GetLocalID();
			m_renderer.RenderGLMesh(pm, n);

			if (vs.m_transparencyMode != 0) glDisable(GL_POLYGON_STIPPLE);
		}
	}
}

//-----------------------------------------------------------------------------
// render selected parts
void CGLView::RenderSelectedParts(GObject* po)
{
	if (!po->IsVisible()) return;

	glPushAttrib(GL_ENABLE_BIT);
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_POLYGON_STIPPLE);
		SetMatProps(0);
		glColor3ub(0, 0, 255);
		GLMesh& m = *po->GetRenderMesh();
		int NF = po->Faces();
		for (int i = 0; i<NF; ++i)
		{
			GFace* pf = po->Face(i);
			GPart* p0 = po->Part(pf->m_nPID[0]);
			GPart* p1 = po->Part(pf->m_nPID[1]);
			GPart* p2 = po->Part(pf->m_nPID[2]);
			if ((p0 && p0->IsSelected()) || (p1 && p1->IsSelected()) || (p2 && p2->IsSelected()))
			{
				m_renderer.RenderGLMesh(&m, i);
			}
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This function renders the object by looping over all the parts and
// for each part render the external surfaces that belong to that part.
// NOTE: The reason why only external surfaces are rendered is because
//       it is possible for an external surface to coincide with an
//       internal surface. E.g., when a shell layer lies on top of a 
//       hex layer.
void CGLView::RenderObject(GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& vs = GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	if (pm == 0) return;
	assert(pm);

	// render non-selected faces
	GPart* pgmat = 0; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n<NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// make sure the face is visible
		if (f.IsVisible())
		{
			// get the part IDs
			int* pid = f.m_nPID;

			// get the part (that is visible)
			GPart* pg = po->Part(pid[0]);
			if (pg && pg->IsVisible() == false)
			{
				if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = 0;
				if (pg && (pg->IsVisible() == false)) pg = 0;
			}

			// make sure we have a part
			if (pg)
			{
				// if this part is not the current part defining the 
				// material, we need to change the mat props
				if (vs.m_objectColor == 0)
				{
					if (pg != pgmat)
					{
						GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
						SetMatProps(pmat);
						GLColor c = po->GetColor();
						if (pmat) c = pmat->Diffuse();

						glColor3ub(c.r, c.g, c.b);
						pgmat = pg;
					}
				}
				else
				{
					SetMatProps(0);
					GLColor c = po->GetColor();
					glColor3ub(c.r, c.g, c.b);
				}

				if (vs.m_transparencyMode != 0)
				{
					switch (vs.m_transparencyMode)
					{
					case 1: if (po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					case 2: if (!po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					}
				}

				// render the face
				m_renderer.RenderGLMesh(pm, n);

				if (vs.m_transparencyMode != 0) glDisable(GL_POLYGON_STIPPLE);
			}
		}
	}

	if (NF == 0)
	{
		// if there are no faces, render edges instead
		int NC = po->Edges();
		for (int n=0; n<NC; ++n)
		{
			GEdge& e = *po->Edge(n);
			if (e.IsVisible())
				m_renderer.RenderGLEdges(pm, e.GetLocalID());
		}
	}
}

//=============================================================================
//					Rendering functions for FEMeshes
//=============================================================================

//-----------------------------------------------------------------------------
// Render the FE nodes
void CGLView::RenderFENodes(GObject* po)
{
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	quatd q = pdoc->GetView()->GetCamera().GetOrientation();

	// set the point size
	float fsize = GetViewSettings().m_node_size;
	m_renderer.SetPointSize(fsize);

	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		int N = pm->Nodes();
		int NF = pm->Faces();
		int NE = pm->Elements();

		// reset all tags
		for (int i = 0; i<N; ++i) pm->Node(i).m_ntag = 1;

		// make sure we render all isolated nodes
		for (int i = 0; i<NE; ++i)
		{
			FSElement& el = pm->Element(i);
			int n = el.Nodes();
			for (int j = 0; j<n; ++j) pm->Node(el.m_node[j]).m_ntag = 0;
		}

		// check visibility
		for (int i = 0; i<NE; ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.IsVisible() && (po->Part(el.m_gid)->IsVisible()))
			{
				int n = el.Nodes();
				for (int j = 0; j<n; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
			}
		}

		// check the cull
		if (view.m_bcull)
		{
			vec3d f;
			for (int i = 0; i<NF; ++i)
			{
				FSFace& face = pm->Face(i);
				int n = face.Nodes();
				for (int j = 0; j<n; ++j)
				{
					vec3d nn = to_vec3d(face.m_nn[j]);
					f = q*nn;
					if (f.z < 0) pm->Node(face.n[j]).m_ntag = 0;
				}
			}
		}

		// check the ext criteria
		if (view.m_bext)
		{
			for (int i = 0; i<N; ++i)
			{
				FSNode& node = pm->Node(i);
				if (!node.IsExterior()) node.m_ntag = 0;
			}
		}

		m_renderer.RenderFENodes(pm);
	}
	else
	{
		FSMeshBase* mesh = po->GetEditableMesh();
		if (mesh)
		{
			// reset all tags
			mesh->TagAllNodes(1);

			// make sure we render all isolated nodes
			int NF = mesh->Faces();
			for (int i = 0; i<NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				int n = face.Nodes();
				for (int j = 0; j<n; ++j) mesh->Node(face.n[j]).m_ntag = 0;
			}

			// check visibility
			for (int i = 0; i<NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				if (face.IsVisible())
				{
					int n = face.Nodes();
					for (int j = 0; j<n; ++j) mesh->Node(face.n[j]).m_ntag = 1;
				}
			}

			// check the cull
			if (view.m_bcull)
			{
				vec3d f;
				for (int i = 0; i<NF; ++i)
				{
					FSFace& face = mesh->Face(i);
					int n = face.Nodes();
					for (int j = 0; j<n; ++j)
					{
						vec3d nn = to_vec3d(face.m_nn[j]);
						f = q*nn;
						if (f.z < 0) mesh->Node(face.n[j]).m_ntag = 0;
					}
				}
			}

			m_renderer.RenderFENodes(mesh);
		}
		else
		{
			FSLineMesh* pm = po->GetEditableLineMesh();
			if (pm)
			{
				pm->TagAllNodes(1);
				m_renderer.RenderFENodes(pm);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLView::RenderFEFaces(GObject* po)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	FSModel& fem = *doc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		RenderObject(po);
		return;
	}

	GLColor col = po->GetColor();
	GLColor dif = col;
	SetMatProps(0);
	glColor3ub(dif.r, dif.g, dif.b);
	int nmatid = -1;

	double vmin, vmax;
	Post::CColorMap map;
	Mesh_Data& data = pm->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());
	if (showContour) { data.GetValueRange(vmin, vmax); map.SetRange((float)vmin, (float)vmax); }

	// render the unselected faces
	for (int i = 0; i<pm->Faces(); i++)
	{
		FSFace& face = pm->Face(i);

		FSElement& el = pm->Element(face.m_elem[0].eid);
		GPart* pg = po->Part(el.m_gid);
		if ((pg->IsVisible() == false) && (face.m_elem[1].eid != -1))
		{
			FSElement& el1 = pm->Element(face.m_elem[1].eid);
			pg = po->Part(el1.m_gid);
		}

		if (!face.IsSelected() && face.IsVisible())
		{
			if (pg && pg->IsVisible())
			{
				if (showContour)
				{
					if (data.GetElementDataTag(face.m_elem[0].eid) > 0)
					{
						int fnl[FSElement::MAX_NODES];
						int nn = el.GetLocalFaceIndices(face.m_elem[0].lid, fnl);
						assert(nn == face.Nodes());

						GLColor c[FSFace::MAX_NODES];
						int nf = face.Nodes();
						for (int j = 0; j<nf; ++j)
							c[j] = map.map(data.GetElementValue(face.m_elem[0].eid, fnl[j]));

						// Render the face
						m_renderer.RenderFace(face, pm, c, 1);
					}
					else
					{
						dif = GLColor(212, 212, 212);
						glColor3ub(dif.r, dif.g, dif.b);

						// Render the face
						glBegin(GL_TRIANGLES);
						{
							m_renderer.RenderFEFace(face, pm);
						}
						glEnd();
					}
				}
				else 
				{
					if (view.m_objectColor == 0)
					{
						if (pg->GetMaterialID() != nmatid)
						{
							nmatid = pg->GetMaterialID();
							GMaterial* pmat = fem.GetMaterialFromID(nmatid);
							SetMatProps(pmat);
							dif = (pmat ? pmat->Diffuse() : col);
							glColor3ub(dif.r, dif.g, dif.b);

							int glmode = 0;
							if (pmat && (pmat->m_nrender != 0))
							{
								GLint n[2];
								glGetIntegerv(GL_POLYGON_MODE, n);
								glmode = n[1];
								if (n[1] != GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							}
						}
					}

					// Render the face
					glBegin(GL_TRIANGLES);
					{
						m_renderer.RenderFEFace(face, pm);
					}
					glEnd();
				}
			}
		}
	}

	// render the selected faces
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_STIPPLE);
	glColor3ub(255, 0, 0);
	m_renderer.RenderSelectedFEFaces(pm);

	// render the selected face outline
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	m_renderer.RenderSelectedFEFacesOutline(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLView::RenderSurfaceMeshFaces(GObject* po)
{
	GSurfaceMeshObject* surfaceObject = dynamic_cast<GSurfaceMeshObject*>(po);
	if (surfaceObject == 0)
	{
		// just render something, otherwise nothing will show up
		RenderObject(po);
		return;
	}

	FSSurfaceMesh* surfaceMesh = surfaceObject->GetSurfaceMesh();
	assert(surfaceMesh);
	if (surfaceMesh == 0) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	FSModel& fem = *doc->GetFSModel();

	GLColor col = po->GetColor();
	SetMatProps(0);
	glColor3ub(col.r, col.g, col.b);

	// render the unselected faces
	// Note that we do not render internal faces
	m_renderer.RenderUnselectedFEFaces(surfaceMesh);

	// render the selected faces
	// override some settings
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_STIPPLE);
	glColor3ub(255, 128, 0);
	m_renderer.RenderSelectedFEFaces(surfaceMesh);

	// render the selected face outline
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	m_renderer.RenderSelectedFEFacesOutline(surfaceMesh);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLView::RenderSurfaceMeshEdges(GObject* po)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	FSModel& fem = *doc->GetFSModel();
	FSLineMesh* pm = po->GetEditableLineMesh();
	assert(pm);
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	// render the unselected edges
	glColor3ub(0, 0, 255);
	m_renderer.RenderUnselectedFEEdges(pm);

	// render the selected edges
	// override some settings
	glDisable(GL_CULL_FACE);
	glColor3ub(255, 0, 0);
	m_renderer.RenderSelectedFEEdges(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLView::RenderSurfaceMeshNodes(GObject* po)
{
	CGLDocument* pdoc = GetDocument();
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	quatd q = pdoc->GetView()->GetCamera().GetOrientation();

	// set the point size
	float fsize = GetViewSettings().m_node_size;
	m_renderer.SetPointSize(fsize);

	FSMeshBase* mesh = po->GetEditableMesh();
	if (mesh)
	{
		// reset all tags
		mesh->TagAllNodes(1);

		// make sure we render all isolated nodes
		int NF = mesh->Faces();
		for (int i = 0; i<NF; ++i)
		{
			FSFace& face = mesh->Face(i);
			int n = face.Nodes();
			for (int j = 0; j<n; ++j) mesh->Node(face.n[j]).m_ntag = 0;
		}

		// check visibility
		for (int i = 0; i<NF; ++i)
		{
			FSFace& face = mesh->Face(i);
			if (face.IsVisible())
			{
				int n = face.Nodes();
				for (int j = 0; j<n; ++j) mesh->Node(face.n[j]).m_ntag = 1;
			}
		}

		// check the cull
		if (view.m_bcull)
		{
			vec3d f;
			for (int i = 0; i<NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				int n = face.Nodes();
				for (int j = 0; j<n; ++j)
				{
					vec3d nn = to_vec3d(face.m_nn[j]);
					f = q*nn;
					if (f.z < 0) mesh->Node(face.n[j]).m_ntag = 0;
				}
			}
		}

		m_renderer.RenderFENodes(mesh);
	}
	else
	{
		FSLineMesh* pm = po->GetEditableLineMesh();
		if (pm)
		{
			pm->TagAllNodes(1);
			m_renderer.RenderFENodes(pm);
		}
	}
}

//-----------------------------------------------------------------------------
// Render the FE Edges
void CGLView::RenderFEEdges(GObject* po)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();
	FSModel& fem = *doc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	// render the unselected edges
	glColor3ub(0, 0, 255);
	m_renderer.RenderUnselectedFEEdges(pm);

	// render the selected edges
	// override some settings
	glDisable(GL_CULL_FACE);
	glColor3ub(255, 0, 0);
	m_renderer.RenderSelectedFEEdges(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render the FE elements
void CGLView::RenderFEElements(GObject* po)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	FSModel& fem = *pdoc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	assert(pm);
	if (pm == 0) return;

	VIEW_SETTINGS& view = GetViewSettings();
	GLColor dif;

	GLColor col = po->GetColor();

	int i;

	int nmatid = -1;
	dif = po->GetColor();
	glColor3ub(dif.r, dif.g, dif.b);
	SetMatProps(0);
	int glmode = 0;

	double vmin, vmax;
	Mesh_Data& data = pm->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());
	if (showContour) { data.GetValueRange(vmin, vmax); m_colorMap.SetRange((float)vmin, (float)vmax); }

	// render the unselected faces
	vector<int> selectedElements;
	int NE = pm->Elements();
	for (i = 0; i<NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsVisible() && el.IsSelected()) selectedElements.push_back(i);

		if (!el.IsSelected() && el.IsVisible())
		{
			GPart* pg = po->Part(el.m_gid);
			if (pg->IsVisible())
			{
				if (showContour)
				{
					GLColor c[FSElement::MAX_NODES];
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j)
					{
						if (data.GetElementDataTag(i) > 0)
							c[j] = m_colorMap.map(data.GetElementValue(i, j));
						else
							c[j] = GLColor(212, 212, 212);
					}

					switch (el.Type())
					{
					case FE_HEX8  : m_renderer.RenderHEX8(&el, pm, c); break;
					case FE_HEX20 : m_renderer.RenderHEX20(&el, pm, true); break;
					case FE_HEX27 : m_renderer.RenderHEX27(&el, pm, true); break;
					case FE_PENTA6: m_renderer.RenderPENTA(&el, pm, true); break;
					case FE_PENTA15: m_renderer.RenderPENTA15(&el, pm, true); break;
					case FE_TET4  : m_renderer.RenderTET4(&el, pm, c); break;
					case FE_TET5  : m_renderer.RenderTET4(&el, pm, c); break;
					case FE_TET10 : m_renderer.RenderTET10(&el, pm, c); break;
					case FE_TET15 : m_renderer.RenderTET15(&el, pm, true); break;
					case FE_TET20 : m_renderer.RenderTET20(&el, pm, true); break;
					case FE_QUAD4 : m_renderer.RenderQUAD(&el, pm, c); break;
					case FE_QUAD8 : m_renderer.RenderQUAD8(&el, pm, true); break;
					case FE_QUAD9 : m_renderer.RenderQUAD9(&el, pm, true); break;
					case FE_TRI3  : m_renderer.RenderTRI3(&el, pm, c); break;
					case FE_TRI6  : m_renderer.RenderTRI6(&el, pm, true); break;
					case FE_PYRA5 : m_renderer.RenderPYRA5(&el, pm, true); break;
                    case FE_PYRA13: m_renderer.RenderPYRA13(&el, pm, true); break;
					case FE_BEAM2 : break;
					case FE_BEAM3 : break;
					default:
						assert(false);
					}

				}
				else
				{
					if (view.m_objectColor == 0)
					{
						if (pg->GetMaterialID() != nmatid)
						{
							GMaterial* pmat = 0;
							if (pg->GetMaterialID() != nmatid)
							{
								nmatid = pg->GetMaterialID();
								pmat = fem.GetMaterialFromID(nmatid);
								SetMatProps(pmat);
							}

							dif = (pmat != 0 ? pmat->Diffuse() : col);

							glColor3ub(dif.r, dif.g, dif.b);

							if (pmat && (pmat->m_nrender != 0))
							{
								GLint n[2];
								glGetIntegerv(GL_POLYGON_MODE, n);
								glmode = n[1];
								if (n[1] != GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							}
						}
					}

					switch (el.Type())
					{
					case FE_HEX8  : m_renderer.RenderHEX8(&el, pm, true); break;
					case FE_HEX20 : m_renderer.RenderHEX20(&el, pm, true); break;
					case FE_HEX27 : m_renderer.RenderHEX27(&el, pm, true); break;
					case FE_PENTA6: m_renderer.RenderPENTA(&el, pm, true); break;
					case FE_PENTA15: m_renderer.RenderPENTA15(&el, pm, true); break;
					case FE_TET4  : m_renderer.RenderTET4(&el, pm, true); break;
					case FE_TET5  : m_renderer.RenderTET4(&el, pm, true); break;
					case FE_TET10 : m_renderer.RenderTET10(&el, pm, true); break;
					case FE_TET15 : m_renderer.RenderTET15(&el, pm, true); break;
					case FE_TET20 : m_renderer.RenderTET20(&el, pm, true); break;
					case FE_QUAD4 : m_renderer.RenderQUAD(&el, pm, true); break;
					case FE_QUAD8 : m_renderer.RenderQUAD8(&el, pm, true); break;
					case FE_QUAD9 : m_renderer.RenderQUAD9(&el, pm, true); break;
					case FE_TRI3  : m_renderer.RenderTRI3(&el, pm, true); break;
					case FE_TRI6  : m_renderer.RenderTRI6(&el, pm, true); break;
					case FE_PYRA5 : m_renderer.RenderPYRA5(&el, pm, true); break;
                    case FE_PYRA13: m_renderer.RenderPYRA13(&el, pm, true); break;
					case FE_BEAM2 : break;
					case FE_BEAM3 : break;
					default:
						assert(false);
					}
				}
			}
		}
	}

	// override some settings
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glColor4ub(255, 0, 0, 128);
	glEnable(GL_POLYGON_STIPPLE);
	glDisable(GL_LIGHTING);

	// render the selected faces
	if (pdoc == nullptr) return;
	if (selectedElements.empty() == false)
	{
		int NE = (int)selectedElements.size();
		for (i = 0; i<NE; ++i)
		{
			FEElement_& el = pm->Element(selectedElements[i]);
			if (el.IsVisible())
			{
				switch (el.Type())
				{
				case FE_HEX8  : m_renderer.RenderHEX8(&el, pm, false); break;
				case FE_HEX20 : m_renderer.RenderHEX20(&el, pm, false); break;
				case FE_HEX27 : m_renderer.RenderHEX27(&el, pm, false); break;
				case FE_PENTA6: m_renderer.RenderPENTA(&el, pm, false); break;
				case FE_PENTA15: m_renderer.RenderPENTA15(&el, pm, true); break;
				case FE_TET4   : m_renderer.RenderTET4(&el, pm, false); break;
				case FE_TET5   : m_renderer.RenderTET4(&el, pm, false); break;
				case FE_TET10 : m_renderer.RenderTET10(&el, pm, false); break;
				case FE_TET15 : m_renderer.RenderTET15(&el, pm, false); break;
				case FE_TET20 : m_renderer.RenderTET20(&el, pm, false); break;
				case FE_QUAD4 : m_renderer.RenderQUAD(&el, pm, false); break;
				case FE_QUAD8 : break;
				case FE_QUAD9 : break;
				case FE_TRI3  : m_renderer.RenderTRI3(&el, pm, false); break;
				case FE_TRI6  : m_renderer.RenderTRI6(&el, pm, false); break;
				case FE_PYRA5 : m_renderer.RenderPYRA5(&el, pm, false); break;
                case FE_PYRA13: m_renderer.RenderPYRA13(&el, pm, false); break;
				case FE_BEAM2 : break;
				case FE_BEAM3 : break;
				default:
					assert(false);
				}
			}
		}

		// render a yellow highlight around selected elements
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glColor3ub(255, 255, 0);


		glBegin(GL_LINES);
		for (i = 0; i < NE; ++i)
		{
			FEElement_& el = pm->Element(selectedElements[i]);
			int ne = el.Nodes();
			if (el.IsVisible())
			{
				switch (el.Type())
				{
				case FE_HEX8:
				case FE_HEX20:
				case FE_HEX27:
					for (int j = 0; j < 12; ++j)
					{
						int n0 = el.m_node[ET_HEX[j][0]];
						int n1 = el.m_node[ET_HEX[j][1]];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
				case FE_TET4:
				case FE_TET10:
				case FE_TET15:
					for (int j = 0; j < 6; ++j)
					{
						int n0 = el.m_node[ET_TET[j][0]];
						int n1 = el.m_node[ET_TET[j][1]];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
                case FE_PYRA5:
                case FE_PYRA13:
                    for (int j = 0; j < 8; ++j)
                    {
                        int n0 = el.m_node[ET_PYR[j][0]];
                        int n1 = el.m_node[ET_PYR[j][1]];
                            
                        vec3d r0 = pm->Node(n0).pos();
                        vec3d r1 = pm->Node(n1).pos();
                            
                        glx::vertex3d(r0);
                        glx::vertex3d(r1);
                    }
                    break;
				case FE_TRI3:
				case FE_QUAD4:
					for (int i = 0; i < ne; ++i)
					{
						int n0 = el.m_node[i];
						int n1 = el.m_node[(i + 1) % ne];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
				}
			}
		}
		glEnd();

		glPopAttrib();
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This function is used for selecting elements
void CGLView::RenderFEAllElements(FSMesh* pm, bool bexterior)
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	bool bcull = view.m_bcull;
	bool bok;
	int i, j;

	glPushAttrib(GL_ENABLE_BIT);
	if (bcull == false) glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	for (i = 0; i<pm->Elements(); i++)
	{
		FSElement& e = pm->Element(i);

		bok = e.IsVisible();
		if (bexterior && (e.IsExterior() == false)) bok = false;

		if (bok)
		{
			switch (e.Type())
			{
			case FE_HEX8:
			{
							glBegin(GL_QUADS);
							{
								for (j = 0; j<6; j++)
								{
									bok = true;
									if (bcull)
									{
										bok = false;
										if ((e.m_nbr[j] == -1) || (!pm->Element(e.m_nbr[j]).IsVisible()))
										{
											bok = true;
										}
									}

									// don't render when the neighbor is a shell
									// we have to do this otherwise the shell will not be selected.
									if ((e.m_nbr[j] != -1) && (pm->Element(e.m_nbr[j]).IsShell())) bok = false;

									if (bok)
									{
										const vec3d& r1 = pm->Node(e.m_node[FTHEX8[j][0]]).r;
										const vec3d& r2 = pm->Node(e.m_node[FTHEX8[j][1]]).r;
										const vec3d& r3 = pm->Node(e.m_node[FTHEX8[j][2]]).r;
										const vec3d& r4 = pm->Node(e.m_node[FTHEX8[j][3]]).r;

										vec3d n = (r2 - r1) ^ (r3 - r1);
										n.Normalize();

										glNormal3d(n.x, n.y, n.z);
										glVertex3d(r1.x, r1.y, r1.z);
										glVertex3d(r2.x, r2.y, r2.z);
										glVertex3d(r3.x, r3.y, r3.z);
										glVertex3d(r4.x, r4.y, r4.z);
									}
								}
							}
							glEnd();
			}
				break;
			case FE_HEX20:
			{
							 glBegin(GL_TRIANGLES);
							 {
								 for (j = 0; j<6; j++)
								 {
									 bok = true;
									 if (bcull)
									 {
										 bok = false;
										 if ((e.m_nbr[j] == -1) || (!pm->Element(e.m_nbr[j]).IsVisible()))
										 {
											 bok = true;
										 }
									 }

									 if (bok)
									 {
										 const vec3d& r1 = pm->Node(e.m_node[FTHEX20[j][0]]).r;
										 const vec3d& r2 = pm->Node(e.m_node[FTHEX20[j][1]]).r;
										 const vec3d& r3 = pm->Node(e.m_node[FTHEX20[j][2]]).r;
										 const vec3d& r4 = pm->Node(e.m_node[FTHEX20[j][3]]).r;
										 const vec3d& r5 = pm->Node(e.m_node[FTHEX20[j][4]]).r;
										 const vec3d& r6 = pm->Node(e.m_node[FTHEX20[j][5]]).r;
										 const vec3d& r7 = pm->Node(e.m_node[FTHEX20[j][6]]).r;
										 const vec3d& r8 = pm->Node(e.m_node[FTHEX20[j][7]]).r;

										 vec3d n = (r2 - r1) ^ (r3 - r1);
										 n.Normalize();

										 glNormal3d(n.x, n.y, n.z);
										 glVertex3d(r1.x, r1.y, r1.z); glVertex3d(r5.x, r5.y, r5.z); glVertex3d(r8.x, r8.y, r8.z);
										 glVertex3d(r5.x, r5.y, r5.z); glVertex3d(r2.x, r2.y, r2.z); glVertex3d(r6.x, r6.y, r6.z);
										 glVertex3d(r6.x, r6.y, r6.z); glVertex3d(r3.x, r3.y, r3.z); glVertex3d(r7.x, r7.y, r7.z);
										 glVertex3d(r7.x, r7.y, r7.z); glVertex3d(r4.x, r4.y, r4.z); glVertex3d(r8.x, r8.y, r8.z);
										 glVertex3d(r5.x, r5.y, r5.z); glVertex3d(r6.x, r6.y, r6.z); glVertex3d(r8.x, r8.y, r8.z);
										 glVertex3d(r6.x, r6.y, r6.z); glVertex3d(r7.x, r7.y, r7.z); glVertex3d(r8.x, r8.y, r8.z);
									 }
								 }
							 }
							 glEnd();
			}
				break;
			case FE_HEX27:
			{
							 glBegin(GL_TRIANGLES);
							 {
								 for (j = 0; j<6; j++)
								 {
									 bok = true;
									 if (bcull)
									 {
										 bok = false;
										 if ((e.m_nbr[j] == -1) || (!pm->Element(e.m_nbr[j]).IsVisible()))
										 {
											 bok = true;
										 }
									 }

									 if (bok)
									 {
										 const vec3d& r0 = pm->Node(e.m_node[FTHEX27[j][0]]).r;
										 const vec3d& r1 = pm->Node(e.m_node[FTHEX27[j][1]]).r;
										 const vec3d& r2 = pm->Node(e.m_node[FTHEX27[j][2]]).r;
										 const vec3d& r3 = pm->Node(e.m_node[FTHEX27[j][3]]).r;
										 const vec3d& r4 = pm->Node(e.m_node[FTHEX27[j][4]]).r;
										 const vec3d& r5 = pm->Node(e.m_node[FTHEX27[j][5]]).r;
										 const vec3d& r6 = pm->Node(e.m_node[FTHEX27[j][6]]).r;
										 const vec3d& r7 = pm->Node(e.m_node[FTHEX27[j][7]]).r;
										 const vec3d& r8 = pm->Node(e.m_node[FTHEX27[j][8]]).r;

										 vec3d n = (r1 - r0) ^ (r2 - r0);
										 n.Normalize();

										 glNormal3d(n.x, n.y, n.z);
										 glVertex3d(r0.x, r0.y, r0.z); glVertex3d(r4.x, r4.y, r4.z); glVertex3d(r8.x, r8.y, r8.z);
										 glVertex3d(r8.x, r8.y, r8.z); glVertex3d(r7.x, r7.y, r7.z); glVertex3d(r0.x, r0.y, r0.z);
										 glVertex3d(r4.x, r4.y, r4.z); glVertex3d(r1.x, r1.y, r1.z); glVertex3d(r5.x, r5.y, r5.z);
										 glVertex3d(r5.x, r5.y, r5.z); glVertex3d(r8.x, r8.y, r8.z); glVertex3d(r4.x, r4.y, r4.z);
										 glVertex3d(r7.x, r7.y, r7.z); glVertex3d(r8.x, r8.y, r8.z); glVertex3d(r6.x, r6.y, r6.z);
										 glVertex3d(r6.x, r6.y, r6.z); glVertex3d(r3.x, r3.y, r3.z); glVertex3d(r7.x, r7.y, r7.z);
										 glVertex3d(r8.x, r8.y, r8.z); glVertex3d(r5.x, r5.y, r5.z); glVertex3d(r2.x, r2.y, r2.z);
										 glVertex3d(r2.x, r2.y, r2.z); glVertex3d(r6.x, r6.y, r6.z); glVertex3d(r8.x, r8.y, r8.z);
									 }
								 }
							 }
							 glEnd();
			}
				break;
			case FE_PENTA6:
			case FE_PENTA15:
			{
					glBegin(GL_QUADS);
					{
						for (j = 0; j<3; j++)
						{
							const vec3d& r1 = pm->Node(e.m_node[FTPENTA[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTPENTA[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTPENTA[j][2]]).r;
							const vec3d& r4 = pm->Node(e.m_node[FTPENTA[j][3]]).r;

							vec3d n = (r2 - r1) ^ (r3 - r1);
							n.Normalize();

							glNormal3d(n.x, n.y, n.z);
							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
							glVertex3d(r4.x, r4.y, r4.z);
						}
					}
					glEnd();

					glBegin(GL_TRIANGLES);
					{
						for (j = 3; j<5; j++)
						{
							const vec3d& r1 = pm->Node(e.m_node[FTPENTA[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTPENTA[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTPENTA[j][2]]).r;

							vec3d n = (r2 - r1) ^ (r3 - r1);
							n.Normalize();

							glNormal3d(n.x, n.y, n.z);
							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
						}
					}
					glEnd();
				}
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
			case FE_TET20:
				{
					glBegin(GL_TRIANGLES);
					{
						for (j = 0; j<4; j++)
						{
							const vec3d& r1 = pm->Node(e.m_node[FTTET[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTTET[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTTET[j][2]]).r;

							//							vec3d n = (r2 - r1)^(r3 - r1);
							//							n.Normalize();

							//							glNormal3d(n.x, n.y, n.z);
							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
						}
					}
					glEnd();
				}
				break;
			case FE_PYRA5:
				{
					glBegin(GL_TRIANGLES);
					{
						for (j = 0; j<4; j++)
						{
							const vec3d& r1 = pm->Node(e.m_node[FTPYRA5[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTPYRA5[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTPYRA5[j][2]]).r;

							vec3d n = (r2 - r1) ^ (r3 - r1);
							n.Normalize();

							glNormal3d(n.x, n.y, n.z);
							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
						}
					}
					glEnd();				

					glBegin(GL_QUADS);
					{
						const vec3d& r1 = pm->Node(e.m_node[FTPYRA5[4][0]]).r;
						const vec3d& r2 = pm->Node(e.m_node[FTPYRA5[4][1]]).r;
						const vec3d& r3 = pm->Node(e.m_node[FTPYRA5[4][2]]).r;
						const vec3d& r4 = pm->Node(e.m_node[FTPYRA5[4][3]]).r;

						vec3d n = (r2 - r1) ^ (r3 - r1);
						n.Normalize();

						glNormal3d(n.x, n.y, n.z);
						glVertex3d(r1.x, r1.y, r1.z);
						glVertex3d(r2.x, r2.y, r2.z);
						glVertex3d(r3.x, r3.y, r3.z);
						glVertex3d(r4.x, r4.y, r4.z);
					}
					glEnd();

				}
				break;
            case FE_PYRA13:
                {
                    glBegin(GL_TRIANGLES);
                    {
                        for (j = 0; j<4; j++)
                        {
                            const vec3d& r1 = pm->Node(e.m_node[FTPYRA13[j][0]]).r;
                            const vec3d& r2 = pm->Node(e.m_node[FTPYRA13[j][1]]).r;
                            const vec3d& r3 = pm->Node(e.m_node[FTPYRA13[j][2]]).r;
                            
                            vec3d n = (r2 - r1) ^ (r3 - r1);
                            n.Normalize();
                            
                            glNormal3d(n.x, n.y, n.z);
                            glVertex3d(r1.x, r1.y, r1.z);
                            glVertex3d(r2.x, r2.y, r2.z);
                            glVertex3d(r3.x, r3.y, r3.z);
                        }
                    }
                    glEnd();
                    
                    glBegin(GL_QUADS);
                    {
                        const vec3d& r1 = pm->Node(e.m_node[FTPYRA13[4][0]]).r;
                        const vec3d& r2 = pm->Node(e.m_node[FTPYRA13[4][1]]).r;
                        const vec3d& r3 = pm->Node(e.m_node[FTPYRA13[4][2]]).r;
                        const vec3d& r4 = pm->Node(e.m_node[FTPYRA13[4][3]]).r;
                        
                        vec3d n = (r2 - r1) ^ (r3 - r1);
                        n.Normalize();
                        
                        glNormal3d(n.x, n.y, n.z);
                        glVertex3d(r1.x, r1.y, r1.z);
                        glVertex3d(r2.x, r2.y, r2.z);
                        glVertex3d(r3.x, r3.y, r3.z);
                        glVertex3d(r4.x, r4.y, r4.z);
                    }
                    glEnd();
                    
                }
			case FE_QUAD4:
			case FE_QUAD8:
			case FE_QUAD9:
			{
							 const vec3d& r1 = pm->Node(e.m_node[0]).r;
							 const vec3d& r2 = pm->Node(e.m_node[1]).r;
							 const vec3d& r3 = pm->Node(e.m_node[2]).r;
							 const vec3d& r4 = pm->Node(e.m_node[3]).r;

							 vec3d n = (r2 - r1) ^ (r3 - r1);
							 n.Normalize();

							 glBegin(GL_QUADS);
							 {
								 glNormal3d(n.x, n.y, n.z);
								 glVertex3d(r1.x, r1.y, r1.z);
								 glVertex3d(r2.x, r2.y, r2.z);
								 glVertex3d(r3.x, r3.y, r3.z);
								 glVertex3d(r4.x, r4.y, r4.z);
							 }
							 glEnd();
			}
				break;
			case FE_TRI3:
			case FE_TRI6:
			{
							const vec3d& r1 = pm->Node(e.m_node[0]).r;
							const vec3d& r2 = pm->Node(e.m_node[1]).r;
							const vec3d& r3 = pm->Node(e.m_node[2]).r;

							vec3d n = (r2 - r1) ^ (r3 - r1);
							n.Normalize();

							glBegin(GL_TRIANGLES);
							{
								glNormal3d(n.x, n.y, n.z);
								glVertex3d(r1.x, r1.y, r1.z);
								glVertex3d(r2.x, r2.y, r2.z);
								glVertex3d(r3.x, r3.y, r3.z);
							}
							glEnd();
			}
				break;
			case FE_BEAM2:
			{
							 const vec3d& r1 = pm->Node(e.m_node[0]).r;
							 const vec3d& r2 = pm->Node(e.m_node[1]).r;
							 glBegin(GL_LINES);
							 {
								 glNormal3d(1, 1, 1);
								 glVertex3d(r1.x, r1.y, r1.z);
								 glVertex3d(r2.x, r2.y, r2.z);
							 }
							 glEnd();
			}
				break;
			default:
				assert(false);
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLView::RenderMeshLines()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	GModel& model = *pdoc->GetGModel();
	int nitem = pdoc->GetItemMode();

	VIEW_SETTINGS& vs = GetViewSettings();
	GLColor c = vs.m_mcol;
	glColor3ub(c.r, c.g, c.b);

	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				glPushMatrix();
				SetModelView(po);
				if (nitem == ITEM_ELEM)
					RenderMeshLines(po);
				else if (nitem == ITEM_MESH)
					m_renderer.RenderMeshLines(pm);
				else if (nitem != ITEM_EDGE)
					m_renderer.RenderMeshLines(po->GetEditableMesh());
				glPopMatrix();
			}
			else if (dynamic_cast<GSurfaceMeshObject*>(po))
			{
				FSSurfaceMesh* surfaceMesh = dynamic_cast<GSurfaceMeshObject*>(po)->GetSurfaceMesh();
				if (surfaceMesh && (nitem != ITEM_EDGE))
				{
					glPushMatrix();
					SetModelView(po);
					m_renderer.RenderMeshLines(surfaceMesh);
					glPopMatrix();
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLView::RenderMeshLines(GObject* po)
{
	if ((po == 0) || !po->IsVisible()) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// loop over all elements
	for (int i = 0; i<pm->Elements(); i++)
	{
		const FEElement_& e = pm->ElementRef(i);
		if (e.IsVisible() && (po->Part(e.m_gid)->IsVisible()))
		{
			switch (e.Type())
			{
			case FE_HEX8:
			{
				for (int j = 0; j<6; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{

							const vec3d& r1 = pm->Node(e.m_node[FTHEX8[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTHEX8[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTHEX8[j][2]]).r;
							const vec3d& r4 = pm->Node(e.m_node[FTHEX8[j][3]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
							glVertex3d(r4.x, r4.y, r4.z);
						}
						glEnd();
					}
				}
			}
			break;
			case FE_HEX20:
			case FE_HEX27:
				{
					for (int j = 0; j<6; j++)
					{
						FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

						if ((pen == 0) || (!pen->IsVisible()))
						{
							glBegin(GL_LINE_LOOP);
							{

								const vec3d& r1 = pm->Node(e.m_node[FTHEX20[j][0]]).r;
								const vec3d& r2 = pm->Node(e.m_node[FTHEX20[j][1]]).r;
								const vec3d& r3 = pm->Node(e.m_node[FTHEX20[j][2]]).r;
								const vec3d& r4 = pm->Node(e.m_node[FTHEX20[j][3]]).r;
								const vec3d& r5 = pm->Node(e.m_node[FTHEX20[j][4]]).r;
								const vec3d& r6 = pm->Node(e.m_node[FTHEX20[j][5]]).r;
								const vec3d& r7 = pm->Node(e.m_node[FTHEX20[j][6]]).r;
								const vec3d& r8 = pm->Node(e.m_node[FTHEX20[j][7]]).r;

								glVertex3d(r1.x, r1.y, r1.z);
								glVertex3d(r5.x, r5.y, r5.z);
								glVertex3d(r2.x, r2.y, r2.z);
								glVertex3d(r6.x, r6.y, r6.z);
								glVertex3d(r3.x, r3.y, r3.z);
								glVertex3d(r7.x, r7.y, r7.z);
								glVertex3d(r4.x, r4.y, r4.z);
								glVertex3d(r8.x, r8.y, r8.z);
							}
							glEnd();
						}
					}
				}
				break;
			case FE_PENTA6:
			case FE_PENTA15:
				{
					for (int j = 0; j<3; j++)
					{
						FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

						if ((pen == 0) || (!pen->IsVisible()))
						{
							glBegin(GL_LINE_LOOP);
							{
								const vec3d& r1 = pm->Node(e.m_node[FTPENTA[j][0]]).r;
								const vec3d& r2 = pm->Node(e.m_node[FTPENTA[j][1]]).r;
								const vec3d& r3 = pm->Node(e.m_node[FTPENTA[j][2]]).r;
								const vec3d& r4 = pm->Node(e.m_node[FTPENTA[j][3]]).r;

								glVertex3d(r1.x, r1.y, r1.z);
								glVertex3d(r2.x, r2.y, r2.z);
								glVertex3d(r3.x, r3.y, r3.z);
								glVertex3d(r4.x, r4.y, r4.z);
							}
							glEnd();
						}
					}

					for (int j = 3; j<5; j++)
					{
						FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

						if ((pen == 0) || (!pen->IsVisible()))
						{
							glBegin(GL_LINE_LOOP);
							{
								const vec3d& r1 = pm->Node(e.m_node[FTPENTA[j][0]]).r;
								const vec3d& r2 = pm->Node(e.m_node[FTPENTA[j][1]]).r;
								const vec3d& r3 = pm->Node(e.m_node[FTPENTA[j][2]]).r;

								glVertex3d(r1.x, r1.y, r1.z);
								glVertex3d(r2.x, r2.y, r2.z);
								glVertex3d(r3.x, r3.y, r3.z);
							}
							glEnd();
						}
					}
				}
				break;
			case FE_PYRA5:
				{
					for (int j = 0; j<4; j++)
					{
						glBegin(GL_LINE_LOOP);
						{
							const vec3d& r1 = pm->Node(e.m_node[FTPYRA5[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTPYRA5[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTPYRA5[j][2]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
						}
						glEnd();
					}

					glBegin(GL_LINE_LOOP);
					{
						const vec3d& r1 = pm->Node(e.m_node[FTPYRA5[4][0]]).r;
						const vec3d& r2 = pm->Node(e.m_node[FTPYRA5[4][1]]).r;
						const vec3d& r3 = pm->Node(e.m_node[FTPYRA5[4][2]]).r;
						const vec3d& r4 = pm->Node(e.m_node[FTPYRA5[4][3]]).r;

						glVertex3d(r1.x, r1.y, r1.z);
						glVertex3d(r2.x, r2.y, r2.z);
						glVertex3d(r3.x, r3.y, r3.z);
						glVertex3d(r4.x, r4.y, r4.z);
					}
					glEnd();

				}
				break;

            case FE_PYRA13:
                {
                    for (int j = 0; j<4; j++)
                    {
                        glBegin(GL_LINE_LOOP);
                        {
                            const vec3d& r1 = pm->Node(e.m_node[FTPYRA13[j][0]]).r;
                            const vec3d& r2 = pm->Node(e.m_node[FTPYRA13[j][1]]).r;
                            const vec3d& r3 = pm->Node(e.m_node[FTPYRA13[j][2]]).r;
                            const vec3d& r4 = pm->Node(e.m_node[FTPYRA13[j][3]]).r;
                            const vec3d& r5 = pm->Node(e.m_node[FTPYRA13[j][4]]).r;
                            const vec3d& r6 = pm->Node(e.m_node[FTPYRA13[j][5]]).r;

                            glVertex3d(r1.x, r1.y, r1.z);
                            glVertex3d(r4.x, r4.y, r4.z);
                            glVertex3d(r2.x, r2.y, r2.z);
                            glVertex3d(r5.x, r5.y, r5.z);
                            glVertex3d(r3.x, r3.y, r3.z);
                            glVertex3d(r6.x, r6.y, r6.z);
                        }
                        glEnd();
                    }
                    
                    glBegin(GL_LINE_LOOP);
                    {
                        const vec3d& r1 = pm->Node(e.m_node[FTPYRA13[4][0]]).r;
                        const vec3d& r2 = pm->Node(e.m_node[FTPYRA13[4][1]]).r;
                        const vec3d& r3 = pm->Node(e.m_node[FTPYRA13[4][2]]).r;
                        const vec3d& r4 = pm->Node(e.m_node[FTPYRA13[4][3]]).r;
                        const vec3d& r5 = pm->Node(e.m_node[FTPYRA13[4][4]]).r;
                        const vec3d& r6 = pm->Node(e.m_node[FTPYRA13[4][5]]).r;
                        const vec3d& r7 = pm->Node(e.m_node[FTPYRA13[4][6]]).r;
                        const vec3d& r8 = pm->Node(e.m_node[FTPYRA13[4][7]]).r;

                        glVertex3d(r1.x, r1.y, r1.z);
                        glVertex3d(r5.x, r5.y, r5.z);
                        glVertex3d(r2.x, r2.y, r2.z);
                        glVertex3d(r6.x, r6.y, r6.z);
                        glVertex3d(r3.x, r3.y, r3.z);
                        glVertex3d(r7.x, r7.y, r7.z);
                        glVertex3d(r4.x, r4.y, r4.z);
                        glVertex3d(r8.x, r8.y, r8.z);
                    }
                    glEnd();
                    
                }
                    break;
                    
			case FE_TET4:
			case FE_TET5:
			case FE_TET20:
				{
					for (int j = 0; j<4; j++)
					{
						FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));
						if ((pen == 0) || (!pen->IsVisible()))
						{
							glBegin(GL_LINE_LOOP);
							{
								const vec3d& r1 = pm->Node(e.m_node[FTTET[j][0]]).r;
								const vec3d& r2 = pm->Node(e.m_node[FTTET[j][1]]).r;
								const vec3d& r3 = pm->Node(e.m_node[FTTET[j][2]]).r;

								glVertex3d(r1.x, r1.y, r1.z);
								glVertex3d(r2.x, r2.y, r2.z);
								glVertex3d(r3.x, r3.y, r3.z);
							}
							glEnd();
						}
					}
				}
				break;
			case FE_TET10:
			case FE_TET15:
				{
					for (int j = 0; j<4; j++)
					{
						FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));
						if ((pen == 0) || (!pen->IsVisible()))
						{
							glBegin(GL_LINE_LOOP);
							{
								const vec3d& r1 = pm->Node(e.m_node[FTTET10[j][0]]).r;
								const vec3d& r2 = pm->Node(e.m_node[FTTET10[j][1]]).r;
								const vec3d& r3 = pm->Node(e.m_node[FTTET10[j][2]]).r;
								const vec3d& r4 = pm->Node(e.m_node[FTTET10[j][3]]).r;
								const vec3d& r5 = pm->Node(e.m_node[FTTET10[j][4]]).r;
								const vec3d& r6 = pm->Node(e.m_node[FTTET10[j][5]]).r;

								glVertex3d(r1.x, r1.y, r1.z);
								glVertex3d(r4.x, r4.y, r4.z);
								glVertex3d(r2.x, r2.y, r2.z);
								glVertex3d(r5.x, r5.y, r5.z);
								glVertex3d(r3.x, r3.y, r3.z);
								glVertex3d(r6.x, r6.y, r6.z);
							}
							glEnd();
						}
					}
				}
				break;
			case FE_QUAD4:
			case FE_QUAD8:
			case FE_QUAD9:
				{
					glBegin(GL_LINE_LOOP);
					{
						const vec3d& r1 = pm->Node(e.m_node[0]).r;
						const vec3d& r2 = pm->Node(e.m_node[1]).r;
						const vec3d& r3 = pm->Node(e.m_node[2]).r;
						const vec3d& r4 = pm->Node(e.m_node[3]).r;

						glVertex3d(r1.x, r1.y, r1.z);
						glVertex3d(r2.x, r2.y, r2.z);
						glVertex3d(r3.x, r3.y, r3.z);
						glVertex3d(r4.x, r4.y, r4.z);
					}
					glEnd();
				}
				break;
			case FE_TRI3:
			case FE_TRI6:
				{
					glBegin(GL_LINE_LOOP);
					{
						const vec3d& r1 = pm->Node(e.m_node[0]).r;
						const vec3d& r2 = pm->Node(e.m_node[1]).r;
						const vec3d& r3 = pm->Node(e.m_node[2]).r;

						glVertex3d(r1.x, r1.y, r1.z);
						glVertex3d(r2.x, r2.y, r2.z);
						glVertex3d(r3.x, r3.y, r3.z);
					}
					glEnd();
				}
				break;
			} // switch
		} // if
	} // for

	glPopAttrib();
}


//-----------------------------------------------------------------
// this function will only adjust the camera if the currently
// selected object is too close.
void CGLView::ZoomSelection(bool forceZoom)
{
	CPostDocument* postDoc = m_pWnd->GetPostDocument();
	if (postDoc == nullptr)
	{
		// get the current selection
		CModelDocument* mdoc = dynamic_cast<CModelDocument*>(GetDocument());
		if (mdoc == nullptr) return;

		FESelection* ps = mdoc->GetCurrentSelection();

		// zoom out on current selection
		if (ps && ps->Size() != 0)
		{
			// get the selection's bounding box
			BOX box = ps->GetBoundingBox();

			double f = box.GetMaxExtent();
			if (f == 0) f = 1;

			CGLCamera& cam = mdoc->GetView()->GetCamera();

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
	else
	{
		if (postDoc->IsValid())
		{
			BOX box = postDoc->GetSelectionBox();

			if (box.IsValid() == false)
			{
				ZoomExtents();
			}
			else
			{
				if (box.Radius() < 1e-8f)
				{
					float L = 1.f;
					BOX bb = postDoc->GetBoundingBox();
					float R = bb.GetMaxExtent();
					if (R < 1e-8f) L = 1.f; else L = 0.05f*R;

					box.InflateTo(L, L, L);
				}

				CGLCamera& cam = postDoc->GetView()->GetCamera();
				cam.SetTarget(box.Center());
				cam.SetTargetDistance(3.f*box.Radius());

				repaint();
			}
		}
	}
}

//-----------------------------------------------------------------
void CGLView::ZoomToObject(GObject *po)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	BOX box = po->GetGlobalBox();

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = doc->GetView()->GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0*f);
	cam.SetOrientation(po->GetTransform().GetRotationInverse());

	repaint();
}

//-----------------------------------------------------------------
//! zoom in on a box
void CGLView::ZoomTo(const BOX& box)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = doc->GetView()->GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0*f);

	repaint();
}

//-----------------------------------------------------------------
void CGLView::ZoomExtents(bool banimate)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	BOX box;
	CPostDocument* postDoc = m_pWnd->GetPostDocument();
	if (postDoc == nullptr)
	{
		CModelDocument* mdoc = m_pWnd->GetModelDocument();
		if (mdoc == 0) return;
		box = mdoc->GetModelBox();
	}
	else
	{
		CPostObject* po = postDoc->GetPostObject();
		if (po == nullptr) return;

		box = po->GetBoundingBox();
	}

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = doc->GetView()->GetCamera();

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

	VIEW_SETTINGS& view = GetViewSettings();

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
	for (int i = 0; i<NN; ++i) pmb->Node(i).m_ntag = 0;

	int mode = doc->GetItemMode();

	// process elements
	if ((mode == ITEM_ELEM) && pm)
	{
		int NE = pm->Elements();
		for (int i = 0; i<NE; i++)
		{
			FEElement_& el = pm->Element(i);
			if (el.IsSelected())
			{
				tag.r = pm->LocalToGlobal(pm->ElementCenter(el));
				tag.bvis = false;
				tag.ntag = 0;
				int nid = el.GetID();
				if (nid < 0) nid = i + 1;
				sprintf(tag.sztag, "E%d", nid);
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
				tag.bvis = false;
				tag.ntag = (f.IsExternal() ? 0 : 1);
				int nid = f.GetID();
				if (nid < 0) nid = i + 1;
				sprintf(tag.sztag, "F%d", nid);
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
		for (int i = 0; i<NC; i++)
		{
			FSEdge& edge = pmb->Edge(i);
			if (edge.IsSelected())
			{
				tag.r = pmb->LocalToGlobal(pmb->EdgeCenter(edge));
				tag.bvis = false;
				tag.ntag = 0;
				int nid = edge.GetID();
				if (nid < 0) nid = i + 1;
				sprintf(tag.sztag, "L%d", nid);
				vtag.push_back(tag);

				int ne = edge.Nodes();
				for (int j = 0; j < ne; ++j) pmb->Node(edge.n[j]).m_ntag = 1;
			}
		}
	}

	// process nodes
	if (mode == ITEM_NODE)
	{
		for (int i = 0; i<NN; i++)
		{
			FSNode& node = pmb->Node(i);
			if (node.IsSelected())
			{
				tag.r = pmb->LocalToGlobal(node.r);
				tag.bvis = false;
				tag.ntag = (node.IsExterior() ? 0 : 1);
				int nid = node.GetID();
				if (nid < 0) nid = i + 1;
				sprintf(tag.sztag, "N%d", nid);
				vtag.push_back(tag);
			}
		}
	}

	// add additional nodes
	if (view.m_ntagInfo == 1)
	{
		for (int i = 0; i<NN; i++)
		{
			FSNode& node = pmb->Node(i);
			if (node.m_ntag == 1)
			{
				tag.r = pmb->LocalToGlobal(node.r);
				tag.bvis = false;
				tag.ntag = (node.IsExterior() ? 0 : 1);
				sprintf(tag.sztag, "N%d", node.GetID());
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

	// find out where the tags are on the screen
	GLViewTransform transform(this);
	for (int i = 0; i<nsel; i++)
	{
		vec3d p = transform.WorldToScreen(vtag[i].r);
		vtag[i].wx = p.x;
		vtag[i].wy = m_viewport[3] - p.y;
		vtag[i].bvis = true;
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

	double dpr = GetDevicePixelRatio();
	for (int i = 0; i<nsel; i++)
		if (vtag[i].bvis)
		{
			glBegin(GL_POINTS);
			{
				glColor3ub(0, 0, 0);
				int x = (int)(vtag[i].wx * dpr);
				int y = (int)(m_viewport[3] - dpr*(m_viewport[3] - vtag[i].wy));
				glVertex2f(x, y);
				if (vtag[i].ntag == 0) glColor3ub(255, 255, 0);
				else glColor3ub(255, 0, 0);
				glVertex2f(x - 1, y + 1);
			}
			glEnd();
		}

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setFont(QFont("Helvetica", 10));
	for (int i = 0; i<nsel; ++i)
		if (vtag[i].bvis)
		{
            int x = vtag[i].wx;
            int y = height()*dpr - vtag[i].wy;
			painter.setPen(Qt::black);

			painter.drawText(x + 3, y - 2, vtag[i].sztag);

			if (vtag[i].ntag == 0) painter.setPen(Qt::yellow);
			else painter.setPen(Qt::red);

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

void CGLView::RenderRigidLabels()
{
	CModelDocument* pdoc = m_pWnd->GetModelDocument();
	if (pdoc == nullptr) return;

	FSModel* fem = pdoc->GetFSModel();
	if (fem == nullptr) return;

	VIEW_SETTINGS& view = GetViewSettings();

	vector<GLTAG> vtag;

	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		FSRigidMaterial* rb = dynamic_cast<FSRigidMaterial*>(mat->GetMaterialProperties());
		if (rb)
		{
			GLTAG tag;
			tag.r = rb->GetCenterOfMass();
			tag.bvis = false;
			tag.ntag = 0;

			string name = mat->GetName();
			int l = name.size(); if (l > 63) l = 63;
			if (l > 0)
			{
				strncpy(tag.sztag, name.c_str(), l);
				tag.sztag[l] = 0;
			}
			else sprintf(tag.sztag, "_no_name");
			vtag.push_back(tag);
		}
	}
	int nsel = vtag.size();
	if (nsel == 0) return;

	// find out where the tags are on the screen
	GLViewTransform transform(this);
	for (int i = 0; i < nsel; i++)
	{
		vec3d p = transform.WorldToScreen(vtag[i].r);
		vtag[i].wx = p.x;
		vtag[i].wy = m_viewport[3] - p.y;
		vtag[i].bvis = true;
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

	for (int i = 0; i < nsel; i++)
		if (vtag[i].bvis)
		{
			glBegin(GL_POINTS);
			{
				glColor3ub(0, 0, 0);
				glVertex2f(vtag[i].wx, vtag[i].wy);
				if (vtag[i].ntag == 0) glColor3ub(255, 255, 0);
				else glColor3ub(255, 0, 0);
				glVertex2f(vtag[i].wx - 1, vtag[i].wy + 1);
			}
			glEnd();
		}

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setFont(QFont("Helvetica", 10));
	for (int i = 0; i < nsel; ++i)
		if (vtag[i].bvis)
		{
			double dpr = GetDevicePixelRatio();
			int x = vtag[i].wx;
			int y = (int)(height()*dpr - vtag[i].wy);
			painter.setPen(Qt::black);

			painter.drawText(x + 3, y - 2, vtag[i].sztag);

			if (vtag[i].ntag == 0) painter.setPen(Qt::yellow);
			else painter.setPen(Qt::red);

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

void CGLView::UpdatePlaneCut(bool breset)
{
	if (m_planeCut) delete m_planeCut;

	CModelDocument* doc = m_pWnd->GetModelDocument();
	if (doc == nullptr) return;

	FSModel& fem = *doc->GetFSModel();

	GModel& mdl = *doc->GetGModel();
	if (mdl.Objects() == 0) return;

	// set the plane normal
	vec3d norm(m_plane[0], m_plane[1], m_plane[2]);
	double ref = -m_plane[3];

	int edge[15][2], edgeNode[15][2], etag[15];

	VIEW_SETTINGS& vs = GetViewSettings();

	double vmin, vmax;

	m_planeCut = new GLMesh;

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

	if ((m_planeCutMode == 0) && (m_showPlaneCut))
	{
		GObject* poa = m_pWnd->GetActiveObject();

		for (int i = 0; i < mdl.Objects(); ++i)
		{
			GObject* po = mdl.Object(i);
			if (po->GetFEMesh())
			{
				FSMesh* mesh = po->GetFEMesh();

				vec3d ex[8];
				int en[8];
				GLColor ec[8];

				bool showContour = false;
				Mesh_Data& data = mesh->GetMeshData();
				if ((po == poa) && (vs.m_bcontour))
				{
					showContour = (vs.m_bcontour && data.IsValid());
					if (showContour) { data.GetValueRange(vmin, vmax); m_colorMap.SetRange((float)vmin, (float)vmax); }
				}

				// repeat over all elements
				GLColor defaultColor(200, 200, 200);
				GLColor c(defaultColor);
				int matId = -1;
				int NE = mesh->Elements();
				for (int i = 0; i < NE; ++i)
				{
					// render only when visible
					FSElement& el = mesh->Element(i);
					GPart* pg = po->Part(el.m_gid);
					if (el.IsVisible() && el.IsSolid() && (pg && pg->IsVisible()))
					{
						int mid = pg->GetMaterialID();
						if (mid != matId)
						{
							GMaterial* pmat = fem.GetMaterialFromID(mid);
							if (pmat)
							{
								c = fem.GetMaterialFromID(mid)->Diffuse();
								matId = mid;
							}
							else
							{
								matId = -1;
								c = defaultColor;
							}
						}


						const int *nt = nullptr;
						switch (el.Type())
						{
						case FE_HEX8: nt = HEX_NT; break;
						case FE_HEX20: nt = HEX_NT; break;
						case FE_HEX27: nt = HEX_NT; break;
						case FE_PENTA6: nt = PEN_NT; break;
						case FE_PENTA15: nt = PEN_NT; break;
						case FE_TET4: nt = TET_NT; break;
						case FE_TET5: nt = TET_NT; break;
						case FE_TET10: nt = TET_NT; break;
						case FE_TET15: nt = TET_NT; break;
						case FE_TET20: nt = TET_NT; break;
						case FE_PYRA5: nt = PYR_NT; break;
                        case FE_PYRA13: nt = PYR_NT; break;
						default:
							assert(false);
						}

						// get the nodal values
						for (int k = 0; k < 8; ++k)
						{
							FSNode& node = mesh->Node(el.m_node[nt[k]]);
							ex[k] = mesh->LocalToGlobal(node.r);
							en[k] = el.m_node[nt[k]];
						}

						if (showContour)
						{
							for (int k = 0; k < 8; ++k)
							{
								if (data.GetElementDataTag(i) > 0)
									ec[k] = m_colorMap.map(data.GetElementValue(i, nt[k]));
								else
									ec[k] = GLColor(212, 212, 212);
							}
						}

						// calculate the case of the element
						int ncase = 0;
						for (int k = 0; k < 8; ++k)
							if (norm*ex[k] > ref*0.999999) ncase |= (1 << k);

						// loop over faces
						int* pf = LUT[ncase];
						int ne = 0;
						for (int l = 0; l < 5; l++)
						{
							if (*pf == -1) break;

							// calculate nodal positions
							vec3d r[3];
							float w1, w2, w;
							for (int k = 0; k < 3; k++)
							{
								int n1 = ET_HEX[pf[k]][0];
								int n2 = ET_HEX[pf[k]][1];

								w1 = norm * ex[n1];
								w2 = norm * ex[n2];

								if (w2 != w1)
									w = (ref - w1) / (w2 - w1);
								else
									w = 0.f;

								r[k] = ex[n1] * (1 - w) + ex[n2] * w;
							}

							int nf = m_planeCut->Faces();
							m_planeCut->AddFace(r, (el.IsSelected() ? 1 : 0));
							GMesh::FACE& face = m_planeCut->Face(nf);
							if (po == poa)
							{
								face.eid = i;
							}

							if (showContour)
							{
								GLColor c;
								for (int k = 0; k < 3; k++)
								{
									int n1 = ET_HEX[pf[k]][0];
									int n2 = ET_HEX[pf[k]][1];

									w1 = norm * ex[n1];
									w2 = norm * ex[n2];

									if (w2 != w1)
										w = (ref - w1) / (w2 - w1);
									else
										w = 0.f;

									c.r = (Byte)((double)ec[n1].r * (1.0 - w) + (double)ec[n2].r * w);
									c.g = (Byte)((double)ec[n1].g * (1.0 - w) + (double)ec[n2].g * w);
									c.b = (Byte)((double)ec[n1].b * (1.0 - w) + (double)ec[n2].b * w);

									face.c[k] = c;
								}
							}
							else
							{
								face.c[0] = face.c[1] = face.c[2] = c;
							}

							// add edges (for mesh rendering)
							for (int k = 0; k < 3; ++k)
							{
								int n1 = pf[k];
								int n2 = pf[(k + 1) % 3];

								bool badd = true;
								for (int m = 0; m < ne; ++m)
								{
									int m1 = edge[m][0];
									int m2 = edge[m][1];
									if (((n1 == m1) && (n2 == m2)) ||
										((n1 == m2) && (n2 == m1)))
									{
										badd = false;
										etag[m]++;
										break;
									}
								}

								if (badd)
								{
									edge[ne][0] = n1;
									edge[ne][1] = n2;
									etag[ne] = 0;

									GMesh::FACE& face = m_planeCut->Face(m_planeCut->Faces() - 1);
									edgeNode[ne][0] = face.n[k];
									edgeNode[ne][1] = face.n[(k + 1) % 3];
									++ne;
								}
							}
							pf += 3;
						}

						for (int k = 0; k < ne; ++k)
						{
							if (etag[k] == 0)
							{
								m_planeCut->AddEdge(edgeNode[k], 2, (el.IsSelected() ? 1 : 0));
							}
						}
					}
				}
			}
		}

		m_planeCut->Update();
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

void CGLView::RenderPlaneCut()
{
	if (m_planeCut == nullptr) return;

	CModelDocument* doc = m_pWnd->GetModelDocument();
	if (doc == nullptr) return;

	BOX box = doc->GetGModel()->GetBoundingBox();

	glColor3ub(200, 0, 200);
	RenderBox(box, false);

	FSModel& fem = *doc->GetFSModel();
	int MAT = fem.Materials();

	GLMeshRender& mr = GetMeshRenderer();

	bool oldFaceColorMode = mr.GetFaceColor();

	// render the unselected faces
	glColor3ub(255, 255, 255);
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	mr.SetFaceColor(true);
	mr.RenderGLMesh(m_planeCut, 0);

	// render the selected faces
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_STIPPLE);
	glColor3ub(255, 0, 0);
	mr.SetFaceColor(false);
	mr.RenderGLMesh(m_planeCut, 1);

	mr.SetFaceColor(oldFaceColorMode);

	if (GetViewSettings().m_bmesh)
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColor3ub(0, 0, 0);

		CGLCamera& cam = doc->GetView()->GetCamera();
		cam.LineDrawMode(true);
		cam.Transform();
		
		GetMeshRenderer().RenderGLEdges(m_planeCut, 0);
		glDisable(GL_DEPTH_TEST);
		glColor3ub(255, 255, 0);
		GetMeshRenderer().RenderGLEdges(m_planeCut, 1);

		cam.LineDrawMode(false);
		cam.Transform();
	}
	glPopAttrib();
}
