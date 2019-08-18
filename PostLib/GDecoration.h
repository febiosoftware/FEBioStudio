#pragma once
#include "math3d.h"

//-------------------------------------------------------------------
// A GDecoration is something that will be drawn onto the Graphics View
class GDecoration
{
public:
	GDecoration(){ bvisible = true; }
	virtual ~GDecoration(){}

	virtual void render() = 0;

	bool isVisible() const { return bvisible; }
	void setVisible(bool b) { bvisible = b; }

private:
	bool	bvisible;
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
	GLineDecoration() { p1 = p2 = 0; }
	GLineDecoration(GPointDecoration* point1, GPointDecoration* point2) : p1(point1), p2(point2) {}

	void render();

private:
	GPointDecoration* p1;
	GPointDecoration* p2;
};
