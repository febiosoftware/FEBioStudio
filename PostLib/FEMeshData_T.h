#pragma once

#include "FEMeshData.h"
#include "FEModel.h"
#include "FEState.h"
#include <set>
using namespace std;

namespace Post {

//=============================================================================
// 
//    N O D E   D A T A
// 
//=============================================================================

//-----------------------------------------------------------------------------
// base class for node data
class FENodeItemData : public FEMeshData
{
public:
	FENodeItemData(FEState* state, Data_Type ntype, Data_Format nfmt) : FEMeshData(state, ntype, nfmt){}
};

//-----------------------------------------------------------------------------
// template base class for defining the data value
template <typename T> class FENodeData_T : public FENodeItemData
{
public:
	FENodeData_T(FEState* state, FEDataField* pdf) : FENodeItemData(state, FEMeshDataTraits<T>::Type(), DATA_ITEM) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static Data_Type Type  () { return FEMeshDataTraits<T>::Type  (); }
	static Data_Format Format() { return DATA_ITEM; }
	static Data_Class Class() { return CLASS_NODE; }
};

//-----------------------------------------------------------------------------
// template class for nodal data stored in vectors
// \todo rename this class to FENodeDataArray or something
template <typename T> class FENodeData : public FENodeData_T<T>
{
public:
	FENodeData(FEState* state, FEDataField* pdf) : FENodeData_T<T>(state, pdf) { m_data.resize(state->GetFEMesh()->Nodes()); }
	void eval(int n, T* pv) { (*pv) = m_data[n]; }
	void copy(FENodeData<T>& d) { m_data = d.m_data; }

	int size() const { return (int) m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	vector<T>	m_data;
};

//-----------------------------------------------------------------------------
class FENodeArrayData : public FENodeItemData
{
public:
	FENodeArrayData(FEState* state, int nsize) : FENodeItemData(state, DATA_ARRAY, DATA_ITEM)
	{
		int N = state->GetFEMesh()->Nodes();
		m_stride = nsize;
		m_data.resize(N*nsize, 0.f);
	}

	float eval(int n, int comp) { return m_data[n*m_stride + comp]; }
	void setData(vector<float>& data)
	{
		assert(data.size() == m_data.size());
		m_data = data;
	}

protected:
	int				m_stride;
	vector<float>	m_data;	
};

//=============================================================================
// 
//    F A C E   D A T A
// 
//=============================================================================

//-----------------------------------------------------------------------------
// base class for face data
class FEFaceItemData : public FEMeshData
{
public:
	FEFaceItemData(FEState* state, Data_Type ntype, Data_Format nfmt) : FEMeshData(state, ntype, nfmt){}
};

//-----------------------------------------------------------------------------
// template base class for defining the data value
template <typename T, Data_Format fmt> class FEFaceData_T : public FEFaceItemData
{
public:
	FEFaceData_T(FEState* state, FEDataField* pdf) : FEFaceItemData(state, FEMeshDataTraits<T>::Type(), fmt) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static Data_Type Type  () { return FEMeshDataTraits<T>::Type  (); }
	static Data_Format Format() { return fmt; }
	static Data_Class Class() { return CLASS_FACE; }
};

//-----------------------------------------------------------------------------
// template class for face data stored in vectors
template <typename T, Data_Format fmt> class FEFaceData : public FEFaceData_T<T, fmt> {};

// *** specialization for DATA_ITEM format ***
template <typename T> class FEFaceData<T, DATA_ITEM> : public FEFaceData_T<T, DATA_ITEM>
{
public:
	FEFaceData(FEState* state, FEDataField* pdf) : FEFaceData_T<T, DATA_ITEM>(state, pdf)
	{ 
		if (m_face.empty())
			m_face.assign(state->GetFEMesh()->Faces(), -1); 
	}
	void eval(int n, T* pv) { (*pv) = m_data[m_face[n]]; }
	bool active(int n) { return (m_face[n] >= 0); }
	void copy(FEFaceData<T,DATA_ITEM>& d) { m_data = d.m_data; }
	bool add(int n, const T& d)
	{ 
		if (m_face[n] >= 0) 
		{
//			assert(m_face[n] == (int) m_data.size());
			if (m_face[n] != (int)m_data.size()) return false;
		}
		else m_face[n] = (int) m_data.size(); 
		m_data.push_back(d); 
		return true;
	}

	int size() const { return (int) m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	vector<T>		m_data;
	vector<int>		m_face;
};

// *** specialization for DATA_REGION format ***
template <typename T> class FEFaceData<T, DATA_REGION> : public FEFaceData_T<T, DATA_REGION>
{
public:
	FEFaceData(FEState* state, FEDataField* pdf) : FEFaceData_T<T, DATA_REGION>(state, pdf)
	{ 
		if (m_face.empty())
			m_face.assign(state->GetFEMesh()->Faces(), -1); 
	}
	void eval(int n, T* pv) { (*pv) = m_data[m_face[n]]; }
	bool active(int n) { return (m_face[n] >= 0); }
	void copy(FEFaceData<T,DATA_ITEM>& d) { m_data = d.m_data; }
	bool add(vector<int>& item, const T& v) 
	{ 
		int m = (int) m_data.size(); 
		m_data.push_back(v);
		for (int i=0; i<(int)item.size(); ++i)
		{
			if (m_face[item[i]] == -1) m_face[item[i]] = m;
			else
			{
//				assert(m_face[item[i]] == m);
				if (m_face[item[i]] != m) return false;
			}
		}

		return true;
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	vector<T>		m_data;
	vector<int>		m_face;
};


// *** specialization for DATA_COMP format ***
template <typename T> class FEFaceData<T, DATA_COMP> : public FEFaceData_T<T, DATA_COMP>
{
public:
	FEFaceData(FEState* state, FEDataField* pdf) : FEFaceData_T<T, DATA_COMP>(state, pdf)
	{ 
		if (m_face.empty())
			m_face.assign(state->GetFEMesh()->Faces(), -1); 
	}
	void eval(int n, T* pv)
	{ 
        int m = FEMeshData::GetFEState()->GetFEMesh()->Face(n).Nodes();
		for (int i=0; i<m; ++i) pv[i] = m_data[m_face[n] + i];
	}
	bool active(int n) { return (m_face[n] >= 0); }
	void copy(FEFaceData<T,DATA_COMP>& d) { m_data = d.m_data; m_face = d.m_face; }
	bool add(int n, T* d, int m) 
	{ 
		if (m_face[n] >= 0) 
		{
//			assert(m_face[n] == (int) m_data.size());
			if (m_face[n] != (int)m_data.size()) return false;
		}
		else m_face[n] = (int) m_data.size();
		for (int i=0; i<m; ++i) m_data.push_back(d[i]);
		return true;
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	vector<T>		m_data;
	vector<int>		m_face;
};

// *** specialization for DATA_NODE format ***
// this assumes 4 values per face
template <typename T> class FEFaceData<T, DATA_NODE> : public FEFaceData_T<T, DATA_NODE>
{
public:
	FEFaceData(FEState* state, FEDataField* pdf) : FEFaceData_T<T, DATA_NODE>(state, pdf)
	{
		if (m_face.empty())
		{
			int N = state->GetFEMesh()->Faces();
			m_face.resize(2*N);
			for (int i=0; i<N; ++i) { m_face[2*i] = -1; m_face[2*i+1] = 0; }
		}
	}
	void eval(int nface, T* pv)
	{ 
		int n = m_face[2*nface];
		int m = m_face[2*nface+1];
		for (int i=0; i<m; ++i) pv[i] = m_data[m_indx[n + i] ]; 
	}
	bool active(int n) { return (m_face[2*n] >= 0); }
	void copy(FEFaceData<T,DATA_NODE>& d) { m_data = d.m_data; m_indx = d.m_indx; }
	void add(vector<T>& data, vector<int>& face, vector<int>& index, vector<int>& nf)
	{
		int n0 = (int)m_data.size();
		m_data.insert(m_data.end(), data.begin(), data.end());
		int c = 0;
		for (int i = 0; i<(int)face.size(); ++i)
		{
			m_face[2 * face[i]] = (int) m_indx.size();
			m_face[2 * face[i] + 1] = nf[i];
			for (int j = 0; j<nf[i]; ++j, ++c) m_indx.push_back(index[c] + n0);
		}
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	vector<T>		m_data;
	vector<int>		m_face;
	vector<int>		m_indx; 
};

//=============================================================================
// 
//    E L E M   D A T A
// 
//=============================================================================

//-----------------------------------------------------------------------------
// base class for element data
class FEElemItemData : public FEMeshData
{
public:
	FEElemItemData(FEState* state, Data_Type ntype, Data_Format nfmt) : FEMeshData(state, ntype, nfmt){}
};

//-----------------------------------------------------------------------------
class FEElemArrayDataItem : public FEElemItemData
{
public:
	FEElemArrayDataItem(FEState* state, int nsize, FEDataField* pdf) : FEElemItemData(state, DATA_ARRAY, DATA_ITEM)
	{
		int NE = state->GetFEMesh()->Elements();
		m_stride = nsize;
		m_data.resize(NE*nsize, 0.f);
		m_elem.resize(NE, -1);
	}

	bool active(int n) { return (m_elem.empty() == false) && (m_elem[n] >= 0); }

	float eval(int n, int comp) 
	{ 
		return m_data[m_elem[n]*m_stride + comp];
	}

	void setData(vector<float>& data, vector<int>& elem)
	{
		assert(data.size() == m_stride*elem.size());
		for (int i=0; i<(int)elem.size(); ++i)
		{
			int m = elem[i];
			for (int j=0; j<m_stride; ++j)
			{
				m_data[m*m_stride + j] = data[i*m_stride + j];
			}
			m_elem[m] = m;
		}
	}

protected:
	int				m_stride;
	vector<float>	m_data;
	vector<int>		m_elem;
};

//-----------------------------------------------------------------------------
class FEElemArrayDataNode : public FEElemItemData
{
public:
	FEElemArrayDataNode(FEState* state, int nsize, FEDataField* pdf) : FEElemItemData(state, DATA_ARRAY, DATA_NODE)
	{
		FEMeshBase& m = *state->GetFEMesh();
		m_stride = nsize;
		if (m_elem.empty())
		{
			int N = m.Elements();
			m_elem.resize(2 * N);
			for (int i = 0; i<N; ++i) { m_elem[2 * i] = -1; m_elem[2 * i + 1] = 0; }
		}
	}
	void eval(int i, int comp, float* pv)
	{
		int n = m_elem[2 * i];	// start index in data array
		int m = m_elem[2 * i + 1];	// size of elem data (should be nr. of nodes)
		for (int j = 0; j<m; ++j) pv[j] = m_data[m_indx[n + j] + comp];
	}
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[2 * n] >= 0); }
	void add(vector<float>& d, vector<int>& e, vector<int>& l, int ne)
	{
		int n0 = (int)m_data.size();
		m_data.insert(m_data.end(), d.begin(), d.end());
		for (int i = 0; i<(int)e.size(); ++i)
		{
			m_elem[2 * e[i]] = (int)m_indx.size();
			m_elem[2 * e[i] + 1] = ne;
			for (int j = 0; j<ne; ++j) m_indx.push_back(m_stride*l[i*ne + j] + n0);
		}
	}

protected:
	int m_stride;
	vector<float>	m_data;
	vector<int>		m_elem;
	vector<int>		m_indx;
};


//-----------------------------------------------------------------------------
class FEElemArrayVec3Data : public FEElemItemData
{
public:
	FEElemArrayVec3Data(FEState* state, int nsize, FEDataField* pdf) : FEElemItemData(state, DATA_ARRAY_VEC3F, DATA_ITEM)
	{
		int NE = state->GetFEMesh()->Elements();
		m_stride = nsize;
		m_data.resize(NE*nsize*3, 0.f);
		m_elem.resize(NE, -1);
	}

	bool active(int n) { return (m_elem.empty() == false) && (m_elem[n] >= 0); }

	// evaluate the field for an element
	// n = element index
	// index = array index (0 ... stride-1)
	vec3f eval(int n, int index)
	{
		float* d = &m_data[3*(m_elem[n] * m_stride + index)];
		return vec3f(d[0], d[1], d[2]);
	}

	void setData(vector<float>& data, vector<int>& elem)
	{
		assert(data.size() == 3*m_stride*elem.size());
		for (int i = 0; i<(int)elem.size(); ++i)
		{
			int m = elem[i];
			for (int j = 0; j<m_stride; ++j)
			{
				m_data[3*(m*m_stride + j)    ] = data[3*(i*m_stride + j)  ];
				m_data[3*(m*m_stride + j) + 1] = data[3*(i*m_stride + j)+1];
				m_data[3*(m*m_stride + j) + 2] = data[3*(i*m_stride + j)+2];
			}
			m_elem[m] = m;
		}
	}

protected:
	int				m_stride;
	vector<float>	m_data;
	vector<int>		m_elem;
};


//-----------------------------------------------------------------------------
// template base class for defining the data value
template <typename T, Data_Format fmt> class FEElemData_T : public FEElemItemData
{
public:
	FEElemData_T(FEState* state, FEDataField* pdf) : FEElemItemData(state, FEMeshDataTraits<T>::Type(), fmt) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static Data_Type Type  () { return FEMeshDataTraits<T>::Type  (); }
	static Data_Format Format() { return fmt; }
	static Data_Class Class() { return CLASS_ELEM; }
};

//-----------------------------------------------------------------------------
// template class for element data stored in vectors
template <typename T, Data_Format fmt> class FEElementData : public FEElemData_T<T, fmt>{};

// *** specialization for DATA_ITEM format ***
template <typename T> class FEElementData<T, DATA_ITEM> : public FEElemData_T<T, DATA_ITEM>
{
public:
	FEElementData(FEState* state, FEDataField* pdf) : FEElemData_T<T, DATA_ITEM>(state, pdf)
	{ 
		m_elem.assign(state->GetFEMesh()->Elements(), -1); 
	}
	void eval(int n, T* pv) { assert(m_elem[n] >= 0); (*pv) = m_data[m_elem[n]]; }
	void set(int n, const T& v) { assert(m_elem[n] >= 0); m_data[m_elem[n]] = v; }
	void copy(FEElementData<T, DATA_ITEM>& d) { m_data = d.m_data; }
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[n] >= 0); }
	void add(int n, const T& v)
	{ 
		int m = m_elem[n]; 
		if (m == -1)
		{
			m_elem[n] = (int) m_data.size(); 
		}
		else assert(m == (int)m_data.size());
		m_data.push_back(v);
	}
	int size() { return (int) m_data.size(); }
	T& operator [] (int i) { return m_data[i]; }

protected:
	vector<T>		m_data;
	vector<int>		m_elem;
};

// *** specialization for DATA_REGION format ***
template <typename T> class FEElementData<T, DATA_REGION> : public FEElemData_T<T, DATA_REGION>
{
public:
	FEElementData(FEState* state, FEDataField* pdf) : FEElemData_T<T, DATA_REGION>(state, pdf)
	{
		if (m_elem.empty())
			m_elem.assign(state->GetFEMesh()->Elements(), -1); 
	}
	void eval(int n, T* pv) { assert(m_elem[n] >= 0); (*pv) = m_data[m_elem[n]]; }
	void copy(FEElementData<T, DATA_REGION>& d) { m_data = d.m_data; }
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[n] >= 0); }
	void add(vector<int>& item, const T& v) 
	{ 
		int m = (int) m_data.size(); 
		m_data.push_back(v);
		for (int i=0; i<(int)item.size(); ++i)
		{
			if (m_elem[item[i]] == -1) m_elem[item[i]] = m;
			else
			{
				assert(m_elem[item[i]] == m);
			}
		}
	}

	void add(int item, const T& v)
	{
		int m = (int)m_data.size();
		m_data.push_back(v);
		if (m_elem[item] == -1) m_elem[item] = m;
		else
		{
			assert(m_elem[item] == m);
		}
	}

	int size() const { return (int) m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	vector<T>		m_data;
	vector<int>		m_elem;
};

// *** specialization for DATA_COMP format ***
template <typename T> class FEElementData<T, DATA_COMP> : public FEElemData_T<T, DATA_COMP>
{
public:
	FEElementData(FEState* state, FEDataField* pdf) : FEElemData_T<T, DATA_COMP>(state, pdf)
	{
		if (m_elem.empty())
		{
			int N = state->GetFEMesh()->Elements();
			m_elem.resize(2*N); 
			for (int i=0; i<N; ++i) { m_elem[2*i] = -1; m_elem[2*i+1] = 0; }
		}
	}
	void eval(int i, T* pv)
	{ 
		int n = m_elem[2*i  ];
		int m = m_elem[2*i+1];
		for (int j=0; j<m; ++j) pv[j] = m_data[n + j];
	}
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[2 * n + 1] > 0); }
	void copy(FEElementData<T,DATA_COMP>& d) { m_data = d.m_data; }
	void add(int n, int m, T* d) 
	{ 
		if (m_elem[2*n] == -1)
		{
			m_elem[2*n  ] = (int) m_data.size(); 
			m_elem[2*n+1] = m;
		}
		for (int j=0; j<m; ++j) m_data.push_back(d[j]); 
	}
	int size() { return (int) m_data.size(); }
	T& operator [] (int i) { return m_data[i]; }

protected:
	vector<T>		m_data;
	vector<int>		m_elem;
};

// *** specialization for DATA_NODE format ***
template <typename T> class FEElementData<T, DATA_NODE> : public FEElemData_T<T, DATA_NODE>
{
public:
	FEElementData(FEState* state, FEDataField* pdf) : FEElemData_T<T, DATA_NODE>(state, pdf)
	{
		FEMeshBase& m = *state->GetFEMesh();
		if (m_elem.empty())
		{
			int N = m.Elements();
			m_elem.resize(2*N);
			for (int i=0; i<N; ++i) { m_elem[2*i] = -1; m_elem[2*i+1] = 0; }
		}
	}
	void eval(int i, T* pv)
	{ 
		int n = m_elem[2*i  ];	// start index in data array
		int m = m_elem[2*i+1];	// size of elem data (should be nr. of nodes)
		for (int j=0; j<m; ++j) pv[j] = m_data[ m_indx[n + j] ];
	}
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[2 * n] >= 0); }
	void copy(FEElementData<T,DATA_NODE>& d) { m_data = d.m_data; m_indx = d.m_indx; }
	void add(vector<T>& d, vector<int>& e, vector<int>& l, int ne) 
	{ 
		int n0 = (int) m_data.size();
		m_data.insert(m_data.end(), d.begin(), d.end());
		for (int i=0; i<(int) e.size(); ++i) 
		{
			m_elem[2*e[i]  ] = (int) m_indx.size();
			m_elem[2*e[i]+1] = ne;
			for (int j=0; j<ne; ++j) m_indx.push_back(l[i*ne + j] + n0);
		}
	}
	int size() { return (int) m_data.size(); }
	T& operator [] (int i) { return m_data[i]; }

protected:
	vector<T>		m_data;
	vector<int>		m_elem;
	vector<int>		m_indx;
};

//=============================================================================
// Additional node data fields
//=============================================================================

//-----------------------------------------------------------------------------
class FENodePosition : public FENodeData_T<vec3f>
{
public:
	FENodePosition(FEState* state, FEDataField* pdf) : FENodeData_T<vec3f>(state, pdf){}
	void eval(int n, vec3f* pv);
};

//-----------------------------------------------------------------------------
class FENodeInitPos : public FENodeData_T<vec3f>
{
public:
	FENodeInitPos(FEState* state, FEDataField* pdf) : FENodeData_T<vec3f>(state, pdf){}
	void eval(int n, vec3f* pv);
};

//=============================================================================
// Additional face data fields
//=============================================================================

class FECurvatureField;

class FECurvature : public FEFaceData_T<float, DATA_NODE>
{
public:
	FECurvature(FEState* state, FECurvatureField* pdf);

	void eval_curvature(int, float* f, int m);

	void eval(int n, float* f);

	bool active(int n) { return (m_face[n] == 1); }

	void set_facelist(vector<int>& l);

private:
	void level(int n, int l, set<int>& nl1);

	float nodal_curvature(int n, int m);

public: // parameters
	FECurvatureField*	m_pdf;
	vector<int> m_face;
};

class FECurvatureField : public FEDataField
{
public:
	enum CURVATURE_TYPE
	{
		GAUSS_CURVATURE,
		MEAN_CURVATURE,
		PRINC1_CURVATURE,
		PRINC2_CURVATURE,
		RMS_CURVATURE,
		DIFF_CURVATURE
	};

public:
	FECurvatureField(const std::string& name, int measure) : FEDataField(name, DATA_FLOAT, DATA_NODE, CLASS_FACE, 0)
	{
		m_measure = measure;
		m_nlevels = 1;
		m_nmax = 10;
		m_bext = false;
	}

	FEDataField* Clone() const override
	{
		FECurvatureField* pd = new FECurvatureField(GetName(), m_measure);
		pd->m_nlevels = m_nlevels;
		pd->m_nmax = m_nmax;
		pd->m_bext = m_bext;
		return pd;
	}

	FEMeshData* CreateData(FEState* pstate) override
	{
		return new FECurvature(pstate, this);
	}

public:
	int	m_measure;	// curvature measure to evaluate
	int	m_nlevels;	// neighbor search radius
	int	m_nmax;		// max iterations
	int m_bext;		// use extended quadric method
};

//-----------------------------------------------------------------------------
class FEPrincCurvatureVector : public FEFaceData_T<vec3f, DATA_NODE>
{
public:
	FEPrincCurvatureVector(FEState* state, FEDataField* pdf) : FEFaceData_T<vec3f, DATA_NODE>(state, pdf) { m_face.assign(state->GetFEMesh()->Faces(), 1); }

	bool active(int n) { return (m_face[n] == 1); }

	void set_facelist(vector<int>& l);

protected:

	void eval(int n, vec3f* f, int m);

	void level(int n, int l, set<int>& nl1);

	vec3f nodal_curvature(int n, int m);

public: // parameters
	static int	m_nlevels;	// neighbor search radius
	static int	m_nmax;		// max iterations
	static int  m_bext;		// use extended quadric method

	vector<int> m_face;
};

//-----------------------------------------------------------------------------
class FEPrincCurvatureVector1 : public FEPrincCurvatureVector
{
public:
	FEPrincCurvatureVector1(FEState* state, FEDataField* pdf) : FEPrincCurvatureVector(state, pdf){}
	void eval(int n, vec3f* f) { FEPrincCurvatureVector::eval(n, f, 0); }
};

//-----------------------------------------------------------------------------
class FEPrincCurvatureVector2 : public FEPrincCurvatureVector
{
public:
	FEPrincCurvatureVector2(FEState* state, FEDataField* pdf) : FEPrincCurvatureVector(state, pdf){}
	void eval(int n, vec3f* f) { FEPrincCurvatureVector::eval(n, f, 1); }
};

//-----------------------------------------------------------------------------
class FECongruency : public FEFaceData_T<float, DATA_NODE>
{
public:
	FECongruency(FEState* state, FEDataField* pdf) : FEFaceData_T<float, DATA_NODE>(state, pdf) { m_face.assign(state->GetFEMesh()->Faces(), 1); }

	bool active(int n) { return (m_face[n] == 1); }

	void set_facelist(vector<int>& l);

protected:
	void eval(int n, float* f);

	vector<int> m_face;

public:
	static int m_nlevels;
	static int m_nmax;
	static int m_bext;
};

//=============================================================================
// Additional element data fields
//=============================================================================

//-----------------------------------------------------------------------------
class FEDeformationGradient : public FEElemData_T<Mat3d, DATA_COMP>
{
public:
	FEDeformationGradient(FEState* pstate, FEDataField* pdf);
	void eval(int n, Mat3d* pv);
};

//-----------------------------------------------------------------------------
// strain data
class FEStrainDataField : public FEDataField
{
public:
	enum {
		INF_STRAIN,
		RIGHT_CAUCHY_GREEN,
		RIGHT_STRETCH,
		LAGRANGE,
		BIOT,
		RIGHT_HENCKY,
		LEFT_CAUCHY_GREEN,
		LEFT_STRETCH,
		LEFT_HENCKY,
		ALMANSI
	};

public:
	FEStrainDataField(const std::string& name, int measure) : FEDataField(name, DATA_MAT3FS, DATA_ITEM, CLASS_ELEM, 0)
	{ 
		m_nref = -1; 
		m_measure = measure; 
	}

	FEDataField* Clone() const override
	{
		FEStrainDataField* pd = new FEStrainDataField(GetName(), m_measure);
		pd->m_nref = m_nref;
		return pd;
	}

	FEMeshData* CreateData(FEState* pstate) override;

public:
	int	m_nref;
	int	m_measure;
};

//-----------------------------------------------------------------------------
class FEStrain : public FEElemData_T<mat3fs, DATA_ITEM>
{
public:
	FEStrain(FEState* state, FEStrainDataField* pdf) : FEElemData_T<mat3fs, DATA_ITEM>(state, pdf), m_strainData(pdf) {}

protected:
	int ReferenceState() { return m_strainData->m_nref; }

private:
	FEStrainDataField*	m_strainData;
};

//-----------------------------------------------------------------------------
class FEInfStrain : public FEStrain
{
public:
	FEInfStrain(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FERightCauchyGreen : public FEStrain
{
public:
	FERightCauchyGreen(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FERightStretch : public FEStrain
{
public:
	FERightStretch(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FELagrangeStrain : public FEStrain
{
public:
	FELagrangeStrain(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FEBiotStrain : public FEStrain
{
public:
	FEBiotStrain(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FERightHencky : public FEStrain
{
public:
	FERightHencky(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FELeftCauchyGreen : public FEStrain
{
public:
	FELeftCauchyGreen(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FELeftStretch : public FEStrain
{
public:
	FELeftStretch(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FELeftHencky : public FEStrain
{
public:
	FELeftHencky(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FEAlmansi : public FEStrain
{
public:
	FEAlmansi(FEState* state, FEStrainDataField* pdf) : FEStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class FEVolRatio : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEVolRatio(FEState* state, FEDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FEElementVolume : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEElementVolume(FEState* state, FEDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FEAspectRatio : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEAspectRatio(FEState* state, FEDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FEMaxEdgeAngle : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEMaxEdgeAngle(FEState* state, FEDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FEMinEdgeAngle : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEMinEdgeAngle(FEState* state, FEDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FEVolStrain : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEVolStrain(FEState* state, FEDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FEElemPressure : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEElemPressure(FEState* state, FEDataField* pdf);
	void eval(int n, float* pv);
private:
	int	m_nstress;	// stress field
};

//-----------------------------------------------------------------------------
// Pressure field corresponding to the "nodal stress" field, which stores for
// each element the stress at the nodes
class FEElemNodalPressure : public FEElemData_T<float, DATA_COMP>
{
public:
	FEElemNodalPressure(FEState* state, FEDataField* pdf);
	void eval(int n, float* pv);
private:
	int	m_nstress;	// nodal stress field
};

//-----------------------------------------------------------------------------
class FESolidStress : public FEElemData_T<mat3fs, DATA_ITEM>
{
public:
	FESolidStress(FEState* state, FEDataField* pdf);
	void eval(int n, mat3fs* pv);
private:
	int	m_nstress;	// total stress field
	int	m_nflp;		// fluid pressure field
};
}
