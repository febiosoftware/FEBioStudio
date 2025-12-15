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
#include <FSCore/math3d.h>
#include <FSCore/color.h>

//-----------------------------------------------------------------------------
// render mode
enum RenderMode {
	RENDER_SOLID = 0,
	RENDER_WIREFRAME = 1
};

// background styles
enum BackgroundStyle {
	BG_COLOR1 = 0,
	BG_COLOR2 = 1,
	BG_HORIZONTAL = 2,
	BG_VERTICAL = 3
};

// Tag Info options
enum TagInfoOption {
	NO_TAG_INFO,
	TAG_ITEM,
	TAG_ITEM_AND_NODES
};

enum ExplodeDirection {
	EXPLODE_X,
	EXPLODE_Y,
	EXPLODE_Z,
};

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

enum Planecut_Mode
{
	PLANECUT,
	HIDE_ELEMENTS
};

//! view settings
struct GLViewSettings
{
	// background
	GLColor	m_col1, m_col2;		//!< background colors
	GLColor	m_fgcol;			//!< foreground color
	int		m_nbgstyle;			//!< back ground style

	// display
	bool	m_bgrid;	//!< show grid flag
	bool	m_bmesh;	//!< show edged faces
	bool	m_bfeat;	//!< show feature edges
	bool	m_bnorm;	//!< show face normals
	double	m_scaleNormals;	//!< scale factor for normals
	int		m_nrender;	//!< render mode
	int     m_nconv;    //!< multiview projection convention
	bool	m_bcontour;	//!< show contour plot
	GLColor	m_meshColor;		//!< mesh line color
	float	m_node_size;		//!< size of nodes when displayed
	float	m_line_size;		//!< line size
	bool	m_bline_smooth;		//!< line smoothing flag
	bool	m_bpoint_smooth;	//!< point smoothing flag

	int		m_defaultFGColorOption;	//!< determines how default FG color for widgets is set (0=theme, 1=user)
	GLColor	m_defaultFGColor;		//!< the default FG color (when m_defaultFGColorOption == 1)
	bool	m_snapToGrid;		//!< snap to grid
	bool	m_snapToNode;		//!< snap to nodes
	bool	m_identifyBackfacing;

	bool	m_bTags;
	int		m_ntagInfo;	// amount of info shown on tags
	int		m_tagFontSize;	// font size used for tags
	vec3d	m_pos3d;	// The location of the 3D cursor
	int		m_transparencyMode;		// 0 = off, 1 = selected only, 2 = unselected only
	bool	m_showHighlights;

	bool m_explode; // enable/disable exploded view
	int m_explode_direction;
	double m_explode_strength;

	bool		m_showPlaneCut = false;
	int			m_planeCutMode = 0;
	double		m_planeCut[4] = { 0 };

	View_Mode	m_nview = VIEW_USER;

	// Lighting
	bool	m_bLighting;	// use lighting or not
	float	m_ambient;		// scene light ambient intensity
	float	m_diffuse;		// scene light diffuse inentisty
	bool	m_use_environment_map;	// use the environment map (if one is provided)
	vec3f	m_light;

	// Physics
	bool	m_bjoint;	//!< show rigid joints
	bool	m_bwall;	//!< show rigid walls
	bool	m_brigid;	//!< show rigid kinematics
	bool	m_bfiber;	//!< show material fibers
	int		m_fibColor;
	int		m_fibLineStyle;
	bool	m_blma;		//!< show local material axes
	double	m_fiber_scale;	//!< scale factor for rendering fibers
	double	m_fiber_width;	//!< line width
	double	m_fiber_density; //!< density of fibers
	bool	m_showHiddenFibers;	//!< show fibers/axes on hidden parts
	bool	m_showSelectFibersOnly;	//!< only show fibers on selected objects
	bool	m_showDiscrete;		//!< render discrete sets
	bool	m_showRigidLabels;	//!< show labels on rigid bodies

	// Selection
	bool	m_bconn;	//!< select connected
	bool	m_bmax;		//!< max angle constraint for select connected
	bool	m_bpart;	//!< respect partition boundaries flag
	bool	m_bext;		//!< ignore interior nodes
	bool	m_bsoft;	//!< soft selection mode
	bool	m_bcullSel;	//!< ignore backfacing when selecting
	bool	m_bselbrush;//!< brush selection mode
	float	m_brushSize;//!< size of brush
	bool	m_bselpath;	//!< select by shortest path
	float	m_fconn;	//!< max connectivity angle (for selecting faces)
	bool	m_selectAndHide;

	// UI
	int		m_apply;	//!< emulate apply via middle mouse button

	GLViewSettings() { Defaults(0); }
	void Defaults(int ntheme = 0);
};
