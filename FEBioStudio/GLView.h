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
#include <RHILib/rhiSceneView.h>
#include <QNativeGestureEvent>
#include <GLLib/GLCamera.h>
#include <FSCore/ColorMap.h>
#include "CommandManager.h"
#include "GManipulator.h"
#include <GLWLib/GLWidgetManager.h>
#include <GLLib/GLContext.h>
#include <GLLib/GLViewSettings.h>
#include <GLLib/ColorTexture.h>
#include "GLViewSelector.h"
#include "GLScreenRecorder.h"
#include <list>

class CMainWindow;
class CGLDocument;
class GDecoration;
class CGView;
class FSModel;
class CGLView;
class GLScene;
class GPart;
class GLViewTransform;

class GLLabel;
class GLTriad;
class GLSafeFrame;
class GLLegendBar;
class GVContextMenu;
class GLRenderEngine;

// coordinate system modes
#define COORD_GLOBAL	0
#define COORD_LOCAL		1
#define COORD_SCREEN	2

// snap modes
enum Snap_Mode
{
	SNAP_NONE,
	SNAP_GRID
};

class CGLPivot
{
public:
	CGLPivot(CGLView* view);

	int GetSelectionMode() const { return m_mode; }

	void SetPosition(const vec3d& r) { m_pos = r; }

	vec3d GetPosition() const { return m_pos; }

	void Render(GLRenderEngine& re, int ntrans, double scale, bool bact);

	int Pick(int ntrans, int x, int y);

public:
	GTranslator		m_Ttor;	//!< the translate manipulator
	GRotator		m_Rtor;	//!< the rotate manipulator
	GScalor			m_Stor;	//!< the scale manipulator

	int		m_mode;		// pivot selection mode
	vec3d	m_pos;		// pivot point
};

//===================================================================
class CGLView : public rhiSceneView
{
	Q_OBJECT

public:
	CGLView(CMainWindow* pwnd, QWidget* parent = 0);
	~CGLView();

public:
	CGLDocument* GetDocument();

	GLScene* GetActiveScene() override;

	GLCamera* GetCamera();

	void UpdateScene();

	GObject* GetActiveObject();

	void Reset();

	void UpdateCamera(bool hitCameraTarget);

	void HighlightNode(int x, int y);
	void HighlightEdge(int x, int y);
	void HighlightSurface(int x, int y);
	void HighlightPart(int x, int y);

	GPart* PickPart(int x, int y);

	bool SelectPivot(int x, int y);

	void SetCoordinateSystem(int nmode);
	
	void UndoViewChange();

	void RedoViewChange();

	void ClearCommandStack();

	vec3d PickPoint(int x, int y, bool* success = 0);

	void SetViewMode(View_Mode n);

	void TogglePerspective(bool b);

	void ShowMeshData(bool b);

	void Set3DCursor(const vec3d& r) { m_view.m_pos3d = r; }
	vec3d Get3DCursor() const { return m_view.m_pos3d; }

	void ToggleFPS();

	void ToggleMeshLines(bool b);
	void ToggleGridLines(bool b);
	void ToggleFeatureEdges(bool b);
	void ToggleNormals(bool b);
	void ToggleWireframe(bool b);

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

	// render functions
public:
	// other rendering functions
	void RenderPivot(GLRenderEngine& re);

	void ShowSafeFrame(bool b);
	bool ShowSafeFrame() const;

	vec3d GetPickPosition();

	QRect CaptureFrameRect() const;

public:
	vec3d GetPivotPosition();
	quatd GetPivotRotation();

	void SetPivotPosition(const vec3d& r);

	bool GetPivotUserMode() const;
	void SetPivotUserMode(bool b);

public:
	void changeViewMode(View_Mode vm);

	CGLWidgetManager* GetGLWidgetManager() { return m_Widget; }
	void AllocateDefaultWidgets(bool b);

	void ToggleContextMenu();

protected:
	void customInit() override;

	void RenderScene(GLRenderEngine& re) override;

	void onFrameFinished() override;

private: // overlay rendering
	void RenderOverlay(GLRenderEngine& re, GLContext& rc);
	void RenderOverlayComponents(QPainter& painter);
	void RenderTags(QPainter& painter);
	void RenderDecorations(GLRenderEngine& re);
	void DrawWidgets(QPainter& painter);
	void RenderRubberBand(QPainter& painter);

private:
	void SetSnapMode(Snap_Mode snap) { m_nsnap = snap; }
	Snap_Mode GetSnapMode() { return m_nsnap; }

public:
	void CaptureScreen();

	void UpdateWidgets();

	bool isTitleVisible() const;
	void showTitle(bool b);

	bool isSubtitleVisible() const;
	void showSubtitle(bool b);

	void setRenderOverlay(bool b) { renderOverlay = b; }

public:
	void AddDecoration(GDecoration* deco);
	void RemoveDecoration(GDecoration* deco);

	GLScreenRecorder& GetScreenRecorder() { return m_recorder; }

	QSize GetSafeFrameSize() const;

	void LockSafeFrame();
	void UnlockSafeFrame();


public:
	void SetColorMap(unsigned int n);

	void AddRegionPoint(int x, int y);

public: // added to support new rhi base class
	GLViewSettings& GetViewSettings() { return m_view; }
	void repaint();
	void update();
	QRect rect() const;

public:
	bool StartRecording();
	bool PauseRecording();
	bool StopRecording();

public slots:
	void updateView();
	void captureFrameReady(QImage img);

signals:
	void captureFrameFinished(QImage img);

protected:
	CMainWindow*	m_pWnd;	// parent window

	CBasicCmdManager m_Cmd;	// view command history

	std::vector<std::pair<int, int> >		m_pl;
	int			m_x0, m_y0, m_x1, m_y1;
	int			m_xp, m_yp;
	int			m_dxp, m_dyp;
	Snap_Mode	m_nsnap;

	bool	m_showFPS;
	std::list<double>	m_fps;

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

	// GL widgets
	GLLabel*		m_ptitle;
	GLLabel*		m_psubtitle;
	GLTriad*		m_ptriad;
	GLSafeFrame*	m_pframe;
	GLLegendBar*	m_legend;	// main legend bar for colormaps
	GLLegendBar*	m_legendPlot; // secondary legend for active plot

	GVContextMenu* m_menu;

	CGLWidgetManager*	m_Widget;
	bool	m_ballocDefaultWidgets;

private:
	std::vector<GDecoration*>	m_deco;

public:
	GLContext	m_rc;

private:
	GLViewSelector	m_select;

	GLScreenRecorder	m_recorder;
	bool m_stopRequested = false;

	GLCamera	m_oldCam;

	bool renderOverlay = false;

	bool mouseIsPressed = false;

	int frameCapturesRequested = 0;

	static GLViewSettings	m_view;
};

bool intersectsRect(const QPoint& p0, const QPoint& p1, const QRect& rt);
