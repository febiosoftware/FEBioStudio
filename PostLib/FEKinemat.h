#pragma once
#include <list>
#include <MathLib/math3d.h>

namespace Post {
	class CGLModel;
}

//-----------------------------------------------------------------------------
//! This class implements a tool to apply kinematics data to a model
class FEKinemat
{
protected:
	struct KINE
	{
		double m[16];

		vec3d apply(const vec3d& r);
	};

	class STATE
	{
	public:
		vector<KINE>	D;

	public:
		STATE(){}
		STATE(const STATE& s) { D = s.D; }
		STATE& operator = (const STATE& s) { D = s.D; return (*this); }
	};

public:
	FEKinemat();

	bool Apply(Post::CGLModel* glm, const char* szkine);
	void SetRange(int n0, int n1, int ni);

protected:
	bool ReadKine(const char* szkine);
	bool BuildStates(Post::CGLModel* glm);

protected:
	int	m_n0, m_n1, m_ni;
	vector<STATE>	m_State;
};
