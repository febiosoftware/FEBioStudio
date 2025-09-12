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
#include <functional>
#include <FSCore/color.h>
#include <QFont>
#include <map>

class CGLView;
class CGLWidgetManager;
class QPainter;

enum GLWAlign {
	GLW_ALIGN_LEFT		= 0x0001,
	GLW_ALIGN_RIGHT		= 0x0002,
	GLW_ALIGN_TOP		= 0x0004,
	GLW_ALIGN_BOTTOM	= 0x0008,
	GLW_ALIGN_HCENTER	= 0x0010,
	GLW_ALIGN_VCENTER	= 0x0020
};

enum GLWEvent {
	GLW_PUSH,
	GLW_DRAG,
	GLW_RELEASE
};

class GLWidget;

typedef std::function<void(GLWidget* w, int nevent)> glw_event_handler;

class GLWidget
{
public:
	enum FillMode { FILL_NONE, FILL_COLOR1, FILL_COLOR2, FILL_HORIZONTAL, FILL_VERTICAL };
	enum LineMode { LINE_NONE, LINE_SOLID };

public:
	GLWidget(int x, int y, int w, int h, const char* szlabel = 0);
	virtual ~GLWidget();

	virtual void draw(QPainter* painter);

	virtual int handle(int x, int y, int nevent) { return 0; }

	void set_fg_color(const GLColor& c, bool setoverrideflag = true) { m_fgc = c; if (setoverrideflag) m_boverridefgc = true; }
	GLColor get_fg_color() { return m_fgc; }
	bool isfgc_overridden() const { return m_boverridefgc; }

	void set_bg_style(int n) { m_bgFillMode = n; }
	void set_bg_color(GLColor c1, GLColor c2) { m_bgFillColor[0] = c1; m_bgFillColor[1] = c2; }
	void set_bg_color(GLColor c) { m_bgFillColor[0] = c; }
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
	}

	void show() { m_bshow = true; }
	void hide() { m_bshow = false; if (this == m_pfocus) m_pfocus = 0; }
	bool visible() { return m_bshow; }

	bool resizable() const { return m_resizable; }

	unsigned int GetSnap() { return m_nsnap; }

	void align(unsigned int n) { m_nsnap = n; }

	QFont get_font() const { return m_font; }
	void set_font(const QFont& f) { m_font = f; }
	void set_font_size(int nsize) { m_font.setPointSize(nsize); }

public:
	void call_event_handlers(int nevent);
	void add_event_handler(glw_event_handler f) { m_eventHandlers.push_back(f); }

protected:
	void draw_bg(int x0, int y0, int x1, int y1, QPainter* painter);

	void snap_to_bounds(QPainter& painter);

public:
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

	bool m_resizable = true;

	bool	m_boverridefgc;	// flag to see if fg color was overridden.
	
	char*			m_szlabel;

	QFont	m_font;	// label font
	
	unsigned int	m_nsnap;	// alignment flag

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

	std::vector<glw_event_handler> m_eventHandlers;

	CGLWidgetManager* m_parent = nullptr;
	friend class CGLWidgetManager;
};
