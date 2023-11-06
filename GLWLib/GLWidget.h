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

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <QPainter>
#include <PostLib/ColorMap.h>
#include <FSCore/math3d.h>

class CGLView;
class CGLWidgetManager;

#define GLW_ALIGN_LEFT		0x0001
#define GLW_ALIGN_RIGHT		0x0002
#define GLW_ALIGN_TOP		0x0004
#define GLW_ALIGN_BOTTOM	0x0008
#define GLW_ALIGN_HCENTER	0x0010
#define GLW_ALIGN_VCENTER	0x0020

//-----------------------------------------------------------------------------
class GLWidget
{
public:
	enum FillMode { FILL_NONE, FILL_COLOR1, FILL_COLOR2, FILL_HORIZONTAL, FILL_VERTICAL };
	enum LineMode { LINE_NONE, LINE_SOLID };

public:
	GLWidget(int x, int y, int w, int h, const char* szlabel = 0);
	virtual ~GLWidget();

	virtual void draw(QPainter* painter) = 0;

	void set_color(GLColor fgc, GLColor bgc);

	void set_fg_color(GLColor c, bool setoverrideflag = true) { m_fgc = c; if (setoverrideflag) m_boverridefgc = true; }
	void set_fg_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255) { m_fgc = GLColor(r,g,b,a); }
	GLColor get_fg_color() { return m_fgc; }
	bool isfgc_overridden() const { return m_boverridefgc; }

	void set_bg_style(int n) { m_bgFillMode = n; }
	void set_bg_color(GLColor c1, GLColor c2) { m_bgFillColor[0] = c1; m_bgFillColor[1] = c2; }
	void set_bg_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255) { m_bgFillColor[0] = GLColor(r,g,b,a); }
	GLColor get_bg_color(int i) { return m_bgFillColor[i]; }
	int get_bg_style() { return m_bgFillMode; }

	void set_bg_line_style(int n) { m_bgLineMode = n; }
	void set_bg_line_color(GLColor c) { m_bgLineColor = c; }
	int get_bg_line_style() const { return m_bgLineMode; }
	GLColor get_bg_line_color() const { return m_bgLineColor; }
	void set_bg_line_size(int n) { m_bgLineSize = n; }
	int get_bg_line_size() const { return m_bgLineSize; }

	void copy_label(const char* szlabel);
	void set_label(const char* szlabel);
	const char* get_label() { return m_szlabel; }

	virtual bool is_inside(int x, int y);

	void set_focus() { m_pfocus = this; }
	static void set_focus(GLWidget* pw) { m_pfocus = pw; }

	static GLWidget* get_focus() { return m_pfocus; }

	bool has_focus() { return (this == m_pfocus); }

	int x() { return m_x; }
	int y() { return m_y; }
	int w() { return m_w; }
	int h() { return m_h; }

	virtual void resize(int x, int y, int w, int h)
	{
		m_x = x;
		m_y = y;
		m_w = w;
		m_h = h;

		if (m_w < m_minw) m_w = m_minw;
		if (m_h < m_minh) m_h = m_minh;

		m_nsnap = 0;
	}

	void show() { m_bshow = true; }
	void hide() { m_bshow = false; if (this == m_pfocus) m_pfocus = 0; }
	bool visible() { return m_bshow; }

	unsigned int GetSnap() { return m_nsnap; }

	void align(unsigned int n) { m_nsnap = n; }

	QFont get_font() const { return m_font; }
	void set_font(const QFont& f) { m_font = f; }
	void set_font_size(int nsize) { m_font.setPointSize(nsize); }

public:
	unsigned int layer() const { return m_layer; }
	void set_layer(unsigned int l) { m_layer = l; }

	std::string processLabel() const;

public:
	static void set_base_color(GLColor c) { m_base = c; }
	static void set_default_font(const QFont& font) { m_defaultFont = font; }
	static const QFont& get_default_font() { return m_defaultFont; }

public:
	static void clearStringTable();
	static void addToStringTable(const std::string& key, const std::string& value);
	static void addToStringTable(const std::string& key, double value);
	static std::string getStringTableValue(const std::string& key);

protected:
	int m_x, m_y;
	int m_w, m_h;
	int	m_minw, m_minh;
	bool	m_balloc;

	bool	m_boverridefgc;	// flag to see if fg color was overridden.
	
	char*			m_szlabel;

	QFont	m_font;	// label font
	
	unsigned int	m_nsnap;	// alignment flag

	unsigned int m_layer;

	GLColor	m_fgc;
	GLColor m_bgFillColor[2], m_bgLineColor;
	int		m_bgFillMode;	// background fille style
	int		m_bgLineMode;	// background line style
	int		m_bgLineSize;

	static GLWidget* m_pfocus;	// the widget that has the focus

	static	GLColor	m_base;	// base color

	static QFont	m_defaultFont;

	bool	m_bshow;	// show the widget or not

	static	std::map<std::string, std::string>	m_stringTable;

	friend class CGLWidgetManager;
};

//-----------------------------------------------------------------------------

class GLBox : public GLWidget
{
public:
	enum AlignOption {
		LeftJustified,
		Centered,
		RightJustified
	};

public:
	GLBox(int x, int y, int w, int h, const char* szlabel = 0);

	void draw(QPainter* painter);

	void fit_to_size();

protected:
	void draw_bg(int x0, int y0, int x1, int y1, QPainter* painter);

public:
	bool	m_bshadow;	// render shadows
	GLColor	m_shc;		// shadow color
	int		m_margin;
	int		m_align;
};

//-----------------------------------------------------------------------------

class GLLegendBar : public GLWidget
{
public:
	enum { GRADIENT, DISCRETE };
	enum { ORIENT_HORIZONTAL, ORIENT_VERTICAL };

public:
	GLLegendBar(Post::CColorTexture* pm, int x, int y, int w, int h, int orientation = ORIENT_VERTICAL);

	void draw(QPainter* painter);

	void SetType(int n) { m_ntype = n; }

	void SetOrientation(int n);
	int Orientation() const { return m_nrot; }

	bool ShowLabels() { return m_blabels; }
	void ShowLabels(bool bshow) { m_blabels = bshow; }

	bool ShowTitle() const { return m_btitle; }
	void ShowTitle(bool b) { m_btitle = b; }

	int GetPrecision() { return m_nprec; }
	void SetPrecision(int n) { n = (n<1?1:(n>7?7:n)); m_nprec = n; }

	int GetLabelPosition() const { return m_labelPos; }
	void SetLabelPosition(int n) { m_labelPos = n; }

	void SetRange(float fmin, float fmax);
	void GetRange(float& fmin, float& fmax);

	int GetDivisions();
	void SetDivisions(int n);

	bool SmoothTexture() const;
	void SetSmoothTexture(bool b);

	float LineThickness() const { return m_lineWidth; }
	void SetLineThickness(float f) { m_lineWidth = f; }

protected:
	void draw_gradient_vert(QPainter* painter);
	void draw_gradient_horz(QPainter* painter);
	void draw_discrete_vert(QPainter* painter);
	void draw_discrete_horz(QPainter* painter);

	void draw_bg(int x0, int y0, int x1, int y1, QPainter* painter);

protected:
	int		m_ntype;	// type of bar
	int		m_nrot;		// orientation
	bool	m_btitle;	// show title
	bool	m_blabels;	// show labels
	int		m_labelPos;	// label placement
	int		m_nprec;	// precision
	float	m_fmin;		// min of range
	float	m_fmax;		// max of range
	float	m_lineWidth;	// line width

	Post::CColorTexture*		m_pMap;
};

//-----------------------------------------------------------------------------
class GLTriad : public GLWidget
{
public:
	GLTriad(int x, int y, int w, int h);

	void draw(QPainter* painter);

	void show_coord_labels(bool bshow) { m_bcoord_labels = bshow; }
	bool show_coord_labels() { return m_bcoord_labels; }

	void setOrientation(const quatd& q) { m_rot = q; }

protected:
	quatd	m_rot;
	bool	m_bcoord_labels;
};

//-----------------------------------------------------------------------------

class GLSafeFrame : public GLWidget
{
public:
	enum STATE {
		FREE,
		FIXED_SIZE,
		LOCKED
	};

public:
	GLSafeFrame(int x, int y, int w, int h);

	void resize(int x, int y, int W, int H) override;

	bool is_inside(int x, int y) override;

	void draw(QPainter* painter) override;

	void SetState(STATE state) { m_state = state; }
	int GetState() { return m_state; }

protected:
	int		m_state;
};
