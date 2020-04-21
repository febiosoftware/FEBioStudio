#include "stdafx.h"
#include "SpringGenerator.h"
#include "GDiscreteObject.h"
#include "GModel.h"

CSpringGenerator::CSpringGenerator(GModel& model) : m_model(model)
{
	m_projectToLines = false;
}

void CSpringGenerator::ProjectToLines(bool b)
{
	m_projectToLines = b;
}

void generate_springs(
	vector<pair<int, int> >& springs,
	const std::vector<vec3d>& r1, const std::vector<vec3d>& r2,
	std::vector<int>& tag1, std::vector<int>& tag2)
{
	int N1 = (int)r1.size();
	int N2 = (int)r2.size();

	for (int i = 0; i<N1; ++i)
	{
		if (tag1[i] == 0)
		{
			vec3d ri = r1[i];

			// we keep track of closest but node used (1), and true closest (2)
			double L1min = 1e99, L2min = 1e99;
			int minj1 = -1, minj2 = -1;
			for (int j = 0; j<N2; ++j)
			{
				vec3d rj = r2[j];
				double L = (ri - rj).SqrLength();

				// check closest, but not used
				if (tag2[j] == 0)
				{
					if ((L < L1min) || (minj1 == -1))
					{
						L1min = L;
						minj1 = j;
					}
				}

				// check true closest
				if ((L < L2min) || (minj2 == -1))
				{
					L2min = L;
					minj2 = j;
				}
			}

			// prefer the node that was not used
			int minj = (minj1 != -1 ? minj1 : minj2);
			if (minj != -1)
			{
				springs.push_back(pair<int, int>(i, minj));
				tag2[minj] = 1;
			}
		}
	}
}

vec3d projectToLine(vector<vec3d>& r, double y)
{
	int n = (int)r.size();
	if (n < 2) return vec3d(1, 0,0);

	// get the end points
	int n0 = 0, n1 = 0;
	double LMax = 0.0;
	for (int i=0; i<n; ++i)
	{
		double L = (r[i] - r[n0]).SqrLength();
		if (L > LMax)
		{
			n1 = i;
			LMax = L;
		}
	}
	n0 = n1;
	LMax = 0;
	for (int i = 0; i<n; ++i)
	{
		double L = (r[i] - r[n0]).SqrLength();
		if (L > LMax)
		{
			n1 = i;
			LMax = L;
		}
	}

	// get the line
	vec3d a = r[n0];
	vec3d b = r[n1];
	vec3d e = b - a; e.Normalize();

	// project onto the line
	double lmin = 0, lmax = 0;
	for (int i = 0; i<n; ++i)
	{
		vec3d c = r[i];
		double l = e*(c - a);
		r[i] = vec3d(l, y, 0);

		if (i == 0) lmin = lmax = l;
		else
		{
			if (l < lmin) lmin = l;
			if (l > lmax) lmax = l;
		}
	}
	if (lmax == lmin) lmax++;

	// map to unit line
	for (int i=0; i<n; ++i)
	{
		double l = r[i].x;
		l = (l - lmin) / (lmax - lmin);
		r[i].x = l;
	}

	return e;
}

bool CSpringGenerator::generate(GDiscreteElementSet* po, const std::vector<int>& node1, const std::vector<int>& node2)
{
	// get the sizes of the sets
	int N1 = (int) node1.size();
	int N2 = (int) node2.size();
	if ((N1 == 0) || (N2 == 0)) return false;

	// use tags to mark progress and try to prevent 
	// two springs connecting to the same node
	vector<int> tag1(N1, 0), tag2(N2, 0);

	// get all the nodes
	vector<GNode*> set1(N1);
	vector<vec3d> r1(N1);
	for (int i=0; i<N1; ++i)
	{
		GNode* ni = m_model.FindNode(node1[i]);
		if (ni == 0) return false;
		set1[i] = ni;
		r1[i] = ni->Position();
	}

	vector<GNode*> set2(N2);
	vector<vec3d> r2(N2);
	for (int i = 0; i<N2; ++i)
	{
		GNode* ni = m_model.FindNode(node2[i]);
		if (ni == 0) return false;
		set2[i] = ni;
		r2[i] = ni->Position();
	}

	if (m_projectToLines)
	{
		// project the nodes to a line
		vec3d e1 = projectToLine(r1, 0.0);
		vec3d e2 = projectToLine(r2, 1.0);
		if (e1*e2 < 0)
		{
			for (int i=0; i<r2.size(); ++i) r2[i].x = 1.0 - r2[i].x;
		}
	}

	// now connect all nodes based on closest point
	vector<pair<int,int> > springs1, springs2;
	generate_springs(springs1, r1, r2, tag1, tag2);
	for (int i=0; i<(int)springs1.size(); ++i)
	{
		pair<int,int>& spring_i = springs1[i];

		GNode* na = set1[spring_i.first];
		GNode* nb = set2[spring_i.second];

		po->AddElement(na->GetID(), nb->GetID());
	}

	generate_springs(springs2, r2, r1, tag2, tag1);
	for (int i = 0; i<(int)springs2.size(); ++i)
	{
		pair<int, int>& spring_i = springs2[i];

		GNode* na = set2[spring_i.first];
		GNode* nb = set1[spring_i.second];

		po->AddElement(na->GetID(), nb->GetID());
	}

	return true;
}

bool CSpringGenerator::generate(const std::vector<vec3d>& r1, const std::vector<vec3d>& r2, std::vector<pair<int, int> >& springs)
{
	int N1 = (int)r1.size();
	int N2 = (int)r2.size();

	if ((N1 == 0) || (N2 == 0)) return false;

	// use tags to mark progress and try to prevent 
	// two springs connecting to the same node
	vector<int> tag1(N1, 0), tag2(N2, 0);

	// now connect all nodes based on closest point
	vector<pair<int, int> > springs1, springs2;
	generate_springs(springs1, r1, r2, tag1, tag2);
	for (int i = 0; i<(int)springs1.size(); ++i)
	{
		pair<int, int>& spring_i = springs1[i];
		springs.push_back(spring_i);
	}

	generate_springs(springs2, r2, r1, tag2, tag1);
	for (int i = 0; i<(int)springs2.size(); ++i)
	{
		pair<int, int>& spring_i = springs2[i];
		springs.push_back(pair<int,int>(spring_i.second, spring_i.first));
	}

	return true;
}
