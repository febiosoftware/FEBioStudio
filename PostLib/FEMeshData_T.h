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

#include "FEMeshData.h"
#include "FEState.h"
#include <MeshLib/FSMesh.h>
#include "FEDataField.h"
#include <set>

namespace Post {

//=============================================================================
// 
//    G L O B A L   D A T A
// 
//=============================================================================

//-----------------------------------------------------------------------------
// base class for global data
class FEGlobalData : public FEMeshData
{
public:
	FEGlobalData(FEState* state, DATA_TYPE ntype) : FEMeshData(state, ntype, DATA_REGION){}
};

template <typename T> class FEGlobalData_T : public FEGlobalData
{
public:
	FEGlobalData_T(FEState* state, ModelDataField* pdf) : FEGlobalData(state, FEMeshDataTraits<T>::Type()) {}
	T value() { return m_data; };
	void setValue(T a) { m_data = a; }
	bool active(int n) { return true; }

	static DATA_TYPE Type() { return FEMeshDataTraits<T>::Type(); }
	static DATA_FORMAT Format() { return DATA_REGION; }
	static DATA_CLASS Class() { return OBJECT_DATA; }

private:
	T	m_data;
};

//-----------------------------------------------------------------------------
class FEGlobalArrayData : public FEGlobalData
{
public:
	FEGlobalArrayData(FEState* state, int nsize) : FEGlobalData(state, DATA_ARRAY)
	{
		m_data.resize(nsize, 0.f);
	}

	float eval(int n) { return m_data[n]; }
	void setData(std::vector<float>& data)
	{
		assert(data.size() == m_data.size());
		m_data = data;
	}

	int components() const { return (int)m_data.size(); }

protected:
	std::vector<float>	m_data;
};

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
	FENodeItemData(FEState* state, DATA_TYPE ntype, DATA_FORMAT nfmt) : FEMeshData(state, ntype, nfmt){}
};

//-----------------------------------------------------------------------------
// template base class for defining the data value
template <typename T> class FENodeData_T : public FENodeItemData
{
public:
	FENodeData_T(FEState* state, ModelDataField* pdf) : FENodeItemData(state, FEMeshDataTraits<T>::Type(), DATA_ITEM) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static DATA_TYPE Type  () { return FEMeshDataTraits<T>::Type  (); }
	static DATA_FORMAT Format() { return DATA_ITEM; }
	static DATA_CLASS Class() { return NODE_DATA; }
};

//-----------------------------------------------------------------------------
// template class for nodal data stored in vectors
// \todo rename this class to FENodeDataArray or something
template <typename T> class FENodeData : public FENodeData_T<T>
{
public:
	FENodeData(FEState* state, ModelDataField* pdf) : FENodeData_T<T>(state, pdf) { m_data.resize(state->GetFEMesh()->Nodes()); }
	void eval(int n, T* pv) { (*pv) = m_data[n]; }
	void copy(FENodeData<T>& d) { m_data = d.m_data; }

	int size() const { return (int) m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	std::vector<T>	m_data;
};

//-----------------------------------------------------------------------------
class FENodeArrayData : public FENodeItemData
{
public:
	FENodeArrayData(FEState* state, int nsize) : FENodeItemData(state, DATA_ARRAY, DATA_ITEM)
	{
		int N = state->GetFEMesh()->Nodes();
		m_stride = nsize;
		m_data.resize(N * nsize, 0.f);
	}

	float eval(int n, int comp) { return m_data[n * m_stride + comp]; }
	void setData(std::vector<float>& data)
	{
		assert(data.size() == m_data.size());
		m_data = data;
	}

	int components() const { return m_stride; }

protected:
	int				m_stride;
	std::vector<float>	m_data;
};

//=============================================================================
// 
//    E D G E   D A T A
// 
//=============================================================================

// base class for edge data
class FEEdgeItemData : public FEMeshData
{
public:
	FEEdgeItemData(FEState* state, DATA_TYPE ntype, DATA_FORMAT nfmt) : FEMeshData(state, ntype, nfmt) {}
};

// template base class for defining the data value
template <typename T, DATA_FORMAT fmt> class FEEdgeData_T : public FEEdgeItemData
{
public:
	FEEdgeData_T(FEState* state, ModelDataField* pdf) : FEEdgeItemData(state, FEMeshDataTraits<T>::Type(), fmt) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static DATA_TYPE Type() { return FEMeshDataTraits<T>::Type(); }
	static DATA_FORMAT Format() { return fmt; }
	static DATA_CLASS Class() { return EDGE_DATA; }
};

// template class for edge data stored in vectors
template <typename T, DATA_FORMAT fmt> class FEEdgeData : public FEEdgeData_T<T, fmt> {};

// *** specialization for DATA_ITEM format ***
template <typename T> class FEEdgeData<T, DATA_ITEM> : public FEEdgeData_T<T, DATA_ITEM>
{
public:
	FEEdgeData(FEState* state, ModelDataField* pdf) : FEEdgeData_T<T, DATA_ITEM>(state, pdf)
	{
		if (m_edge.empty())
			m_edge.assign(state->GetFEMesh()->Edges(), -1);
	}
	void eval(int n, T* pv) { (*pv) = m_data[m_edge[n]]; }
	bool active(int n) { return (m_edge[n] >= 0); }
	void copy(FEEdgeData<T, DATA_ITEM>& d) { m_data = d.m_data; m_edge = d.m_edge; }
	bool add(int n, const T& d)
	{
		if ((n < 0) || (n >= m_edge.size())) return false;
		if (m_edge[n] >= 0)
		{
//			assert(m_edge[n] == (int) m_data.size());
			if (m_edge[n] != (int)m_data.size()) return false;
		}
		else m_edge[n] = (int)m_data.size();
		m_data.push_back(d);
		return true;
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	std::vector<T>		m_data;
	std::vector<int>	m_edge;
};

// *** specialization for DATA_REGION format ***
template <typename T> class FEEdgeData<T, DATA_REGION> : public FEEdgeData_T<T, DATA_REGION>
{
public:
	FEEdgeData(FEState* state, ModelDataField* pdf) : FEEdgeData_T<T, DATA_REGION>(state, pdf)
	{
		if (m_edge.empty())
			m_edge.assign(state->GetFEMesh()->Edges(), -1);
	}
	void eval(int n, T* pv) { (*pv) = m_data[m_edge[n]]; }
	bool active(int n) { return (m_edge[n] >= 0); }
	void copy(FEEdgeData<T, DATA_REGION>& d) { m_edge = d.m_edge; m_data = d.m_data; }
	bool add(std::vector<int>& item, const T& v)
	{
		int m = (int)m_data.size();
		m_data.push_back(v);
		for (int i = 0; i < (int)item.size(); ++i)
		{
			if (m_edge[item[i]] == -1) m_edge[item[i]] = m;
			else
			{
//				assert(m_edge[item[i]] == m);
				if (m_edge[item[i]] != m) return false;
			}
		}

		return true;
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	std::vector<T>		m_data;
	std::vector<int>	m_edge;
};


// *** specialization for DATA_MULT format ***
template <typename T> class FEEdgeData<T, DATA_MULT> : public FEEdgeData_T<T, DATA_MULT>
{
public:
	FEEdgeData(FEState* state, ModelDataField* pdf) : FEEdgeData_T<T, DATA_MULT>(state, pdf)
	{
		if (m_edge.empty())
			m_edge.assign(state->GetFEMesh()->Edges(), -1);
	}
	void eval(int n, T* pv)
	{
		int m = FEMeshData::GetFEState()->GetFEMesh()->Edge(n).Nodes();
		for (int i = 0; i < m; ++i) pv[i] = m_data[m_edge[n] + i];
	}
	bool active(int n) { return (m_edge[n] >= 0); }
	void copy(FEEdgeData<T, DATA_MULT>& d) { m_data = d.m_data; m_edge = d.m_edge; }
	bool add(int n, T* d, int m)
	{
		if (m_edge[n] >= 0)
		{
//			assert(m_edge[n] == (int) m_data.size());
			if (m_edge[n] != (int)m_data.size()) return false;
		}
		else m_edge[n] = (int)m_data.size();
		for (int i = 0; i < m; ++i) m_data.push_back(d[i]);
		return true;
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	std::vector<T>		m_data;
	std::vector<int>	m_edge;
};

// *** specialization for DATA_NODE format ***
template <typename T> class FEEdgeData<T, DATA_NODE> : public FEEdgeData_T<T, DATA_NODE>
{
public:
	FEEdgeData(FEState* state, ModelDataField* pdf) : FEEdgeData_T<T, DATA_NODE>(state, pdf)
	{
		if (m_edge.empty())
		{
			int N = state->GetFEMesh()->Edges();
			m_edge.resize(2 * N);
			for (int i = 0; i < N; ++i) { m_edge[2 * i] = -1; m_edge[2 * i + 1] = 0; }
		}
	}
	void eval(int nedge, T* pv)
	{
		int n = m_edge[2 * nedge];
		int m = m_edge[2 * nedge + 1];
		for (int i = 0; i < m; ++i) pv[i] = m_data[m_indx[n + i]];
	}
	bool active(int n) { return (m_edge[2 * n] >= 0); }
	void copy(FEEdgeData<T, DATA_NODE>& d) { m_data = d.m_data; m_indx = d.m_indx; }
	void add(std::vector<T>& data, std::vector<int>& edge, std::vector<int>& index, std::vector<int>& ne)
	{
		int n0 = (int)m_data.size();
		m_data.insert(m_data.end(), data.begin(), data.end());
		int c = 0;
		for (int i = 0; i < (int)edge.size(); ++i)
		{
			m_edge[2 * edge[i]] = (int)m_indx.size();
			m_edge[2 * edge[i] + 1] = ne[i];
			for (int j = 0; j < ne[i]; ++j, ++c) m_indx.push_back(index[c] + n0);
		}
	}

	int size() const { return (int)m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	std::vector<T>		m_data;
	std::vector<int>	m_edge;
	std::vector<int>	m_indx;
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
	FEFaceItemData(FEState* state, DATA_TYPE ntype, DATA_FORMAT nfmt) : FEMeshData(state, ntype, nfmt){}
};

//-----------------------------------------------------------------------------
class FEFaceArrayDataItem : public FEFaceItemData
{
public:
	FEFaceArrayDataItem(FEState* state, int nsize, ModelDataField* pdf) : FEFaceItemData(state, DATA_ARRAY, DATA_ITEM)
	{
		int NF = state->GetFEMesh()->Faces();
		m_stride = nsize;
		m_data.resize(NF * nsize, 0.f);
		m_face.resize(NF, -1);
	}

	bool active(int n) { return (m_face.empty() == false) && (m_face[n] >= 0); }

	float eval(int n, int comp)
	{
		return m_data[m_face[n] * m_stride + comp];
	}

	int arraySize() const { return m_stride; }

	void setData(std::vector<float>& data, std::vector<int>& face)
	{
		assert(data.size() == m_stride * face.size());
		for (int i = 0; i < (int)face.size(); ++i)
		{
			int m = face[i];
			for (int j = 0; j < m_stride; ++j)
			{
				m_data[m * m_stride + j] = data[i * m_stride + j];
			}
			m_face[m] = m;
		}
	}

	int components() const { return m_stride; }

protected:
	int					m_stride;
	std::vector<float>	m_data;
	std::vector<int>	m_face;
};

//-----------------------------------------------------------------------------
// template base class for defining the data value
template <typename T, DATA_FORMAT fmt> class FEFaceData_T : public FEFaceItemData
{
public:
	FEFaceData_T(FEState* state, ModelDataField* pdf) : FEFaceItemData(state, FEMeshDataTraits<T>::Type(), fmt) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static DATA_TYPE Type  () { return FEMeshDataTraits<T>::Type  (); }
	static DATA_FORMAT Format() { return fmt; }
	static DATA_CLASS Class() { return FACE_DATA; }
};

//-----------------------------------------------------------------------------
// template class for face data stored in vectors
template <typename T, DATA_FORMAT fmt> class FEFaceData : public FEFaceData_T<T, fmt> {};

// *** specialization for DATA_ITEM format ***
template <typename T> class FEFaceData<T, DATA_ITEM> : public FEFaceData_T<T, DATA_ITEM>
{
public:
	FEFaceData(FEState* state, ModelDataField* pdf) : FEFaceData_T<T, DATA_ITEM>(state, pdf)
	{ 
		if (m_face.empty())
			m_face.assign(state->GetFEMesh()->Faces(), -1); 
	}
	void eval(int n, T* pv) { (*pv) = m_data[m_face[n]]; }
	void set(int n, const T& v) { if (active(n)) m_data[m_face[n]] = v; else add(n, v); }
	bool active(int n) { return (m_face[n] >= 0); }
	void copy(FEFaceData<T, DATA_ITEM>& d) { m_data = d.m_data; m_face = d.m_face; }
	bool add(int n, const T& d)
	{ 
		if ((n < 0) || (n >= m_face.size())) return false;
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
	std::vector<T>		m_data;
	std::vector<int>		m_face;
};

// *** specialization for DATA_REGION format ***
template <typename T> class FEFaceData<T, DATA_REGION> : public FEFaceData_T<T, DATA_REGION>
{
public:
	FEFaceData(FEState* state, ModelDataField* pdf) : FEFaceData_T<T, DATA_REGION>(state, pdf)
	{ 
		if (m_face.empty())
			m_face.assign(state->GetFEMesh()->Faces(), -1); 
	}
	void eval(int n, T* pv) { (*pv) = m_data[m_face[n]]; }
	bool active(int n) { return (m_face[n] >= 0); }
	void copy(FEFaceData<T, DATA_REGION>& d) { m_face = d.m_face; m_data = d.m_data; }
	bool add(std::vector<int>& item, const T& v)
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
	std::vector<T>		m_data;
	std::vector<int>		m_face;
};


// *** specialization for DATA_MULT format ***
template <typename T> class FEFaceData<T, DATA_MULT> : public FEFaceData_T<T, DATA_MULT>
{
public:
	FEFaceData(FEState* state, ModelDataField* pdf) : FEFaceData_T<T, DATA_MULT>(state, pdf)
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
	void copy(FEFaceData<T,DATA_MULT>& d) { m_data = d.m_data; m_face = d.m_face; }
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
	std::vector<T>		m_data;
	std::vector<int>		m_face;
};

// *** specialization for DATA_NODE format ***
// this assumes 4 values per face
template <typename T> class FEFaceData<T, DATA_NODE> : public FEFaceData_T<T, DATA_NODE>
{
public:
	FEFaceData(FEState* state, ModelDataField* pdf) : FEFaceData_T<T, DATA_NODE>(state, pdf)
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
	void add(std::vector<T>& data, std::vector<int>& face, std::vector<int>& index, std::vector<int>& nf)
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
	std::vector<T>		m_data;
	std::vector<int>		m_face;
	std::vector<int>		m_indx;
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
	FEElemItemData(FEState* state, DATA_TYPE ntype, DATA_FORMAT nfmt) : FEMeshData(state, ntype, nfmt){}
};

//-----------------------------------------------------------------------------
class FEElemArrayDataItem : public FEElemItemData
{
public:
	FEElemArrayDataItem(FEState* state, int nsize, ModelDataField* pdf) : FEElemItemData(state, DATA_ARRAY, DATA_ITEM)
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

	int arraySize() const { return m_stride; }

	void setData(std::vector<float>& data, std::vector<int>& elem)
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

	int components() const { return m_stride; }

protected:
	int					m_stride;
	std::vector<float>	m_data;
	std::vector<int>	m_elem;
};

//-----------------------------------------------------------------------------
class FEElemArrayDataNode : public FEElemItemData
{
public:
	FEElemArrayDataNode(FEState* state, int nsize, ModelDataField* pdf) : FEElemItemData(state, DATA_ARRAY, DATA_NODE)
	{
		FSMesh& m = *state->GetFEMesh();
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
	void add(std::vector<float>& d, std::vector<int>& e, std::vector<int>& l, int ne)
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
	std::vector<float>	m_data;
	std::vector<int>		m_elem;
	std::vector<int>		m_indx;
};


//-----------------------------------------------------------------------------
class FEElemArrayVec3Data : public FEElemItemData
{
public:
	FEElemArrayVec3Data(FEState* state, int nsize, ModelDataField* pdf) : FEElemItemData(state, DATA_ARRAY_VEC3, DATA_ITEM)
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

	void setData(std::vector<float>& data, std::vector<int>& elem)
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

	int components() const { return m_stride; }

protected:
	int					m_stride;
	std::vector<float>	m_data;
	std::vector<int>	m_elem;
};


//-----------------------------------------------------------------------------
// template base class for defining the data value
template <typename T, DATA_FORMAT fmt> class FEElemData_T : public FEElemItemData
{
public:
	FEElemData_T(FEState* state, ModelDataField* pdf) : FEElemItemData(state, FEMeshDataTraits<T>::Type(), fmt) {}
	virtual void eval(int n, T* pv) = 0;
	virtual bool active(int n) { return true; }

	static DATA_TYPE Type  () { return FEMeshDataTraits<T>::Type  (); }
	static DATA_FORMAT Format() { return fmt; }
	static DATA_CLASS Class() { return ELEM_DATA; }
};

//-----------------------------------------------------------------------------
// template class for element data stored in vectors
template <typename T, DATA_FORMAT fmt> class FEElementData : public FEElemData_T<T, fmt>{};

// *** specialization for DATA_ITEM format ***
template <typename T> class FEElementData<T, DATA_ITEM> : public FEElemData_T<T, DATA_ITEM>
{
public:
	FEElementData(FEState* state, ModelDataField* pdf) : FEElemData_T<T, DATA_ITEM>(state, pdf)
	{ 
		m_elem.assign(state->GetFEMesh()->Elements(), -1); 
	}
	void eval(int n, T* pv) { assert(m_elem[n] >= 0); (*pv) = m_data[m_elem[n]]; }
	void set(int n, const T& v) { assert(m_elem[n] >= 0); m_data[m_elem[n]] = v; }
	void copy(FEElementData<T, DATA_ITEM>& d) { m_elem = d.m_elem; m_data = d.m_data; }
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
	std::vector<T>		m_data;
	std::vector<int>	m_elem;
};

// *** specialization for DATA_REGION format ***
template <typename T> class FEElementData<T, DATA_REGION> : public FEElemData_T<T, DATA_REGION>
{
public:
	FEElementData(FEState* state, ModelDataField* pdf) : FEElemData_T<T, DATA_REGION>(state, pdf)
	{
		if (m_elem.empty())
			m_elem.assign(state->GetFEMesh()->Elements(), -1); 
	}
	void eval(int n, T* pv) { assert(m_elem[n] >= 0); (*pv) = m_data[m_elem[n]]; }
	void copy(FEElementData<T, DATA_REGION>& d) { m_data = d.m_data; m_elem = d.m_elem; }
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[n] >= 0); }
	void add(std::vector<int>& item, const T& v)
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
		m_elem[item] = m;
	}

	int size() const { return (int) m_data.size(); }
	T& operator [] (int n) { return m_data[n]; }

protected:
	std::vector<T>		m_data;
	std::vector<int>	m_elem;
};

// *** specialization for DATA_MULT format ***
template <typename T> class FEElementData<T, DATA_MULT> : public FEElemData_T<T, DATA_MULT>
{
public:
	FEElementData(FEState* state, ModelDataField* pdf) : FEElemData_T<T, DATA_MULT>(state, pdf)
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
	void set(int i, T* pv)
	{
		if (active(i))
		{
			int n = m_elem[2 * i];
			int m = m_elem[2 * i + 1];
			for (int j = 0; j < m; ++j) m_data[n + j] = pv[j];
		}
	}
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[2 * n + 1] > 0); }
	void copy(FEElementData<T, DATA_MULT>& d) { m_data = d.m_data; m_elem = d.m_elem; }
	void add(int n, int m, T* d) 
	{ 
		if (m_elem[2*n] == -1)
		{
			m_elem[2*n  ] = (int) m_data.size(); 
			m_elem[2*n+1] = m;
			for (int j=0; j<m; ++j) m_data.push_back(d[j]); 
		}
		else
		{
			int n0 = m_elem[2 * n];
			assert(m_elem[2 * n + 1] == m);
			for (int j = 0; j < m; ++j) m_data[n0 + j] = d[j];
		}
	}
	int size() { return (int) m_data.size(); }
	T& operator [] (int i) { return m_data[i]; }

protected:
	std::vector<T>		m_data;
	std::vector<int>	m_elem;
};

// *** specialization for DATA_NODE format ***
template <typename T> class FEElementData<T, DATA_NODE> : public FEElemData_T<T, DATA_NODE>
{
public:
	FEElementData(FEState* state, ModelDataField* pdf) : FEElemData_T<T, DATA_NODE>(state, pdf)
	{
		FSMesh& m = *state->GetFEMesh();
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
	void set(int i, int j, T& v)
	{
		int n = m_elem[2 * i];	// start index in data array
		m_data[m_indx[n + j]] = v;
	}
	bool active(int n) { return (m_elem.empty() == false) && (m_elem[2 * n] >= 0); }
	void copy(FEElementData<T, DATA_NODE>& d) { m_data = d.m_data; m_indx = d.m_indx; m_elem = d.m_elem; }
	void add(std::vector<T>& d, std::vector<int>& e, std::vector<int>& l, int ne)
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
	std::vector<T>			m_data;
	std::vector<int>		m_elem;
	std::vector<int>		m_indx;
};

//=============================================================================
// Additional node data fields
//=============================================================================

//-----------------------------------------------------------------------------
class NodePosition : public FENodeData_T<vec3f>
{
public:
	NodePosition(FEState* state, ModelDataField* pdf) : FENodeData_T<vec3f>(state, pdf){}
	void eval(int n, vec3f* pv);
};

//-----------------------------------------------------------------------------
class NodeInitPos : public FENodeData_T<vec3f>
{
public:
	NodeInitPos(FEState* state, ModelDataField* pdf) : FENodeData_T<vec3f>(state, pdf){}
	void eval(int n, vec3f* pv);
};

//=============================================================================
// Additional face data fields
//=============================================================================

class CurvatureField;

class Curvature : public FEFaceData_T<float, DATA_NODE>
{
public:
	Curvature(FEState* state, CurvatureField* pdf);

	void eval_curvature(int, float* f, int m);

	void eval(int n, float* f);

	bool active(int n) { return (m_face[n] == 1); }

	void set_facelist(std::vector<int>& l);

private:
	void level(int n, int l, std::set<int>& nl1);

	float nodal_curvature(int n, int m);

public: // parameters
	CurvatureField*	m_pdf;
	std::vector<int> m_face;
};

class CurvatureField : public ModelDataField
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
	CurvatureField(FEPostModel* fem, int measure) : ModelDataField(fem, DATA_SCALAR, DATA_NODE, FACE_DATA, 0)
	{
		m_measure = measure;
		m_nlevels = 1;
		m_nmax = 10;
		m_bext = false;
	}

	ModelDataField* Clone() const override
	{
		CurvatureField* pd = new CurvatureField(m_fem, m_measure);
		pd->SetName(GetName());
		pd->m_nlevels = m_nlevels;
		pd->m_nmax = m_nmax;
		pd->m_bext = m_bext;
		return pd;
	}

	FEMeshData* CreateData(FEState* pstate) override
	{
		return new Curvature(pstate, this);
	}

public:
	int	m_measure;	// curvature measure to evaluate
	int	m_nlevels;	// neighbor search radius
	int	m_nmax;		// max iterations
	int m_bext;		// use extended quadric method
};

//-----------------------------------------------------------------------------
class PrincCurvatureVector : public FEFaceData_T<vec3f, DATA_NODE>
{
public:
	PrincCurvatureVector(FEState* state, ModelDataField* pdf) : FEFaceData_T<vec3f, DATA_NODE>(state, pdf) { m_face.assign(state->GetFEMesh()->Faces(), 1); }

	bool active(int n) { return (m_face[n] == 1); }

	void set_facelist(std::vector<int>& l);

protected:

	void eval(int n, vec3f* f, int m);

	void level(int n, int l, std::set<int>& nl1);

	vec3f nodal_curvature(int n, int m);

public: // parameters
	static int	m_nlevels;	// neighbor search radius
	static int	m_nmax;		// max iterations
	static int  m_bext;		// use extended quadric method

	std::vector<int> m_face;
};

//-----------------------------------------------------------------------------
class PrincCurvatureVector1 : public PrincCurvatureVector
{
public:
	PrincCurvatureVector1(FEState* state, ModelDataField* pdf) : PrincCurvatureVector(state, pdf){}
	void eval(int n, vec3f* f) { PrincCurvatureVector::eval(n, f, 0); }
};

//-----------------------------------------------------------------------------
class PrincCurvatureVector2 : public PrincCurvatureVector
{
public:
	PrincCurvatureVector2(FEState* state, ModelDataField* pdf) : PrincCurvatureVector(state, pdf){}
	void eval(int n, vec3f* f) { PrincCurvatureVector::eval(n, f, 1); }
};

//-----------------------------------------------------------------------------
class SurfaceCongruency : public FEFaceData_T<float, DATA_NODE>
{
public:
	SurfaceCongruency(FEState* state, ModelDataField* pdf) : FEFaceData_T<float, DATA_NODE>(state, pdf) { m_face.assign(state->GetFEMesh()->Faces(), 1); }

	bool active(int n) { return (m_face[n] == 1); }

	void set_facelist(std::vector<int>& l);

protected:
	void eval(int n, float* f);

	std::vector<int> m_face;

public:
	static int m_nlevels;
	static int m_nmax;
	static int m_bext;
};

//-----------------------------------------------------------------------------
class FEFacetArea : public FEFaceData_T<float, DATA_ITEM>
{
public:
	FEFacetArea(FEState* state, ModelDataField* pdf) : FEFaceData_T<float, DATA_ITEM>(state, pdf) { m_face.assign(state->GetFEMesh()->Faces(), 1); }

	bool active(int n) { return (m_face[n] == 1); }

protected:
	void eval(int n, float* f);

	std::vector<int> m_face;
};

//=============================================================================
// Additional element data fields
//=============================================================================

//-----------------------------------------------------------------------------
class DeformationGradient : public FEElemData_T<mat3f, DATA_MULT>
{
public:
	DeformationGradient(FEState* pstate, ModelDataField* pdf);
	void eval(int n, mat3f* pv);
};

//-----------------------------------------------------------------------------
// strain data
class StrainDataField : public ModelDataField
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

	enum { MP_REF_STATE };

public:
	StrainDataField(FEPostModel* fem, int measure) : ModelDataField(fem, DATA_MAT3S, DATA_ITEM, ELEM_DATA, 0)
	{
		m_measure = measure;

		AddIntParam(-1, "ref_state", "Reference state");
	}

	int ReferenceState() const { return GetIntValue(MP_REF_STATE); }
	void SetReferenceState(int n) { SetIntValue(MP_REF_STATE, n); }

	ModelDataField* Clone() const override
	{
		StrainDataField* pd = new StrainDataField(m_fem, m_measure);
		pd->SetName(GetName());
		pd->SetReferenceState(ReferenceState());
		return pd;
	}

	FEMeshData* CreateData(FEState* pstate) override;

public:
	int	m_measure;
};

//-----------------------------------------------------------------------------
class ElemStrain : public FEElemData_T<mat3fs, DATA_ITEM>
{
public:
	ElemStrain(FEState* state, StrainDataField* pdf) : FEElemData_T<mat3fs, DATA_ITEM>(state, pdf), m_strainData(pdf) {}

protected:
	int ReferenceState() { return m_strainData->ReferenceState(); }

private:
	StrainDataField*	m_strainData;
};

//-----------------------------------------------------------------------------
class InfStrain : public ElemStrain
{
public:
	InfStrain(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class RightCauchyGreen : public ElemStrain
{
public:
	RightCauchyGreen(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class RightStretch : public ElemStrain
{
public:
	RightStretch(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class LagrangeStrain : public ElemStrain
{
public:
	LagrangeStrain(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
// TODO: This should be mat2fs, but that type doesn't exist yet. 
class LagrangeStrain2D : public FEElemData_T<mat3fs, DATA_ITEM>
{
public:
	LagrangeStrain2D(FEState* state, ModelDataField* pdf) : FEElemData_T<mat3fs, DATA_ITEM>(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
// TODO: This should be mat2fs, but that type doesn't exist yet. 
class InfStrain2D : public FEElemData_T<mat3fs, DATA_ITEM>
{
public:
	InfStrain2D(FEState* state, ModelDataField* pdf) : FEElemData_T<mat3fs, DATA_ITEM>(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class BiotStrain : public ElemStrain
{
public:
	BiotStrain(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class RightHencky : public ElemStrain
{
public:
	RightHencky(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
	void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class LeftCauchyGreen : public ElemStrain
{
public:
	LeftCauchyGreen(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class LeftStretch : public ElemStrain
{
public:
	LeftStretch(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class LeftHencky : public ElemStrain
{
public:
	LeftHencky(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class AlmansiStrain : public ElemStrain
{
public:
	AlmansiStrain(FEState* state, StrainDataField* pdf) : ElemStrain(state, pdf) {}
    void eval(int n, mat3fs* pv);
};

//-----------------------------------------------------------------------------
class VolumeRatio : public FEElemData_T<float, DATA_ITEM>
{
public:
	VolumeRatio(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class ElementVolume : public FEElemData_T<float, DATA_ITEM>
{
public:
	ElementVolume(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class AspectRatio : public FEElemData_T<float, DATA_ITEM>
{
public:
	AspectRatio(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class MaxEdgeAngle : public FEElemData_T<float, DATA_ITEM>
{
public:
	MaxEdgeAngle(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class MinEdgeAngle : public FEElemData_T<float, DATA_ITEM>
{
public:
	MinEdgeAngle(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class VolumeStrain : public FEElemData_T<float, DATA_ITEM>
{
public:
	VolumeStrain(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf){}
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class ElemPressure : public FEElemData_T<float, DATA_ITEM>
{
public:
	ElemPressure(FEState* state, ModelDataField* pdf);
	void eval(int n, float* pv);
private:
	int	m_nstress;	// stress field
};

//-----------------------------------------------------------------------------
// Pressure field corresponding to the "nodal stress" field, which stores for
// each element the stress at the nodes
class ElemNodalPressure : public FEElemData_T<float, DATA_MULT>
{
public:
	ElemNodalPressure(FEState* state, ModelDataField* pdf);
	void eval(int n, float* pv);
private:
	int	m_nstress;	// nodal stress field
};

//-----------------------------------------------------------------------------
class SolidStress : public FEElemData_T<mat3fs, DATA_ITEM>
{
public:
	SolidStress(FEState* state, ModelDataField* pdf);
	void eval(int n, mat3fs* pv);
private:
	int	m_nstress;	// total stress field
	int	m_nflp;		// fluid pressure field
};

//-----------------------------------------------------------------------------
class FEElementMaterial : public FEElemData_T<float, DATA_ITEM>
{
public:
	FEElementMaterial(FEState* state, ModelDataField* pdf);
	void eval(int n, float* pv);
};

//-----------------------------------------------------------------------------
class FESurfaceNormal : public FEFaceData_T<vec3f, DATA_ITEM>
{
public:
	FESurfaceNormal(FEState* state, ModelDataField* pdf);
	void eval(int n, vec3f* pv);
};
}
