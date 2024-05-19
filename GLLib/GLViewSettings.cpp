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
#include "GLViewSettings.h"

void GLViewSettings::Defaults(int ntheme)
{
	m_bgrid = true;
	m_bmesh = true;
	m_bfeat = true;
	m_bnorm = false;
	m_nrender = RENDER_SOLID;
	m_scaleNormals = 1.0;
	//	m_nconv = 0; // Don't reset this, since this is read from settings file. TODO: Put this option elsewhere. 

	m_selectAndHide = false;

	m_bjoint = true;
	m_bwall = true;
	m_brigid = true;
	m_bfiber = false;
	m_fibColor = 0;
	m_fibLineStyle = 0;
	m_fiber_scale = 1.0;
	m_fiber_width = 1.0;

	m_bcontour = false;
	m_blma = false;
	m_showHiddenFibers = false;
	m_showSelectFibersOnly = false;

	m_showDiscrete = true;
	m_showRigidLabels = true;

	m_bcull = false;
	m_bconn = false;
	m_bmax = true;
	m_bpart = true;
	m_bext = false;
	m_bsoft = false;
	m_fconn = 30.f;
	m_bcullSel = true;
	m_bselpath = false;
	m_bselbrush = false;
	m_brushSize = 50.f;

	m_apply = 0;

	m_pos3d = vec3d(0, 0, 0);

	m_bTags = true;
	m_ntagInfo = TagInfoOption::TAG_ITEM;

	m_defaultFGColorOption = 0;

	if (ntheme == 0)
	{
		m_col1 = GLColor(255, 255, 255);
		m_col2 = GLColor(128, 128, 255);
		m_nbgstyle = BG_HORIZONTAL;
		m_defaultFGColor = GLColor(0, 0, 0);
	}
	else
	{
		m_col1 = GLColor(83, 83, 83);
		m_col2 = GLColor(128, 128, 128);
		m_nbgstyle = BG_HORIZONTAL;
		m_defaultFGColor = GLColor(255, 255, 255);
	}

	m_meshColor = GLColor(0, 0, 128, 64);
	m_fgcol = GLColor(0, 0, 0);
	m_node_size = 3.f;
	m_line_size = 1.0f;
	m_bline_smooth = true;
	m_bpoint_smooth = true;
	m_bzsorting = true;

	m_snapToGrid = true;
	m_snapToNode = false;

	m_bLighting = true;
	m_bShadows = false;
	m_shadow_intensity = 0.5f;
	m_ambient = 0.09f;
	m_diffuse = 0.8f;
	m_use_environment_map = false;

	m_transparencyMode = 0; // = off

	m_showHighlights = true;
}
