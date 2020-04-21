#pragma once
#include <vector>
#include <MathLib/math3d.h>

class GModel;
class GDiscreteElementSet;

class CSpringGenerator
{
public:
	CSpringGenerator(GModel& model);

	void ProjectToLines(bool b);

	bool generate(GDiscreteElementSet* po, const std::vector<int>& node1, const std::vector<int>& node2);

	bool generate(const std::vector<vec3d>& node1, const std::vector<vec3d>& node2, std::vector<pair<int,int> >& springs);

private:
	GModel&	m_model;
	bool	m_projectToLines;
};
