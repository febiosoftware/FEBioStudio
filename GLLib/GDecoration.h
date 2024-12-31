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
#include <FSCore/box.h>
#include <vector>

class GLRenderEngine;

// A GDecoration is something that will be drawn onto the Graphics View
class GDecoration
{
public:
	GDecoration() { bvisible = true; m_col = GLColor(255, 255, 0); m_col2 = GLColor(0, 0, 0); }
	virtual ~GDecoration(){}

	virtual void render(GLRenderEngine& re) = 0;

	bool isVisible() const { return bvisible; }
	void setVisible(bool b) { bvisible = b; }

	void setColor(GLColor c) { m_col = c; }
	void setColor2(GLColor c) { m_col2 = c; }

protected:
	bool	bvisible;
	GLColor	m_col, m_col2;
};

//-------------------------------------------------------------------
class GPointDecoration : public GDecoration
{
public:
	GPointDecoration() { m_renderAura = false; }
	GPointDecoration(const vec3f& r) : m_pos(r) { m_renderAura = false; }

	void render(GLRenderEngine& re) override;

	void setPosition(const vec3f& r) { m_pos = r; }

	vec3f& position() { return m_pos; }

	void renderAura(bool b) { m_renderAura = b; }

private:
	vec3f	m_pos;
	bool	m_renderAura;
};

//-------------------------------------------------------------------
class GLineDecoration : public GDecoration
{
public:
	GLineDecoration() { p1 = p2 = 0; m_del = false; }
	GLineDecoration(GPointDecoration* point1, GPointDecoration* point2) : p1(point1), p2(point2), m_del(false) {}
	GLineDecoration(const vec3f& a, const vec3f& b);
	~GLineDecoration();

	void render(GLRenderEngine& re) override;

private:
	bool	m_del;
	GPointDecoration* p1;
	GPointDecoration* p2;
};

//-------------------------------------------------------------------
class GTriangleDecoration : public GDecoration
{
public:
	GTriangleDecoration() { p1 = p2 = p3 = 0; m_del = false; }
	~GTriangleDecoration()
	{
		if (m_del)
		{
			delete p1;
			delete p2;
			delete p3;
		}
	}

	GTriangleDecoration(GPointDecoration* point1, GPointDecoration* point2, GPointDecoration* point3) : p1(point1), p2(point2), p3(point3) { m_del = false; }
	GTriangleDecoration(const vec3f& a, const vec3f& b, const vec3f& c)
	{
		m_del = true;
		p1 = new GPointDecoration(a);
		p2 = new GPointDecoration(b);
		p3 = new GPointDecoration(c);
	}

	void render(GLRenderEngine& re) override;

private:
	bool m_del;
	GPointDecoration* p1;
	GPointDecoration* p2;
	GPointDecoration* p3;
};


//-------------------------------------------------------------------
class GArcDecoration : public GDecoration
{
public:
	GArcDecoration(const vec3f& c, const vec3f& p0, const vec3f& p1, int ndivs = 10, double scale = 0.25);

	void render(GLRenderEngine& re) override;

private:
	vec3f	m_c;
	vec3f	m_e0;
	vec3f	m_e1;

	double	m_scale;
	int		m_divs;
};

//-------------------------------------------------------------------
class GSphereDecoration : public GDecoration
{
public:
	GSphereDecoration(const vec3f& a, double R);

	void render(GLRenderEngine& re) override;

private:
	vec3d	m_c;
	double	m_R;
};

//-------------------------------------------------------------------
class GCompositeDecoration : public GDecoration
{
public:
	GCompositeDecoration();
	~GCompositeDecoration();
	void AddDecoration(GDecoration* deco);

	void render(GLRenderEngine& re) override;

private:
	std::vector<GDecoration*>	m_deco;
};

//-------------------------------------------------------------------
class GPlaneCutDecoration : public GDecoration
{
public:
	GPlaneCutDecoration();
	~GPlaneCutDecoration();

	void setBoundingBox(BOX box);
	void setPlane(double n0, double n1, double n2, double d);

	void render(GLRenderEngine& re) override;

private:
	BOX		m_box;
	double	m_a[4];
};
