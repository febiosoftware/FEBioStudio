#pragma once
#include <MathLib/math3d.h>
#include <FSCore/color.h>

//-------------------------------------------------------------------
// A GDecoration is something that will be drawn onto the Graphics View
class GDecoration
{
public:
	GDecoration() { bvisible = true; m_col = GLColor(255, 255, 0); }
	virtual ~GDecoration(){}

	virtual void render() = 0;

	bool isVisible() const { return bvisible; }
	void setVisible(bool b) { bvisible = b; }

protected:
	bool	bvisible;
	GLColor	m_col;
};

//-------------------------------------------------------------------
class GPointDecoration : public GDecoration
{
public:
	GPointDecoration() {}
	GPointDecoration(const vec3f& r) : pos(r) {}

	void render();

	void setPosition(const vec3f& r) { pos = r; }

	vec3f& position() { return pos; }

private:
	vec3f	pos;
};

//-------------------------------------------------------------------
class GLineDecoration : public GDecoration
{
public:
	GLineDecoration() { p1 = p2 = 0; m_del = false; }
	GLineDecoration(GPointDecoration* point1, GPointDecoration* point2) : p1(point1), p2(point2), m_del(false) {}
	GLineDecoration(const vec3f& a, const vec3f& b);
	~GLineDecoration();

	void render();

private:
	bool	m_del;
	GPointDecoration* p1;
	GPointDecoration* p2;
};

//-------------------------------------------------------------------
class GArcDecoration : public GDecoration
{
public:
	GArcDecoration(vec3d& c, vec3d& p0, vec3d& p1, int ndivs = 10, double scale = 0.25);

	void render();

private:
	vec3d	m_c;
	vec3d	m_e0;
	vec3d	m_e1;

	double	m_scale;
	int		m_divs;
};

//-------------------------------------------------------------------
class GCompositeDecoration : public GDecoration
{
public:
	GCompositeDecoration();
	~GCompositeDecoration();
	void AddDecoration(GDecoration* deco);

	void render();

private:
	std::vector<GDecoration*>	m_deco;
};
