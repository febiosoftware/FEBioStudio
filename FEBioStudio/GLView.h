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
#include "GLSceneView.h"
#include <QNativeGestureEvent>
#include <GLLib/GLCamera.h>
#include "CommandManager.h"
#include "GManipulator.h"
#include "GTriad.h"
#include "GGrid.h"
#include "GLPlaneCut.h"
#include <GLWLib/GLWidgetManager.h>
#include "Animation.h"
#include <GLLib/GLContext.h>
#include <GLLib/GLViewSettings.h>
#include "GLViewSelector.h"
#include "GLScreenRecorder.h"

class CMainWindow;
class CGLDocument;
class GDecoration;
class CGView;
class FSModel;
class CGLView;
class CGLScene;

// coordinate system modes
#define COORD_GLOBAL	0
#define COORD_LOCAL		1
#define COORD_SCREEN	2

// preset views
enum View_Mode {
	VIEW_USER,
	VIEW_TOP,
	VIEW_BOTTOM,
	VIEW_LEFT,
	VIEW_RIGHT,
	VIEW_FRONT,
	VIEW_BACK,
	VIEW_ISOMETRIC
};

// view conventions
enum View_Convention {
	CONV_FR_XZ,
	CONV_FR_XY,
	CONV_US_XY
};

// snap modes
enum Snap_Mode
{
	SNAP_NONE,
	SNAP_GRID
};

enum Planecut_Mode
{
	PLANECUT,
	HIDE_ELEMENTS
};

class CGLPivot
{
public:
	CGLPivot(CGLView* view);

	int GetSelectionMode() const { return m_mode; }

	void SetPosition(const vec3d& r) { m_pos = r; }

	vec3d GetPosition() const { return m_pos; }

	void Render(int ntrans, double scale, bool bact);

	int Pick(int ntrans, int x, int y);

public:
	GTranslator		m_Ttor;	//!< the translate manipulator
	GRotator		m_Rtor;	//!< the rotate manipulator
	GScalor			m_Stor;	//!< the scale manipulator

	int		m_mode;		// pivot selection mode
	vec3d	m_pos;		// pivot point
};

// tag structure
struct GLTAG
{
	char	sztag[64];	// name of tag
	float	wx, wy;		// window coordinates for tag
	vec3d	r;			// world coordinates of tag
	GLColor	c;			// tag color
};

//===================================================================
class CGLView : public CGLSceneView
{
	Q_OBJECT

public:
	CGLView(CMainWindow* pwnd, QWidget* parent = 0);
	~CGLView();

public:
	CGLDocument* GetDocument();

	CGLScene* GetActiveScene() override;

	GObject* GetActiveObject();

	void Reset();

	void UpdateCamera(bool hitCameraTarget);

	void HighlightNode(int x, int y);
	void HighlightEdge(int x, int y);

	bool SelectPivot(int x, int y);

	void SetCoordinateSystem(int nmode);
	
	void UndoViewChange();

	void RedoViewChange();

	void ClearCommandStack();

	vec3d PickPoint(int x, int y, bool* success = 0);

	void SetViewMode(View_Mode n);
	View_Mode GetViewMode() { return m_nview; }

	void TogglePerspective(bool b);

	void ShowMeshData(bool b);

	void Set3DCursor(const vec3d& r) { m_view.m_pos3d = r; }
	vec3d Get3DCursor() const { return m_view.m_pos3d; }

	std::string GetOGLVersionString();

	void ToggleFPS();

protected:
	void mousePressEvent  (QMouseEvent* ev) override;
	void mouseMoveEvent   (QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void wheelEvent       (QWheelEvent* ev) override;
	void mouseDoubleClickEvent(QMouseEvent* ev) override;

    bool gestureEvent     (QNativeGestureEvent* ev);
    bool event            (QEvent* event) override;

	void keyPressEvent(QKeyEvent* ev) override;

signals:
	void pointPicked(const vec3d& p);
	void selectionChanged();

public:
	//! Zoom out on current selection
	void ZoomSelection(bool forceZoom = true);

	//! zoom in on a box
	void ZoomTo(const BOX& box);

	//! Zoom in on an object
	void ZoomToObject(GObject* po);

	// zoom to the models extents
	void ZoomExtents(bool banimate = true);

	// render functions
public:
	// other rendering functions
	void RenderRubberBand();
	void RenderBrush();
	void RenderPivot();

	void Render3DCursor();
	void RenderTags(std::vector<GLTAG>& tags);
	void RenderDecorations();

	void ShowSafeFrame(bool b);

	vec3d GetPickPosition();

public:
	vec3d GetPivotPosition();
	quatd GetPivotRotation();

	void SetPivotPosition(const vec3d& r);

	bool GetPivotUserMode() const;
	void SetPivotUserMode(bool b);

public:
	void changeViewMode(View_Mode vm);

	void ShowContextMenu(bool b);

	CGLWidgetManager* GetGLWidgetManager() { return m_Widget; }
	void AllocateDefaultWidgets(bool b);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;

	void RenderScene() override;

	void RenderCanvas(CGLContext& rc);

private:
	void SetSnapMode(Snap_Mode snap) { m_nsnap = snap; }
	Snap_Mode GetSnapMode() { return m_nsnap; }

	// convert from device pixel to physical pixel
	QPoint DeviceToPhysical(int x, int y);

public:
	QImage CaptureScreen();

	void UpdateWidgets();

	bool isTitleVisible() const;
	void showTitle(bool b);

	bool isSubtitleVisible() const;
	void showSubtitle(bool b);

	void setLegendRange(float vmin, float vmax);

public:
	void AddDecoration(GDecoration* deco);
	void RemoveDecoration(GDecoration* deco);

	void ShowPlaneCut(bool b);
	bool ShowPlaneCut() const;
	void SetPlaneCut(double d[4]);
	void SetPlaneCutMode(int nmode);
	void UpdatePlaneCut(bool breset = false);

	GLScreenRecorder& GetScreenRecorder() { return m_recorder; }

	QSize GetSafeFrameSize() const;

	void LockSafeFrame();
	void UnlockSafeFrame();

public:
	void SetColorMap(unsigned int n);

	Post::CColorMap& GetColorMap();

	void AddRegionPoint(int x, int y);

public:
	void RenderPlaneCut();

	bool ShowPlaneCut();

	GLPlaneCut& GetPlaneCut();

	void DeletePlaneCutMesh();

	int PlaneCutMode();

	double* PlaneCoordinates();

protected slots:
	void repaintEvent();

protected:
	CMainWindow*	m_pWnd;	// parent window

	CBasicCmdManager m_Cmd;	// view command history

	vector<pair<int, int> >		m_pl;
	int			m_x0, m_y0, m_x1, m_y1;
	int			m_xp, m_yp;
	int			m_dxp, m_dyp;
	View_Mode	m_nview;
	Snap_Mode	m_nsnap;

	bool	m_showFPS;
	double	m_fps;

	vec3d	m_rt;	// total translation
	vec3d	m_rg;

	double	m_st;	// total scale
	double	m_sa;	// accumulated scale
	vec3d	m_ds;	// direction of scale

	double	m_wt;	// total rotation
	double	m_wa;	// total accumulated rotation

	bool	m_bshift;
	bool	m_bctrl;
	bool	m_bsel;		// selection mode

public:
	bool	m_bpick;

protected:
	bool	m_bsnap;	// snap to grid

	int		m_coord;	// coordinate system

	CGLPivot m_pivot;
	bool	m_userPivot;

	// triad
	GLBox*			m_ptitle;
	GLBox*			m_psubtitle;
	GLTriad*		m_ptriad;
	GLSafeFrame*	m_pframe;
	GLLegendBar*	m_legend;

	CGLWidgetManager*	m_Widget;
	bool	m_ballocDefaultWidgets;

	bool	m_showContextMenu;

private:
	vector<GDecoration*>	m_deco;

public:
	CGLContext	m_rc;

private:
	GLViewSelector	m_select;

	GLScreenRecorder	m_recorder;

	CGLCamera	m_oldCam;

	Post::CColorTexture m_colorMap;	// color map used for rendering mesh data

	bool		m_showPlaneCut;
	int			m_planeCutMode;
	GLPlaneCut	m_planeCut;

	std::string		m_oglVersionString;
};

bool intersectsRect(const QPoint& p0, const QPoint& p1, const QRect& rt);
void SetModelView(GObject* po);
