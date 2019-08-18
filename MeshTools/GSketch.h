#pragma once
#include <MathLib/math3d.h>
#include <FSCore/FSObject.h>
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// This class defines a 2D sketch
class GSketch
{
public:
	// edge types
	enum { LINE, CIRCLE, ARC };

public:
	struct POINT
	{
		vec2d	r;
		int		ntag;
	};

	struct EDGE
	{
		int		ntype;
		int		n[2];

		int		nc;	// for arcs, circles
		double	R;	// for circles
	};

public:
	GSketch(void);
	~GSketch(void);

	void Clear();

	void Render();

	int AddPoint(vec2d r);
	void AddCircle(vec2d c, double R);
	void AddLine(vec2d a, vec2d b);
	void AddArc(vec2d a, vec2d b, vec2d c);

	int Points() { return (int) m_pt.size(); }
	POINT& Point(int i) { return m_pt[i]; }

	int Edges() { return (int) m_edge.size(); }
	EDGE& Edge(int i) { return m_edge[i]; }

protected:
	double Project(EDGE& e, vec2d r, double& D, vec2d& q);
	double Intersect(EDGE& a, EDGE& b, vec2d& q);
	void Split(int ie, double w, int n);
	void InsertEdge(EDGE& e);

protected:
	vector<POINT>	m_pt;
	vector<EDGE>	m_edge;
};

//-----------------------------------------------------------------------------
class GCircle  : public FSObject {};
class GP2PLine : public FSObject {};
class G3PArc   : public FSObject {};
class GExtrude : public FSObject {};
class GRevolve : public FSObject {};
