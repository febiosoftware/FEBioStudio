#pragma once
#include <MathLib/math3d.h>
#include <vector>
using namespace std;

namespace Post {
class FEPostMesh;

//-----------------------------------------------------------------------------
template <typename T>
class DataMap
{
public:
	DataMap(void) { m_pmesh = 0; }
	~DataMap(void) {}

	int States() { return (int)m_Data.size(); }
	vector<T>& State(int n) { return m_Data[n]; }
	int GetTag(int n) { return m_tag[n]; }
	void SetTag(int n, int ntag) { m_tag[n] = ntag; }
	void SetTags(int n)
	{
		for (int i = 0; i < m_tag.size(); ++i) m_tag[i] = n;
	}

	void Create(int nstates, int items, T val = T(0), int ntag = 0)
	{
		m_tag.assign(nstates, ntag);
		m_Data.resize(nstates);
		for (int i=0; i<nstates; ++i) m_Data[i].assign(items, val);
	}

	void Clear() { m_Data.clear(); m_tag.clear(); }

	void SetFEMesh(FEPostMesh* pm) { m_pmesh = pm; }

protected:
	vector<int>	m_tag;
	vector< vector<T> >	m_Data;
	FEPostMesh*	m_pmesh;
};

//-----------------------------------------------------------------------------
class VectorMap : public DataMap<vec3f>
{
public:
	// calculate the data gradient
	void Gradient(int ntime, vector<float>& v);
};
}
