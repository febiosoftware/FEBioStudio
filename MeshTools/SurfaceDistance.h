#pragma once
#include <vector>
using namespace std;

class GObject;

//-----------------------------------------------------------------------------
// This class measures the distance between two (shell) meshes and assigns
// the distance to the element thickness.
class CSurfaceDistance
{
public:
	enum {
		NORMAL,
		CLOSEST_POINT,
	};

public:
	// constructor
	CSurfaceDistance();

	// set the max range of the distances
	void SetRange(double gmin, double gmax);

	// calculate signed or unsigned distance
	void SignedDistance(bool b) { m_bsigned = b; }
	bool SignedDistance() const { return m_bsigned; }

	// set the projection method
	void SetProjectionMethod(int n) { m_ntype = n; }

	// set the multiplier
	void SetMultiplier(double s) { m_scale = s; }

	void SetClamp(bool b) { m_bclamp = b; }

	bool Apply(GObject* pso, GObject* pmo);

protected:
	bool	m_bclamp;
	double	m_min, m_max;
	double	m_scale;
	bool	m_bsigned;
	int		m_ntype;

protected:
	bool NormalProject(GObject* pso, GObject* pmo, vector<double>& dist);
	bool ClosestPoint (GObject* pso, GObject* pmo, vector<double>& dist);

public:
	double	m_mean;
	double	m_stddev;
};
