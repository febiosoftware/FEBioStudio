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

#include "stdafx.h"
#include "DataFilter.h"
#include "FEPostModel.h"
#include "constants.h"
#include "FEMeshData_T.h"
#include "evaluate.h"
using namespace Post;
using namespace std;

bool Post::DataScale(FEPostModel& fem, int nfield, double scale)
{
	FSMesh& mesh = *fem.GetFEMesh(0);
	float fscale = (float) scale;
	// loop over all states
	int NN = mesh.Nodes();
	int ndata = FIELD_CODE(nfield);
	for (int i = 0; i<fem.GetStates(); ++i)
	{
		FEState& s = *fem.GetState(i);
		FEMeshData& d = s.m_Data[ndata];
		DATA_TYPE type = d.GetType();
		DATA_FORMAT fmt = d.GetFormat();
		if (IS_NODE_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_SCALAR:
			{
				FENodeData<float>* pf = dynamic_cast< FENodeData<float>* >(&d);
				for (int n = 0; n<NN; ++n) { float& v = (*pf)[n]; v *= fscale; }
			}
			break;
			case DATA_VEC3:
			{
				FENodeData<vec3f>* pv = dynamic_cast< FENodeData<vec3f>* >(&d);
				for (int n = 0; n<NN; ++n) { vec3f& v = (*pv)[n]; v *= fscale; }
			}
			break;
			case DATA_MAT3S:
			{
				FENodeData<mat3fs>* pv = dynamic_cast< FENodeData<mat3fs>* >(&d);
				for (int n = 0; n<NN; ++n) { mat3fs& v = (*pv)[n]; v *= fscale; }
			}
			break;
			case DATA_MAT3:
			{
				FENodeData<mat3f>* pv = dynamic_cast< FENodeData<mat3f>* >(&d);
				for (int n = 0; n<NN; ++n) { mat3f& v = (*pv)[n]; v *= fscale; }
			}
			break;
			default:
				break;
			}
		}
		else if (IS_ELEM_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_SCALAR:
			{
				if (fmt == DATA_NODE)
				{
					FEElementData<float, DATA_NODE>* pf = dynamic_cast<FEElementData<float, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<float, DATA_ITEM>* pf = dynamic_cast<FEElementData<float, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEElementData<float, DATA_MULT>* pf = dynamic_cast<FEElementData<float, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<float, DATA_REGION>* pf = dynamic_cast<FEElementData<float, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_VEC3:
			{
				if (fmt == DATA_NODE)
				{
					FEElementData<vec3f, DATA_NODE>* pf = dynamic_cast<FEElementData<vec3f, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<vec3f, DATA_ITEM>* pf = dynamic_cast<FEElementData<vec3f, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEElementData<vec3f, DATA_MULT>* pf = dynamic_cast<FEElementData<vec3f, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<vec3f, DATA_REGION>* pf = dynamic_cast<FEElementData<vec3f, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_MAT3S:
			{
				if (fmt == DATA_NODE)
				{
					FEElementData<mat3fs, DATA_NODE>* pf = dynamic_cast<FEElementData<mat3fs, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<mat3fs, DATA_ITEM>* pf = dynamic_cast<FEElementData<mat3fs, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEElementData<mat3fs, DATA_MULT>* pf = dynamic_cast<FEElementData<mat3fs, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<mat3fs, DATA_REGION>* pf = dynamic_cast<FEElementData<mat3fs, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_MAT3:
			{
				if (fmt == DATA_NODE)
				{
					FEElementData<mat3f, DATA_NODE>* pf = dynamic_cast<FEElementData<mat3f, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<mat3f, DATA_ITEM>* pf = dynamic_cast<FEElementData<mat3f, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEElementData<mat3f, DATA_MULT>* pf = dynamic_cast<FEElementData<mat3f, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<mat3f, DATA_REGION>* pf = dynamic_cast<FEElementData<mat3f, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			default:
				return false;
				break;
			}
		}
		else if (IS_FACE_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_SCALAR:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData<float, DATA_NODE>* pf = dynamic_cast<FEFaceData<float, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<float, DATA_ITEM>* pf = dynamic_cast<FEFaceData<float, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEFaceData<float, DATA_MULT>* pf = dynamic_cast<FEFaceData<float, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<float, DATA_REGION>* pf = dynamic_cast<FEFaceData<float, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n=0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_VEC3:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData<vec3f, DATA_NODE>* pf = dynamic_cast<FEFaceData<vec3f, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<vec3f, DATA_ITEM>* pf = dynamic_cast<FEFaceData<vec3f, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEFaceData<vec3f, DATA_MULT>* pf = dynamic_cast<FEFaceData<vec3f, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<vec3f, DATA_REGION>* pf = dynamic_cast<FEFaceData<vec3f, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_MAT3S:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData<mat3fs, DATA_NODE>* pf = dynamic_cast<FEFaceData<mat3fs, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<mat3fs, DATA_ITEM>* pf = dynamic_cast<FEFaceData<mat3fs, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_MULT)
				{
					FEFaceData<mat3fs, DATA_MULT>* pf = dynamic_cast<FEFaceData<mat3fs, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<mat3fs, DATA_REGION>* pf = dynamic_cast<FEFaceData<mat3fs, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_MAT3:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData<mat3f, DATA_NODE>* pf = dynamic_cast<FEFaceData<mat3f, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= (float) scale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<mat3f, DATA_ITEM>* pf = dynamic_cast<FEFaceData<mat3f, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= (float) scale;
				}
				else if (fmt == DATA_MULT)
				{
					FEFaceData<mat3f, DATA_MULT>* pf = dynamic_cast<FEFaceData<mat3f, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= (float) scale;
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<mat3f, DATA_REGION>* pf = dynamic_cast<FEFaceData<mat3f, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			default:
				return false;
				break;
			}
		}
		else 
		{
			break; 
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool Post::DataScaleVec3(FEPostModel& fem, int nfield, vec3d scale)
{
	FSMesh& mesh = *fem.GetFEMesh(0);

	vec3f fscale = to_vec3f(scale);

	// loop over all states
	int NN = mesh.Nodes();
	int ndata = FIELD_CODE(nfield);
	for (int i = 0; i < fem.GetStates(); ++i)
	{
		FEState& s = *fem.GetState(i);
		FEMeshData& d = s.m_Data[ndata];
		DATA_TYPE type = d.GetType();
		DATA_FORMAT fmt = d.GetFormat();
		if (IS_NODE_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_VEC3:
			{
				FENodeData<vec3f>* pv = dynamic_cast<FENodeData<vec3f>*>(&d);
				for (int n = 0; n < NN; ++n) 
				{ 
					vec3f& v = (*pv)[n]; 
					v.x *= fscale.x; 
					v.y *= fscale.y;
					v.z *= fscale.z;
				}
			}
			break;
			default:
				break;
			}
		}
		else if (IS_ELEM_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_VEC3:
			{
				if (fmt == DATA_NODE)
				{
					FEElementData<vec3f, DATA_NODE>* pf = dynamic_cast<FEElementData<vec3f, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<vec3f, DATA_ITEM>* pf = dynamic_cast<FEElementData<vec3f, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
				else if (fmt == DATA_MULT)
				{
					FEElementData<vec3f, DATA_MULT>* pf = dynamic_cast<FEElementData<vec3f, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<vec3f, DATA_REGION>* pf = dynamic_cast<FEElementData<vec3f, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
			}
			break;
			default:
				return false;
				break;
			}
		}
		else if (IS_FACE_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_VEC3:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData<vec3f, DATA_NODE>* pf = dynamic_cast<FEFaceData<vec3f, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<vec3f, DATA_ITEM>* pf = dynamic_cast<FEFaceData<vec3f, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
				else if (fmt == DATA_MULT)
				{
					FEFaceData<vec3f, DATA_MULT>* pf = dynamic_cast<FEFaceData<vec3f, DATA_MULT>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<vec3f, DATA_REGION>* pf = dynamic_cast<FEFaceData<vec3f, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n < N; ++n)
					{
						vec3f& v = (*pf)[n];
						v.x *= fscale.x;
						v.y *= fscale.y;
						v.z *= fscale.z;
					}
				}
			}
			break;
			default:
				return false;
				break;
			}
		}
		else
		{
			break;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Apply a smoothing step operation on data
bool DataSmoothStep(FEPostModel& fem, int nfield, double theta)
{
	// loop over all states
	int ndata = FIELD_CODE(nfield);
	for (int n = 0; n<fem.GetStates(); ++n)
	{
		FEState& s = *fem.GetState(n);
		FSMesh& mesh = *s.GetFEMesh();
		if (IS_NODE_FIELD(nfield))
		{
			int NN = mesh.Nodes();
			Post::FEMeshData& d = s.m_Data[ndata];
			
			switch (d.GetType())
			{
			case DATA_SCALAR:
			{
				vector<float> D; D.assign(NN, 0.f);
				vector<int> tag; tag.assign(NN, 0);
				Post::FENodeData<float>& data = dynamic_cast< Post::FENodeData<float>& >(d);

				// evaluate the average value of the neighbors
				int NE = mesh.Elements();
				for (int i=0; i<NE; ++i)
				{
					FSElement_& el = mesh.ElementRef(i);
					int ne = el.Nodes();
					for (int j=0; j<ne; ++j)
					{
						float f = data[el.m_node[j]];
						for (int k = 0; k<ne; ++k)
						if (k != j)
						{
							int nk = el.m_node[k];
							D[nk] += f;
							tag[nk]++;
						}
					}
				}

				// normalize 
				for (int i=0; i<NN; ++i) if (tag[i]>0) D[i] /= (float) tag[i];

				// assign to data field
				for (int i = 0; i<NN; ++i) { data[i] = (float)((1.0 - theta)*data[i] + theta*D[i]); }
			}
			break;
			case DATA_VEC3:
			{
				vector<vec3f> D; D.assign(NN, vec3f(0.f, 0.f, 0.f));
				vector<int> tag; tag.assign(NN, 0);
				Post::FENodeData<vec3f>& data = dynamic_cast< Post::FENodeData<vec3f>& >(d);

				// evaluate the average value of the neighbors
				int NE = mesh.Elements();
				for (int i = 0; i<NE; ++i)
				{
					FSElement_& el = mesh.ElementRef(i);
					int ne = el.Nodes();
					for (int j = 0; j<ne; ++j)
					{
						vec3f v = data[el.m_node[j]];
						for (int k = 0; k<ne; ++k)
						if (k != j)
						{
							int nk = el.m_node[k];
							D[nk] += v;
							tag[nk]++;
						}
					}
				}

				// normalize 
				for (int i = 0; i<NN; ++i) if (tag[i]>0) D[i] /= (float)tag[i];

				// assign to data field
				for (int i = 0; i<NN; ++i) { data[i] = data[i] * (1.0 - theta) + D[i]*theta; }
			}
			break;
			default:
				return false;
			}
		}
		else if (IS_ELEM_FIELD(nfield))
		{
			Post::FEMeshData& d = s.m_Data[ndata];
			if ((d.GetFormat() == DATA_ITEM)&&(d.GetType() == DATA_SCALAR))
			{
				int NE = mesh.Elements();

				vector<float> D; D.assign(NE, 0.f);
				vector<int> tag; tag.assign(NE, 0);
				Post::FEElementData<float, DATA_ITEM>& data = dynamic_cast< Post::FEElementData<float, DATA_ITEM>& >(d);

				for (int i=0; i<NE; ++i) mesh.ElementRef(i).m_ntag = i;

				// evaluate the average value of the neighbors
				for (int i=0; i<NE; ++i)
				{
					FSElement_& el = mesh.ElementRef(i);
					int nf = el.Faces();
					for (int j=0; j<nf; ++j)
					{
						FSElement_* pj = mesh.ElementPtr(el.m_nbr[j]);
						if (pj && (data.active(pj->m_ntag)))
						{
							float f;
							data.eval(pj->m_ntag, &f);
							D[i] += f;
							tag[i]++;
						}
					}
				}

				// normalize 
				for (int i=0; i<NE; ++i) if (tag[i]>0) D[i] /= (float) tag[i];

				// assign to data field
				for (int i = 0; i<NE; ++i) 
					if (data.active(i))
					{
						float f;
						data.eval(i, &f);
						D[i] =(float)((1.0 - theta)*f + theta*D[i]);
						data.set(i, D[i]);
					}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Apply a smoothing operation on data
bool Post::DataSmooth(FEPostModel& fem, int nfield, double theta, int niters)
{
	for (int n = 0; n<niters; ++n) 
	{
		if (DataSmoothStep(fem, nfield, theta) == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// functions used in arithemtic filters
double flt_add(double d, double s) { return d+s; }
double flt_sub(double d, double s) { return d-s; }
double flt_mul(double d, double s) { return d*s; }
double flt_div(double d, double s) { return d/s; }
double flt_err(double d, double s) { return fabs(d - s); }

//-----------------------------------------------------------------------------
bool Post::DataArithmetic(FEPostModel& fem, int nfield, int nop, int noperand)
{
	int ndst = FIELD_CODE(nfield);
	int nsrc = FIELD_CODE(noperand);

	FSMesh& mesh = *fem.GetFEMesh(0);

	// loop over all states
	for (int n = 0; n<fem.GetStates(); ++n)
	{
		FEState& state = *fem.GetState(n);
		FEMeshData& d = state.m_Data[ndst];
		FEMeshData& s = state.m_Data[nsrc];

		DATA_FORMAT fmt = d.GetFormat();
		if (d.GetFormat() != s.GetFormat()) return false;
		if ((d.GetType() != s.GetType()) && (s.GetType() != DATA_SCALAR)) return false;

		if (IS_NODE_FIELD(nfield) && IS_NODE_FIELD(noperand))
		{
			if ((d.GetType() == DATA_SCALAR) && (s.GetType() == DATA_SCALAR))
			{
				double(*f)(double, double) = 0;
				if      (nop == 0) f = flt_add;
				else if (nop == 1) f = flt_sub;
				else if (nop == 2) f = flt_mul;
				else if (nop == 3) f = flt_div;
				else if (nop == 4) f = flt_err;
				else
				{
					return false;
				}

				FENodeData<float>*   pd = dynamic_cast<FENodeData  <float>*>(&d);
				FENodeData_T<float>* ps = dynamic_cast<FENodeData_T<float>*>(&s);
				int N = pd->size();
				for (int i = 0; i<N; ++i) { float v; ps->eval(i, &v); (*pd)[i] = (float)f((*pd)[i], v); }
			}
			else if (d.GetType() == DATA_VEC3)
			{
				if (s.GetType() == DATA_VEC3)
				{
					FENodeData<vec3f>* pd = dynamic_cast<FENodeData<vec3f>*>(&d);
					FENodeData_T<vec3f>* ps = dynamic_cast<FENodeData_T<vec3f>*>(&s);
					int N = pd->size();
					switch (nop)
					{
					case 0: for (int i = 0; i<N; ++i) { vec3f v; ps->eval(i, &v); (*pd)[i] += v; } break;
					case 1: for (int i = 0; i<N; ++i) { vec3f v; ps->eval(i, &v); (*pd)[i] -= v; } break;
					}
				}
				else if (s.GetType() == DATA_SCALAR)
				{
					FENodeData<vec3f>* pd = dynamic_cast<FENodeData<vec3f>*>(&d);
					FENodeData_T<float>* ps = dynamic_cast<FENodeData_T<float>*>(&s);
					int N = pd->size();
					switch (nop)
					{
					case 2: for (int i = 0; i<N; ++i) { float v; ps->eval(i, &v); (*pd)[i] *= v; } break;
					case 3: for (int i = 0; i<N; ++i) { float v; ps->eval(i, &v); (*pd)[i] /= v; } break;
					}
				}
				else return false;
			}
		}
		else if (IS_ELEM_FIELD(nfield) && IS_ELEM_FIELD(noperand))
		{
			if ((d.GetType() == DATA_SCALAR) && (s.GetType() == DATA_SCALAR))
			{
				double (*f)(double,double) = 0;
				if      (nop == 0) f = flt_add;
				else if (nop == 1) f = flt_sub;
				else if (nop == 2) f = flt_mul;
				else if (nop == 3) f = flt_div;
				else if (nop == 4) f = flt_err;
				else
				{
					return false;
				}

				if (fmt == DATA_ITEM)
				{
					FEElementData<float, DATA_ITEM>* pd = dynamic_cast<FEElementData<float, DATA_ITEM>*>(&d);
					FEElemData_T<float, DATA_ITEM>* ps = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&s);
					if (pd && ps)
					{
						int N = mesh.Elements();
						for (int i = 0; i<N; ++i)
						{
							if (pd->active(i) && ps->active(i))
							{
								float vs, vd;
								pd->eval(i, &vd);
								ps->eval(i, &vs);
								float r = (float)f(vd, vs);
								pd->set(i, r);
							}
						}
					}
					else return false;
				}
				else if (fmt == DATA_NODE)
				{
					FEElementData<float, DATA_NODE>* pd = dynamic_cast<FEElementData<float, DATA_NODE>*>(&d);
					FEElemData_T<float, DATA_NODE>* ps = dynamic_cast<FEElemData_T<float, DATA_NODE>*>(&s);
					if (pd && ps)
					{
						int N = mesh.Elements();
						float vs[FSElement::MAX_NODES], vd[FSElement::MAX_NODES];
						for (int i = 0; i<N; ++i)
						{
							FSElement& el = mesh.Element(i);
							if (pd->active(i) && ps->active(i))
							{
								pd->eval(i, vd);
								ps->eval(i, vs);
								for (int j = 0; j < el.Nodes(); ++j)
								{
									float r = (float)f(vd[j], vs[j]);
									pd->set(i, j, r);
								}
							}
						}
					}
					else return false;
				}
				else if (fmt == DATA_MULT)
				{
					FEElementData<float, DATA_MULT>* pd = dynamic_cast<FEElementData<float, DATA_MULT>*>(&d);
					FEElemData_T<float, DATA_MULT>* ps = dynamic_cast<FEElemData_T<float, DATA_MULT>*>(&s);
					if (pd && ps)
					{
						int N = mesh.Elements();
						for (int i = 0; i < N; ++i)
						{
							FSElement& el = mesh.Element(i);
							float vs[FSElement::MAX_NODES] = { 0.f }, vd[FSElement::MAX_NODES] = { 0.f }, vr[FSElement::MAX_NODES] = { 0.f };
							if (pd->active(i) || ps->active(i))
							{
								if (pd->active(i)) pd->eval(i, vd);
								if (ps->active(i)) ps->eval(i, vs);

								for (int j = 0; j < el.Nodes(); ++j)
								{
									vr[j] = (float)f(vd[j], vs[j]);
								}
								pd->add(i, el.Nodes(), vr);
							}
						}
					}
					else return false;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<float, DATA_REGION>* pd = dynamic_cast<FEElementData<float, DATA_REGION>*>(&d);
					FEElemData_T<float, DATA_REGION>* ps = dynamic_cast<FEElemData_T<float, DATA_REGION>*>(&s);
					if (pd && ps)
					{
						int N = mesh.Elements();
						for (int i = 0; i < N; ++i)
						{
							FSElement& el = mesh.Element(i);
							float vs = 0.f, vd = 0.f;
							if (pd->active(i) || ps->active(i))
							{
								if (pd->active(i)) pd->eval(i, &vd);
								if (ps->active(i)) ps->eval(i, &vs);

								float vr = (float)f(vd, vs);
								pd->add(i, vr);
							}
						}
					}
					else return false;
				}
				else
				{
					return false;
				}
			}
			else if (d.GetType() == DATA_MAT3S)
			{
				if (s.GetType() == DATA_MAT3S)
				{
					if (fmt == DATA_ITEM)
					{
						FEElementData<mat3fs, DATA_ITEM>* pd = dynamic_cast<FEElementData<mat3fs, DATA_ITEM>*>(&d);
						FEElemData_T<mat3fs, DATA_ITEM>* ps = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&s);
						if (pd && ps)
						{
							int N = mesh.Elements();
							switch (nop)
							{
							case 0: for (int i = 0; i<N; ++i) if (pd->active(i) && (ps->active(i))) { mat3fs s, d, r; pd->eval(i, &d); ps->eval(i, &s); pd->set(i, d + s); } break;
							case 1: for (int i = 0; i<N; ++i) if (pd->active(i) && (ps->active(i))) { mat3fs s, d, r; pd->eval(i, &d); ps->eval(i, &s); pd->set(i, d - s); } break;
							default:
								{
									return false;
								}
							}
						}
					}
					else return false;
				}
				else if (s.GetType() == DATA_SCALAR)
				{
					if (fmt == DATA_ITEM)
					{
						FEElementData<mat3fs, DATA_ITEM>* pd = dynamic_cast<FEElementData<mat3fs, DATA_ITEM>*>(&d);
						FEElemData_T<float, DATA_ITEM>* ps = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&s);
						if (pd && ps)
						{
							mat3fs I(1.f, 1.f, 1.f, 0.f, 0.f, 0.f);
							int N = mesh.Elements();
							switch (nop)
							{
							case 0: for (int i = 0; i<N; ++i) if (pd->active(i) && (ps->active(i))) { mat3fs d, r; float s; pd->eval(i, &d); ps->eval(i, &s); pd->set(i, d + I*s); } break;
							case 1: for (int i = 0; i<N; ++i) if (pd->active(i) && (ps->active(i))) { mat3fs d, r; float s; pd->eval(i, &d); ps->eval(i, &s); pd->set(i, d - I*s); } break;
							case 2: for (int i = 0; i<N; ++i) if (pd->active(i) && (ps->active(i))) { mat3fs d, r; float s; pd->eval(i, &d); ps->eval(i, &s); pd->set(i, d*s); } break;
							case 3: for (int i = 0; i<N; ++i) if (pd->active(i) && (ps->active(i))) { mat3fs d, r; float s; pd->eval(i, &d); ps->eval(i, &s); pd->set(i, d/s); } break;
							default:
								{
									return false;
								}
							}
						}
						else return false;
					}
					else return false;
				}
				else
				{
					return false;
				}
			}
			else if ((d.GetType() == DATA_VEC3) && (s.GetType() == DATA_VEC3))
			{
				double (*f)(double, double) = 0;
				if      (nop == 0) f = flt_add;
				else if (nop == 1) f = flt_sub;
				else
				{
					return false;
				}

				if (fmt == DATA_ITEM)
				{
					FEElementData<vec3f, DATA_ITEM>* pd = dynamic_cast<FEElementData<vec3f, DATA_ITEM>*>(&d);
					FEElemData_T<vec3f, DATA_ITEM>* ps = dynamic_cast<FEElemData_T<vec3f, DATA_ITEM>*>(&s);
					if (pd && ps)
					{
						int N = mesh.Elements();
						for (int i = 0; i < N; ++i)
						{
							if (pd->active(i) && ps->active(i))
							{
								vec3f vs, vd;
								pd->eval(i, &vd);
								ps->eval(i, &vs);
								float x = (float)f(vd.x, vs.x);
								float y = (float)f(vd.y, vs.y);
								float z = (float)f(vd.z, vs.z);
								pd->set(i, vec3f(x,y,z));
							}
						}
					}
					else return false;
				}
				else return false;
			}
			else
			{
				return false;
			}
		}
		else if (IS_FACE_FIELD(nfield) && IS_FACE_FIELD(noperand))
		{
			if ((d.GetType() == DATA_SCALAR) && (s.GetType() == DATA_SCALAR))
			{
				double (*f)(double, double) = 0;
				if (nop == 0) f = flt_add;
				else if (nop == 1) f = flt_sub;
				else if (nop == 2) f = flt_mul;
				else if (nop == 3) f = flt_div;
				else if (nop == 4) f = flt_err;
				else
				{
					return false;
				}

				if (fmt == DATA_ITEM)
				{
					FEFaceData<float, DATA_ITEM>* pd = dynamic_cast<FEFaceData<float, DATA_ITEM>*>(&d);
					FEFaceData_T<float, DATA_ITEM>* ps = dynamic_cast<FEFaceData_T<float, DATA_ITEM>*>(&s);
					if (pd && ps)
					{
						int N = mesh.Elements();
						for (int i = 0; i < N; ++i)
						{
							if (pd->active(i) && ps->active(i))
							{
								float vs, vd;
								pd->eval(i, &vd);
								ps->eval(i, &vs);
								float r = (float)f(vd, vs);
								pd->set(i, r);
							}
						}
					}
					else return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

// functions used in math filter
double math_neg(double v) { return -v; }
double math_abs(double v) { return fabs(v); }
double math_ramp(double v) { return (v > 0 ? v : 0.0); }

bool Post::DataMath(FEPostModel& fem, int nfield, int nop)
{
	int ndst = FIELD_CODE(nfield);

	FSMesh& mesh = *fem.GetFEMesh(0);

	double (*f)(double) = 0;
	switch (nop)
	{
	case 0: f = math_neg; break;
	case 1: f = math_abs; break;
	case 2: f = math_ramp; break;
		default:
		return false;
	}

	// loop over all states
	for (int n = 0; n < fem.GetStates(); ++n)
	{
		FEState& state = *fem.GetState(n);
		FEMeshData& d = state.m_Data[ndst];
		if (d.GetType() != DATA_SCALAR) return false;

		DATA_FORMAT fmt = d.GetFormat();
		if (IS_NODE_FIELD(nfield))
		{
			FENodeData<float>* pd = dynamic_cast<FENodeData  <float>*>(&d); assert(pd);
			if (pd == 0) return false;
			int N = pd->size();
			for (int i = 0; i < N; ++i) { (*pd)[i] = (float)f((*pd)[i]); }
		}
		else if (IS_ELEM_FIELD(nfield))
		{
			if (fmt == DATA_ITEM)
			{
				FEElementData<float, DATA_ITEM>* pd = dynamic_cast<FEElementData<float, DATA_ITEM>*>(&d); assert(pd);
				if (pd == 0) return false;
				int N = mesh.Elements();
				for (int i = 0; i < N; ++i)
				{
					if (pd->active(i))
					{
						float vd;
						pd->eval(i, &vd);
						float r = (float)f(vd);
						pd->set(i, r);
					}
				}
			}
			else if (fmt == DATA_NODE)
			{
				FEElementData<float, DATA_NODE>* pd = dynamic_cast<FEElementData<float, DATA_NODE>*>(&d); assert(pd);
				if (pd == 0) return false;
				int N = mesh.Elements();
				float vd[FSElement::MAX_NODES];
				for (int i = 0; i < N; ++i)
				{
					FSElement& el = mesh.Element(i);
					if (pd->active(i))
					{
						pd->eval(i, vd);
						for (int j = 0; j < el.Nodes(); ++j)
						{
							float r = (float)f(vd[j]);
							pd->set(i, j, r);
						}
					}
				}
			}
			else if (fmt == DATA_MULT)
			{
				FEElementData<float, DATA_MULT>* pd = dynamic_cast<FEElementData<float, DATA_MULT>*>(&d); assert(pd);
				if (pd == 0) return false;
				int N = mesh.Elements();
				for (int i = 0; i < N; ++i)
				{
					FSElement& el = mesh.Element(i);
					float vd[FSElement::MAX_NODES] = { 0.f }, vr[FSElement::MAX_NODES] = { 0.f };
					if (pd->active(i))
					{
						pd->eval(i, vd);
						for (int j = 0; j < el.Nodes(); ++j)
						{
							vr[j] = (float)f(vd[j]);
						}
						pd->add(i, el.Nodes(), vr);
					}
				}
			}
			else if (fmt == DATA_REGION)
			{
				FEElementData<float, DATA_REGION>* pd = dynamic_cast<FEElementData<float, DATA_REGION>*>(&d); assert(pd);
				if (pd == 0) return false;
				int N = mesh.Elements();
				for (int i = 0; i < N; ++i)
				{
					FSElement& el = mesh.Element(i);
					float vd = 0.f;
					if (pd->active(i))
					{
						pd->eval(i, &vd);
						float vr = (float)f(vd);
						pd->add(i, vr);
					}
				}
			}
			else
			{
				return false;
			}
		}
		else if (IS_FACE_FIELD(nfield))
		{
			if (fmt == DATA_ITEM)
			{
				FEFaceData<float, DATA_ITEM>* pd = dynamic_cast<FEFaceData<float, DATA_ITEM>*>(&d); assert(pd);
				if (pd == 0) return false;
				int N = mesh.Elements();
				for (int i = 0; i < N; ++i)
				{
					if (pd->active(i))
					{
						float vd;
						pd->eval(i, &vd);
						float r = (float)f(vd);
						pd->set(i, r);
					}
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
bool Post::DataGradient(FEPostModel& fem, int vecField, int sclField, int config)
{
	int nvec = FIELD_CODE(vecField);
	int nscl = FIELD_CODE(sclField);

	// loop over all the states
	for (int n=0; n<fem.GetStates(); ++n)
	{
		FEState& state = *fem.GetState(n);
		FEMeshData& v = state.m_Data[nvec];
		FEMeshData& s = state.m_Data[nscl];

		// zero the vector field
		if (IS_NODE_FIELD(vecField) && (v.GetType() == DATA_VEC3))
		{
			FENodeData<vec3f>* pv = dynamic_cast<FENodeData<vec3f>*>(&v);
			int N = pv->size();
			for (int i = 0; i<N; ++i) (*pv)[i] = vec3f(0,0,0);
		}
		else return false;

		// get the mesh
		FSMesh* mesh = state.GetFEMesh();

		// evaluate the field over all the nodes
		const int NN = mesh->Nodes();
		vector<double> d(NN, 0.f);

		if (s.GetType() == DATA_SCALAR)
		{
			if (IS_NODE_FIELD(sclField))
			{
				FENodeData_T<float>* ps = dynamic_cast<FENodeData_T<float>*>(&s); assert(ps);
				for (int i=0; i<NN; ++i) 
				{	
					float f;	
					ps->eval(i, &f);
					d[i] = (double) f;
				}
			}
			else if (IS_ELEM_FIELD(sclField))
			{
				if (s.GetFormat() == DATA_NODE)
				{
					vector<int> tag(NN, 0);
					FEElemData_T<float, DATA_NODE>* ps = dynamic_cast<FEElemData_T<float, DATA_NODE>*>(&s);

					float ed[FSElement::MAX_NODES] = {0.f};
					for (int i=0; i<mesh->Elements(); ++i)
					{
						FSElement_& el = mesh->ElementRef(i);
						if (ps->active(i))
						{
							ps->eval(i, ed);
							for (int j=0; j<el.Nodes(); ++j)
							{
								d[el.m_node[j]] += ed[j];
								tag[el.m_node[j]]++;
							}
						}
					}
					for (int i=0; i<NN; ++i)
						if (tag[i] > 0) d[i] /= (double) tag[i];
				}
				else if (s.GetFormat() == DATA_ITEM)
				{
					vector<int> tag(NN, 0);
					FEElemData_T<float, DATA_ITEM>* ps = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&s);

					float ed =  0.f;
					for (int i = 0; i<mesh->Elements(); ++i)
					{
						FSElement_& el = mesh->ElementRef(i);
						if (ps->active(i))
						{
							ps->eval(i, &ed);
							for (int j = 0; j<el.Nodes(); ++j)
							{
								d[el.m_node[j]] += ed;
								tag[el.m_node[j]]++;
							}
						}
					}
					for (int i = 0; i<NN; ++i)
						if (tag[i] > 0) d[i] /= (double)tag[i];
				}
				else if (s.GetFormat() == DATA_MULT)
				{
					vector<int> tag(NN, 0);
					FEElemData_T<float, DATA_MULT>* ps = dynamic_cast<FEElemData_T<float, DATA_MULT>*>(&s);

					float ed[FSElement::MAX_NODES] = { 0.f };
					for (int i = 0; i<mesh->Elements(); ++i)
					{
						FSElement_& el = mesh->ElementRef(i);
						if (ps->active(i))
						{
							ps->eval(i, ed);
							for (int j = 0; j<el.Nodes(); ++j)
							{
								d[el.m_node[j]] += ed[j];
								tag[el.m_node[j]]++;
							}
						}
					}
					for (int i = 0; i<NN; ++i)
						if (tag[i] > 0) d[i] /= (double)tag[i];
				}
			}
		}

		// now, calculate the gradient for each element
		vector<vec3f> G(NN, vec3f(0.f, 0.f, 0.f));
		vec3f eg[FSElement::MAX_NODES];
		float ed[FSElement::MAX_NODES];
		vector<int> tag(NN, 0);
		for (int i=0; i<mesh->Elements(); ++i)
		{
			FSElement_& el = mesh->ElementRef(i);

			for (int j = 0; j<el.Nodes(); ++j) ed[j] = (float)d[el.m_node[j]];

			for (int j=0; j<el.Nodes(); ++j)
			{
				// get the iso-coords at the nodes
				double q[3] = {0,0,0};
				el.iso_coord(j, q);

				// evaluate the gradient at the node
				if (config == 1)
					shape_grad(fem, i, q, n, eg);
				else 
					shape_grad_ref(fem, i, q, n, eg);

				vec3f grad(0.f, 0.f, 0.f);
				for (int k=0; k<el.Nodes(); ++k) grad += eg[k] * ed[k];
				
				G[el.m_node[j]] += grad;
				tag[el.m_node[j]]++;
			}
		}

		FENodeData<vec3f>* pv = dynamic_cast<FENodeData<vec3f>*>(&v);
		for (int i = 0; i<NN; ++i)
		{
			if (tag[i] > 0) G[i] /= (float) tag[i];
			(*pv)[i] = G[i];
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
template <typename T> void extractNodeDataComponent_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FENodeData_T<T>& vec = dynamic_cast<FENodeData_T<T>&>(src);
	Post::FENodeData<float>& scl = dynamic_cast<Post::FENodeData<float>&>(dst);

	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		T v; vec.eval(i, &v);
		scl[i] = component(v, ncomp);
	}
}

void extractNodeDataComponent(DATA_TYPE ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	switch (ntype)
	{
	case DATA_VEC3  : extractNodeDataComponent_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3S : extractNodeDataComponent_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3SD: extractNodeDataComponent_T<mat3fd >(dst, src, ncomp, mesh); break;
	case DATA_TENS4S: extractNodeDataComponent_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3  : extractNodeDataComponent_T<mat3f  >(dst, src, ncomp, mesh); break;
	}
}

template <typename T> void extractElemDataComponentITEM_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FEElemData_T<T, DATA_ITEM>& vec = dynamic_cast<FEElemData_T<T, DATA_ITEM>&>(src);
	Post::FEElementData<float, DATA_ITEM>& scl = dynamic_cast<Post::FEElementData<float, DATA_ITEM>&>(dst);

	int NE = mesh.Elements();
	for (int i = 0; i<NE; ++i)
	{
		if (vec.active(i))
		{
			T v; vec.eval(i, &v);
			scl.add(i, component(v, ncomp));
		}
	}
}

void extractElemDataComponentITEM_ARRAY(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FEElemArrayDataItem& vec = dynamic_cast<FEElemArrayDataItem&>(src);
	Post::FEElementData<float, DATA_ITEM>& scl = dynamic_cast<Post::FEElementData<float, DATA_ITEM>&>(dst);

	int NE = mesh.Elements();
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			float f = vec.eval(i, ncomp);
			scl.add(i, f);
		}
	}
}

void extractElemDataComponentITEM_ARRAY_VEC3F(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FEElemArrayVec3Data& vec = dynamic_cast<FEElemArrayVec3Data&>(src);
	Post::FEElementData<float, DATA_ITEM>& scl = dynamic_cast<Post::FEElementData<float, DATA_ITEM>&>(dst);

	int index = ncomp / 4;
	int veccomp = ncomp % 4;

	int NE = mesh.Elements();
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			vec3f v = vec.eval(i, index);
			float f = component2(v, veccomp);
			scl.add(i, f);
		}
	}
}

void extractElemDataComponentITEM(DATA_TYPE ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3  : extractElemDataComponentITEM_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3S : extractElemDataComponentITEM_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3SD: extractElemDataComponentITEM_T<mat3fd >(dst, src, ncomp, mesh); break;
	case DATA_TENS4S: extractElemDataComponentITEM_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3  : extractElemDataComponentITEM_T<mat3f  >(dst, src, ncomp, mesh); break;
	case DATA_ARRAY      : extractElemDataComponentITEM_ARRAY(dst, src, ncomp, mesh); break;
	case DATA_ARRAY_VEC3: extractElemDataComponentITEM_ARRAY_VEC3F(dst, src, ncomp, mesh); break;
	}
}

template <typename T> void extractElemDataComponentNODE_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FEElemData_T<T, DATA_NODE>& vec = dynamic_cast<FEElemData_T<T, DATA_NODE>&>(src);
	Post::FEElementData<float, DATA_NODE>& scl = dynamic_cast<Post::FEElementData<float, DATA_NODE>&>(dst);

	mesh.TagAllNodes(-1);
	int n = 0;
	int ne_max = 0;
	int NE = mesh.Elements();
	vector<int> elemList; elemList.reserve(NE);
	for (int i = 0; i < NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (vec.active(i))
		{
			elemList.push_back(i);
			int ne = el.Nodes();
			if (ne > ne_max) ne_max = ne;
			for (int j = 0; j < ne; ++j) mesh.Node(el.m_node[j]).m_ntag = n++;
		}
	}
	if (n == 0) return;

	vector<float> val(n, 0.f);
	vector<int> l(ne_max * elemList.size());
	for (int i : elemList)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		for (int j = 0; j < ne; ++j) l[i * ne_max + j] = mesh.Node(el.m_node[j]).m_ntag;
	}

	T data[FSElement::MAX_NODES];
	for (int i : elemList)
	{
		FSElement_& el = mesh.ElementRef(i);
		vec.eval(i, data);

		int ne = el.Nodes();
		for (int j = 0; j < ne; ++j)
		{
			float f = component(data[j], ncomp);
			val[l[i * ne_max + j]] = f;
		}
	}

	scl.add(val, elemList, l, ne_max);
}

void extractElemDataComponentNODE_ARRAY(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FEElemArrayDataNode& vec = dynamic_cast<FEElemArrayDataNode&>(src);
	Post::FEElementData<float, DATA_NODE>& scl = dynamic_cast<Post::FEElementData<float, DATA_NODE>&>(dst);

	int NE = mesh.Elements();
	float val[FSElement::MAX_NODES];
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			vec.eval(i, ncomp, val);

			data.resize(ne);
			for (int j = 0; j<ne; ++j) data[j] = val[j];

			l.resize(ne);
			for (int j = 0; j<ne; ++j) l[j] = j;

			elem[0] = i;

			scl.add(data, elem, l, ne);
		}
	}
}

void extractElemDataComponentNODE(DATA_TYPE ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3  : extractElemDataComponentNODE_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3S : extractElemDataComponentNODE_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3SD: extractElemDataComponentNODE_T<mat3fd >(dst, src, ncomp, mesh); break;
	case DATA_TENS4S: extractElemDataComponentNODE_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3  : extractElemDataComponentNODE_T<mat3f  >(dst, src, ncomp, mesh); break;
	case DATA_ARRAY  : extractElemDataComponentNODE_ARRAY(dst, src, ncomp, mesh); break;
	}
}


template <typename T> void extractElemDataComponentCOMP_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	FEElemData_T<T, DATA_MULT>& vec = dynamic_cast<FEElemData_T<T, DATA_MULT>&>(src);
	Post::FEElementData<float, DATA_MULT>& scl = dynamic_cast<Post::FEElementData<float, DATA_MULT>&>(dst);

	int NE = mesh.Elements();
	T val[FSElement::MAX_NODES];
	vector<float> data;
	for (int i = 0; i < NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			vec.eval(i, val);

			data.resize(ne);
			for (int j = 0; j < ne; ++j) data[j] = component(val[j], ncomp);

			scl.add(i, ne, data.data());
		}
	}
}

void extractElemDataComponentCOMP(DATA_TYPE ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, FSMesh& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3  : extractElemDataComponentNODE_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3S : extractElemDataComponentCOMP_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3SD: extractElemDataComponentNODE_T<mat3fd >(dst, src, ncomp, mesh); break;
	case DATA_TENS4S: extractElemDataComponentNODE_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3  : extractElemDataComponentNODE_T<mat3f  >(dst, src, ncomp, mesh); break;
//	case DATA_ARRAY  : extractElemDataComponentNODE_ARRAY(dst, src, ncomp, mesh); break;
	}
}

ModelDataField* Post::DataComponent(FEPostModel& fem, ModelDataField* pdf, int ncomp, const std::string& sname)
{
	if (pdf == 0) return 0;

	int nclass = pdf->DataClass();
	DATA_TYPE ntype = pdf->Type();
	int nfmt = pdf->Format();

	FSMesh& mesh = *fem.GetFEMesh(0);

	ModelDataField* newField = 0;
	if (nclass == NODE_DATA)
	{
		newField = new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA);
		fem.AddDataField(newField, sname);

		int nvec = pdf->GetFieldID(); nvec = FIELD_CODE(nvec);
		int nscl = newField->GetFieldID(); nscl = FIELD_CODE(nscl);

		for (int n=0; n<fem.GetStates(); ++n)
		{
			FEState* state = fem.GetState(n);
			extractNodeDataComponent(ntype, state->m_Data[nscl], state->m_Data[nvec], ncomp, mesh);
		}
	}
	else if (nclass == FACE_DATA)
	{
		
	}
	else if (nclass == ELEM_DATA)
	{
		if (nfmt == DATA_ITEM)
		{
			newField = new FEDataField_T<FEElementData<float, DATA_ITEM> >(&fem, EXPORT_DATA);
			fem.AddDataField(newField, sname);

			int nvec = pdf->GetFieldID(); nvec = FIELD_CODE(nvec);
			int nscl = newField->GetFieldID(); nscl = FIELD_CODE(nscl);

			for (int n = 0; n<fem.GetStates(); ++n)
			{
				FEState* state = fem.GetState(n);
				extractElemDataComponentITEM(ntype, state->m_Data[nscl], state->m_Data[nvec], ncomp, mesh);
			}
		}
		else if (nfmt == DATA_NODE)
		{
			newField = new FEDataField_T<FEElementData<float, DATA_NODE> >(&fem, EXPORT_DATA);
			fem.AddDataField(newField, sname);

			int nvec = pdf->GetFieldID(); nvec = FIELD_CODE(nvec);
			int nscl = newField->GetFieldID(); nscl = FIELD_CODE(nscl);

			for (int n = 0; n<fem.GetStates(); ++n)
			{
				FEState* state = fem.GetState(n);
				extractElemDataComponentNODE(ntype, state->m_Data[nscl], state->m_Data[nvec], ncomp, mesh);
			}
		}
		else if (nfmt == DATA_MULT)
		{
			newField = new FEDataField_T<FEElementData<float, DATA_MULT> >(&fem, EXPORT_DATA);
			fem.AddDataField(newField, sname);

			int nvec = pdf->GetFieldID(); nvec = FIELD_CODE(nvec);
			int nscl = newField->GetFieldID(); nscl = FIELD_CODE(nscl);

			for (int n = 0; n < fem.GetStates(); ++n)
			{
				FEState* state = fem.GetState(n);
				extractElemDataComponentCOMP(ntype, state->m_Data[nscl], state->m_Data[nvec], ncomp, mesh);
			}
		}
	}

	return newField;
}

//-----------------------------------------------------------------------------
// Calculate the fractional anisotropy of a tensor field
bool Post::DataFractionalAnsisotropy(FEPostModel& fem, int scalarField, int tensorField)
{
	int ntns = FIELD_CODE(tensorField);
	int nscl = FIELD_CODE(scalarField);

	// loop over all the states
	for (int n = 0; n<fem.GetStates(); ++n)
	{
		FEState& state = *fem.GetState(n);
		FEMeshData& v = state.m_Data[ntns];
		FEMeshData& s = state.m_Data[nscl];

		// zero the scalar field
		Post::FEElementData<float, DATA_ITEM>* ps = nullptr;
		if (IS_ELEM_FIELD(scalarField) && (s.GetType() == DATA_SCALAR))
		{
			ps = dynamic_cast<Post::FEElementData<float, DATA_ITEM>*>(&s);
			int N = ps->size();
			for (int i = 0; i < N; ++i) (*ps)[i] = 0.f;
		}
		else return false;

		// get the mesh
		FSMesh* mesh = state.GetFEMesh();

		// evaluate the field
		if (v.GetType() == DATA_MAT3S)
		{
			if (IS_ELEM_FIELD(tensorField) && (v.GetFormat() == DATA_ITEM))
			{
				FEElemData_T<mat3fs, DATA_ITEM>* pv = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&v);

				mat3fs ev;
				for (int i = 0; i<mesh->Elements(); ++i)
				{
					FSElement_& el = mesh->ElementRef(i);
					if (pv->active(i))
					{
						pv->eval(i, &ev);

						float fa = (float)fractional_anisotropy(ev);
						ps->add(i, fa);
					}
				}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// convert between formats
ModelDataField* Post::DataConvert(FEPostModel& fem, ModelDataField* dataField, int newClass, int newFormat, const std::string& name)
{
	if (dataField == nullptr) return nullptr;

	int nclass = dataField->DataClass();
	DATA_TYPE ntype = dataField->Type();
	int nfmt = dataField->Format();

	FSMesh& mesh = *fem.GetFEMesh(0);

	ModelDataField* newField = nullptr;
	if (ntype == DATA_SCALAR)
	{
		if ((nclass == ELEM_DATA) && (newClass == ELEM_DATA))
		{
			if ((nfmt == DATA_ITEM) && (newFormat == DATA_NODE))
			{
				newField = new FEDataField_T<FEElementData<float, DATA_NODE> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NN = mesh.Nodes();
				int NE = mesh.Elements();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<float> data(NN, 0.f);
					vector<int> tag(NN, 0);

					FEElemData_T<float, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&state->m_Data[nold]);
					FEElementData<float, DATA_NODE>* pnew = dynamic_cast<FEElementData<float, DATA_NODE>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							float v = 0.0;
							pold->eval(i, &v);

							FSElement& el = mesh.Element(i);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v;
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i) if (tag[i] != 0) data[i] /= (float)tag[i];

					vector<float> d;
					vector<int> e(1);
					vector<int> l;
					for (int i = 0; i < NE; ++i)
					{
						FSElement& el = mesh.Element(i);
						e[0] = i;
						l.resize(el.Nodes());
						d.resize(el.Nodes());
						for (int j = 0; j < el.Nodes(); ++j)
						{
							d[j] = data[el.m_node[j]];
							l[j] = j;
						}
						pnew->add(d, e, l, el.Nodes());
					}
				}
			}
			else if ((nfmt == DATA_NODE) && (newFormat == DATA_ITEM))
			{
				newField = new FEDataField_T<FEElementData<float, DATA_ITEM> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NN = mesh.Nodes();
				int NE = mesh.Elements();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					FEElemData_T<float, DATA_NODE>* pold = dynamic_cast<FEElemData_T<float, DATA_NODE>*>(&state->m_Data[nold]);
					FEElementData<float, DATA_ITEM>* pnew = dynamic_cast<FEElementData<float, DATA_ITEM>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							int ne = el.Nodes();

							float v[FSElement::MAX_NODES] = { 0.f };
							pold->eval(i, v);

							float avg = 0.f;
							for (int j = 0; j < ne; ++j) avg += v[j];
							avg /= (float)ne;

							pnew->add(i, avg);
						}
					}
				}
			}
			else if ((nfmt == DATA_MULT) && (newFormat == DATA_ITEM))
			{
				newField = new FEDataField_T<FEElementData<float, DATA_ITEM> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NN = mesh.Nodes();
				int NE = mesh.Elements();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<float> data(NN, 0.f);
					vector<int> tag(NN, 0);

					FEElemData_T<float, DATA_MULT>* pold = dynamic_cast<FEElemData_T<float, DATA_MULT>*>(&state->m_Data[nold]);
					FEElementData<float, DATA_ITEM>* pnew = dynamic_cast<FEElementData<float, DATA_ITEM>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							float v[FSElement::MAX_NODES] = { 0.f };
							pold->eval(i, v);

							FSElement& el = mesh.Element(i);
							int ne = el.Nodes();
							float avg = 0.f;
							for (int j = 0; j < ne; ++j) avg += v[j];
							avg /= (float)ne;

							pnew->add(i, avg);
						}
					}
				}
			}
			else if ((nfmt == DATA_MULT) && (newFormat == DATA_NODE))
			{
				newField = new FEDataField_T<FEElementData<float, DATA_NODE> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NN = mesh.Nodes();
				int NE = mesh.Elements();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<float> data(NN, 0.f);
					vector<int> tag(NN, 0);
					vector<int> elem;

					FEElemData_T<float, DATA_MULT>* pold = dynamic_cast<FEElemData_T<float, DATA_MULT>*>(&state->m_Data[nold]);
					FEElementData<float, DATA_NODE>* pnew = dynamic_cast<FEElementData<float, DATA_NODE>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							float v[FSElement::MAX_NODES] = { 0.f };
							pold->eval(i, v);

							FSElement& el = mesh.Element(i);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v[j];
								tag[el.m_node[j]] += 1;
							}

							elem.push_back(i);
						}
					}

					int nc = 0;
					for (int i = 0; i < NN; ++i)
					{
						float w = (float)tag[i];
						if (w != 0.f)
						{
							data[i] /= w;
							tag[i] = nc++;
						}
						else tag[i] = -1;
					}

					vector<float> nodeData(nc, 0.f);
					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] >= 0) nodeData[i] = data[tag[i]];
					}

					vector<int> index;
					for (int i = 0; i < elem.size(); ++i)
					{
						FSElement& el = mesh.Element(elem[i]);
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j)
						{
							index.push_back(tag[el.m_node[j]]);
						}
					}

					// TODO: This will only work if all elements have the same nr of nodes!!
					int ne = mesh.Element(elem[0]).Nodes();
					pnew->add(nodeData, elem, index, ne);
				}
			}
			else if ((nfmt == DATA_REGION) && (newFormat == DATA_MULT))
			{
				newField = new FEDataField_T<FEElementData<float, DATA_MULT> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NE = mesh.Elements();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					FEElemData_T<float, DATA_REGION>* pold = dynamic_cast<FEElemData_T<float, DATA_REGION>*>(&state->m_Data[nold]);
					FEElementData<float, DATA_MULT>* pnew = dynamic_cast<FEElementData<float, DATA_MULT>*>(&state->m_Data[nnew]);

					float vr[FSElement::MAX_NODES] = { 0.f };
					for (int i = 0; i < NE; ++i)
					{
						FSElement& el = mesh.Element(i);
						if (pold->active(i))
						{
							float v = 0.f;
							pold->eval(i, &v);

							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j) vr[j] = v;

							pnew->add(i, ne, vr);
						}
					}
				}
			}
		}
		else if ((nclass == ELEM_DATA) && (newClass == NODE_DATA))
		{
			int NN = mesh.Nodes();
			int NE = mesh.Elements();

			if (nfmt == DATA_ITEM)
			{
				newField = new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<float> data(NN, 0.f);
					vector<int> tag(NN, 0);

					FEElemData_T<float, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&state->m_Data[nold]);
					FENodeData<float>* pnew = dynamic_cast<FENodeData<float>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							float v = 0.f; pold->eval(i, &v);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v;
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] != 0) data[i] /= (float)tag[i];
					}

					for (int i = 0; i < NN; ++i) (*pnew)[i] = data[i];
				}
			}
			else if (nfmt == DATA_NODE)
			{
				newField = new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<float> data(NN, 0.f);
					vector<int> tag(NN, 0);

					FEElemData_T<float, DATA_NODE>* pold = dynamic_cast<FEElemData_T<float, DATA_NODE>*>(&state->m_Data[nold]);
					FENodeData<float>* pnew = dynamic_cast<FENodeData<float>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							float v[FSElement::MAX_NODES] = { 0.f }; pold->eval(i, v);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v[j];
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] != 0) data[i] /= (float)tag[i];
					}

					for (int i = 0; i < NN; ++i) (*pnew)[i] = data[i];
				}
			}
			else if (nfmt == DATA_MULT)
			{
				newField = new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<float> data(NN, 0.f);
					vector<int> tag(NN, 0);

					FEElemData_T<float, DATA_MULT>* pold = dynamic_cast<FEElemData_T<float, DATA_MULT>*>(&state->m_Data[nold]);
					FENodeData<float>* pnew = dynamic_cast<FENodeData<float>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							float v[FSElement::MAX_NODES] = { 0.f }; pold->eval(i, v);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v[j];
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] != 0) data[i] /= (float)tag[i];
					}

					for (int i = 0; i < NN; ++i) (*pnew)[i] = data[i];
				}
			}
		}
		else if ((nclass == ELEM_DATA) && (newClass == FACE_DATA))
		{
			if ((nfmt == DATA_ITEM) && (newFormat == DATA_ITEM))
			{
				newField = new FEDataField_T<FEFaceData<float, DATA_ITEM> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NE = mesh.Elements();
				int NF = mesh.Faces();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					FEElemData_T<float, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&state->m_Data[nold]);
					FEFaceData<float, DATA_ITEM>* pnew = dynamic_cast<FEFaceData<float, DATA_ITEM>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NF; ++i)
					{
						FSFace& face = mesh.Face(i);
						if (face.IsExternal())
						{
							int eid = face.m_elem[0].eid;
							if (pold->active(eid))
							{
								float v; pold->eval(eid, &v);
								pnew->add(i, v);
							}
						}
					}
				}
			}
		}
	}
	else if (ntype == DATA_VEC3)
	{
		if ((nclass == ELEM_DATA) && (newClass == NODE_DATA))
		{
			int NN = mesh.Nodes();
			int NE = mesh.Elements();

			if (nfmt == DATA_ITEM)
			{
				newField = new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<vec3f> data(NN);
					vector<int> tag(NN, 0);

					FEElemData_T<vec3f, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<vec3f, DATA_ITEM>*>(&state->m_Data[nold]);
					FENodeData<vec3f>* pnew = dynamic_cast<FENodeData<vec3f>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							vec3f v; pold->eval(i, &v);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v;
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] != 0) data[i] /= (float)tag[i];
					}

					for (int i = 0; i < NN; ++i) (*pnew)[i] = data[i];
				}
			}
		}
		else if ((nclass == ELEM_DATA) && (newClass == FACE_DATA))
		{
			if ((nfmt == DATA_ITEM) && (newFormat == DATA_ITEM))
			{
				newField = new FEDataField_T<FEFaceData<vec3f, DATA_ITEM> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NE = mesh.Elements();
				int NF = mesh.Faces();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					FEElemData_T<vec3f, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<vec3f, DATA_ITEM>*>(&state->m_Data[nold]);
					FEFaceData<vec3f, DATA_ITEM>* pnew = dynamic_cast<FEFaceData<vec3f, DATA_ITEM>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NF; ++i)
					{
						FSFace& face = mesh.Face(i);
						if (face.IsExternal())
						{
							int eid = face.m_elem[0].eid;
							if (pold->active(eid))
							{
								vec3f v; pold->eval(eid, &v);
								pnew->add(i, v);
							}
						}
					}
				}
			}
		}
	}
	else if (ntype == DATA_MAT3S)
	{
		if ((nclass == ELEM_DATA) && (newClass == NODE_DATA))
		{
			int NN = mesh.Nodes();
			int NE = mesh.Elements();

			if (nfmt == DATA_ITEM)
			{
				newField = new FEDataField_T<FENodeData<mat3fs> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<mat3fs> data(NN);
					vector<int> tag(NN, 0);

					FEElemData_T<mat3fs, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&state->m_Data[nold]);
					FENodeData<mat3fs>* pnew = dynamic_cast<FENodeData<mat3fs>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							mat3fs v; pold->eval(i, &v);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v;
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] != 0) data[i] /= (float)tag[i];
					}

					for (int i = 0; i < NN; ++i) (*pnew)[i] = data[i];
				}
			}
		}
		else if ((nclass == ELEM_DATA) && (newClass == FACE_DATA))
		{
			if ((nfmt == DATA_ITEM) && (newFormat == DATA_ITEM))
			{
				newField = new FEDataField_T<FEFaceData<mat3fs, DATA_ITEM> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				int NE = mesh.Elements();
				int NF = mesh.Faces();

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					FEElemData_T<mat3fs, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&state->m_Data[nold]);
					FEFaceData<mat3fs, DATA_ITEM>* pnew = dynamic_cast<FEFaceData<mat3fs, DATA_ITEM>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NF; ++i)
					{
						FSFace& face = mesh.Face(i);
						if (face.IsExternal())
						{
							int eid = face.m_elem[0].eid;
							if (pold->active(eid))
							{
								mat3fs v; pold->eval(eid, &v);
								pnew->add(i, v);
							}
						}
					}
				}
			}
		}
	}
	else if (ntype == DATA_MAT3)
	{
		if ((nclass == ELEM_DATA) && (newClass == NODE_DATA))
		{
			int NN = mesh.Nodes();
			int NE = mesh.Elements();

			if (nfmt == DATA_ITEM)
			{
				newField = new FEDataField_T<FENodeData<mat3f> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					FEState* state = fem.GetState(n);

					vector<mat3f> data(NN);
					vector<int> tag(NN, 0);

					FEElemData_T<mat3f, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<mat3f, DATA_ITEM>*>(&state->m_Data[nold]);
					FENodeData<mat3f>* pnew = dynamic_cast<FENodeData<mat3f>*>(&state->m_Data[nnew]);

					for (int i = 0; i < NE; ++i)
					{
						if (pold->active(i))
						{
							FSElement& el = mesh.Element(i);
							mat3f v; pold->eval(i, &v);
							int ne = el.Nodes();
							for (int j = 0; j < ne; ++j)
							{
								data[el.m_node[j]] += v;
								tag[el.m_node[j]]++;
							}
						}
					}

					for (int i = 0; i < NN; ++i)
					{
						if (tag[i] != 0) data[i] /= (float)tag[i];
					}

					for (int i = 0; i < NN; ++i) (*pnew)[i] = data[i];
				}
			}
		}
	}
	return newField;
}

ModelDataField* Post::DataEigenTensor(FEPostModel& fem, ModelDataField* dataField, const std::string& name)
{
	int dataType = dataField->Type();
	int nfmt = dataField->Format();
	int nclass = dataField->DataClass();

	if (dataType != DATA_MAT3S) return nullptr;
	if (nclass != ELEM_DATA) return nullptr;
	if (nfmt != DATA_ITEM) return nullptr;

	ModelDataField* newField = new FEDataField_T<FEElementData<mat3f, DATA_ITEM> >(&fem, EXPORT_DATA);
	fem.AddDataField(newField, name);

	int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
	int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

	FSMesh& mesh = *fem.GetFEMesh(0);
	int NE = mesh.Elements();

	for (int n = 0; n < fem.GetStates(); ++n)
	{
		FEState* state = fem.GetState(n);

		FEElemData_T<mat3fs, DATA_ITEM>* pold = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&state->m_Data[nold]);
		Post::FEElementData<mat3f, DATA_ITEM>* pnew = dynamic_cast<FEElementData<mat3f, DATA_ITEM>*>(&state->m_Data[nnew]);

		for (int i = 0; i < NE; ++i)
		{
			if (pold->active(i))
			{
				mat3fs m;
				pold->eval(i, &m);

				vec3f e[3]; float l[3];
				m.eigen(e, l);

				mat3f a;
				a[0][0] = e[0].x; a[0][1] = e[1].x; a[0][2] = e[2].x;
				a[1][0] = e[0].y; a[1][1] = e[1].y; a[1][2] = e[2].y;
				a[2][0] = e[0].z; a[2][1] = e[1].z; a[2][2] = e[2].z;

				pnew->add(i, a);
			}
		}
	}

	return newField;
}

ModelDataField* Post::DataTimeRate(FEPostModel& fem, ModelDataField* dataField, const std::string& name)
{
	if (dataField == nullptr) return nullptr;

	int nclass = dataField->DataClass();
	DATA_TYPE ntype = dataField->Type();
	int nfmt = dataField->Format();

	FSMesh& mesh = *fem.GetFEMesh(0);

	ModelDataField* newField = 0;
	if (nclass == NODE_DATA)
	{
		if (ntype == DATA_SCALAR)
		{
			newField = new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA);
			fem.AddDataField(newField, name);

			int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
			int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

			for (int n = 0; n < fem.GetStates(); ++n)
			{
				Post::FENodeData<float>& vt = dynamic_cast<FENodeData<float>&>(fem.GetState(n)->m_Data[nnew]);
				if (n == 0)
				{
					for (int i = 0; i < mesh.Nodes(); ++i)
					{
						vt[i] = 0.f;
					}
				}
				else
				{
					FEState* state0 = fem.GetState(n - 1);
					FEState* state1 = fem.GetState(n);

					double dt = state1->m_time - state0->m_time;

					Post::FENodeData_T<float>& d0 = dynamic_cast<FENodeData_T<float>&>(state0->m_Data[nold]);
					Post::FENodeData_T<float>& d1 = dynamic_cast<FENodeData_T<float>&>(state1->m_Data[nold]);


					for (int i = 0; i < mesh.Nodes(); ++i)
					{
						float v0, v1;
						d0.eval(i, &v0);
						d1.eval(i, &v1);

						float dvdt = (v1 - v0) / dt;

						vt[i] = dvdt;
					}
				}
			}
		}
		else if (ntype == DATA_VEC3)
		{
			newField = new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA);
			fem.AddDataField(newField, name);

			int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
			int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

			for (int n = 0; n < fem.GetStates(); ++n)
			{
				Post::FENodeData<vec3f>& vt = dynamic_cast<FENodeData<vec3f>&>(fem.GetState(n)->m_Data[nnew]);
				if (n == 0)
				{
					for (int i = 0; i < mesh.Nodes(); ++i)
					{
						vt[i] = vec3f(0.f, 0.f, 0.f);
					}
				}
				else
				{
					FEState* state0 = fem.GetState(n - 1);
					FEState* state1 = fem.GetState(n    );

					double dt = state1->m_time - state0->m_time;

					Post::FENodeData_T<vec3f>& d0 = dynamic_cast<FENodeData_T<vec3f>&>(state0->m_Data[nold]);
					Post::FENodeData_T<vec3f>& d1 = dynamic_cast<FENodeData_T<vec3f>&>(state1->m_Data[nold]);


					for (int i = 0; i < mesh.Nodes(); ++i)
					{
						vec3f v0, v1;
						d0.eval(i, &v0);
						d1.eval(i, &v1);

						vec3f dvdt = (v1 - v0) / dt;

						vt[i] = dvdt;
					}
				}
			}
		}
	}
	else if (nclass == ELEM_DATA)
	{
		if (ntype == DATA_VEC3)
		{
			if (nfmt == DATA_REGION)
			{
				newField = new FEDataField_T< FEElementData<vec3f, DATA_REGION> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, name);

				int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
				int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

				for (int n = 0; n < fem.GetStates(); ++n)
				{
					Post::FEElementData<vec3f, DATA_REGION>& vt = dynamic_cast<FEElementData<vec3f, DATA_REGION>&>(fem.GetState(n)->m_Data[nnew]);

					int n0 = n, n1 = n;
					if (fem.GetStates() > 1)
					{
						if (n == 0) { n1 = n + 1; }
						else n0 = n - 1;
					}
					
					FEState* state0 = fem.GetState(n0);
					FEState* state1 = fem.GetState(n1);

					double dt = state1->m_time - state0->m_time;
					if (dt == 0) dt = 1.f;

					Post::FEElementData<vec3f, DATA_REGION>& d0 = dynamic_cast<FEElementData<vec3f, DATA_REGION>&>(state0->m_Data[nold]);
					Post::FEElementData<vec3f, DATA_REGION>& d1 = dynamic_cast<FEElementData<vec3f, DATA_REGION>&>(state1->m_Data[nold]);

					vt.copy(d0);
					int NE = vt.size();
					for (int i = 0; i < NE; ++i)
					{
						vt[i] = (d1[i] - d0[i]) / dt;
					}
				}
			}
		}
	}

	return newField;
}

ModelDataField* Post::SurfaceNormalProjection(FEPostModel& fem, ModelDataField* dataField, const std::string& name)
{
	if (dataField == nullptr) return nullptr;

	DATA_CLASS nclass = dataField->DataClass();
	DATA_TYPE ntype = dataField->Type();
	DATA_FORMAT nfmt = dataField->Format();
	int nsrc = dataField->GetFieldID(); nsrc = FIELD_CODE(nsrc);

	FSMesh& mesh = *fem.GetFEMesh(0);

	std::string newname = name;
	if (newname.empty()) newname = "Normal projection of " + dataField->GetName();

	ModelDataField* newField = nullptr;
	if (nclass == ELEM_DATA)
	{
		if (nfmt == DATA_ITEM)
		{
			if (ntype == DATA_MAT3S)
			{
				newField = new FEDataField_T<FEFaceData<float, DATA_ITEM> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, newname);

				int ndst = newField->GetFieldID(); ndst = FIELD_CODE(ndst);

				for (int nstep = 0; nstep < fem.GetStates(); ++nstep)
				{
					Post::FEMeshData& ms = fem.GetState(nstep)->m_Data[nsrc];
					Post::FEMeshData& md = fem.GetState(nstep)->m_Data[ndst];

					Post::FEElemData_T<mat3fs, DATA_ITEM>& src = dynamic_cast<Post::FEElemData_T<mat3fs, DATA_ITEM>&>(ms);
					Post::FEFaceData<float, DATA_ITEM>& dst = dynamic_cast<Post::FEFaceData<float, DATA_ITEM>&>(md);

					int NF = mesh.Faces();
					for (int i = 0; i < NF; ++i)
					{
						FSFace& face = mesh.Face(i);
						face.m_ntag = -1;
						if (face.IsExternal())
						{
							int eid = face.m_elem[0].eid;
							if (src.active(eid))
							{
								mat3fs m;
								src.eval(eid, &m);

								vec3f n = to_vec3f(mesh.FaceNormal(face));
								float v = n * (m * n);

								dst.add(i, v);
							}
						}
					}
				}
			}
		}
		else if (nfmt == DATA_NODE)
		{
			if (ntype == DATA_MAT3S)
			{
				newField = new FEDataField_T<FEFaceData<float, DATA_NODE> >(&fem, EXPORT_DATA);
				fem.AddDataField(newField, newname);

				int ndst = newField->GetFieldID(); ndst = FIELD_CODE(ndst);

				for (int nstep = 0; nstep < fem.GetStates(); ++nstep)
				{
					Post::FEMeshData& ms = fem.GetState(nstep)->m_Data[nsrc];
					Post::FEMeshData& md = fem.GetState(nstep)->m_Data[ndst];

					Post::FEElemData_T<mat3fs, DATA_NODE>& src = dynamic_cast<Post::FEElemData_T<mat3fs, DATA_NODE>&>(ms);
					Post::FEFaceData<float, DATA_NODE>& dst = dynamic_cast<Post::FEFaceData<float, DATA_NODE>&>(md);

					int NF = mesh.Faces();

					// set nodal tags to local node number
					int NN = mesh.Nodes();
					for (int i = 0; i < NN; ++i) mesh.Node(i).m_ntag = -1;

					int n = 0;
					for (int i = 0; i < NF; ++i)
					{
						FSFace& f = mesh.Face(i);
						int nf = f.Nodes();
						for (int j = 0; j < nf; ++j)
							if (mesh.Node(f.n[j]).m_ntag == -1) mesh.Node(f.n[j]).m_ntag = n++;
					}

					// create the face list
					vector<int> f(NF);
					for (int i = 0; i < NF; ++i) f[i] = i;

					// create vector that stores the number of nodes for each facet
					vector<int> fn(NF, 0);
					for (int i = 0; i < NF; ++i) fn[i] = mesh.Face(i).Nodes();

					// create the local node index list
					vector<int> l; l.reserve(NF * FSFace::MAX_NODES);
					for (int i = 0; i < NF; ++i)
					{
						FSFace& f = mesh.Face(i);
						int nn = f.Nodes();
						for (int j = 0; j < nn; ++j)
						{
							int n = mesh.Node(f.n[j]).m_ntag; assert(n >= 0);
							l.push_back(n);
						}
					}

					// create the value array
					std::vector<float> values(n, 0.f);
					std::vector<int> tag(n, 0);

					for (int i = 0; i < NF; ++i)
					{
						FSFace& face = mesh.Face(i);
						if (face.IsExternal())
						{
							int eid = face.m_elem[0].eid;
							FSElement& el = mesh.Element(eid);
							if (src.active(eid))
							{
								mat3fs v[FSElement::MAX_NODES];
								src.eval(eid, v);

								vec3f N = to_vec3f(mesh.FaceNormal(face));

								float fv[FSFace::MAX_NODES] = { 0.f };
								for (int m = 0; m < face.Nodes(); ++m)
								{
									int a = face.n[m];
									int b = el.FindNodeIndex(a);
									mat3fs S = v[b];
									float v = N * (S * N);

									int al = mesh.Node(a).m_ntag;  assert(al >= 0);

									values[al] += v;
									tag[al]++;
								}
							}
						}
					}

					for (int i = 0; i < n; ++i)
					{
						if (tag[i] != 0) values[i] /= (float)tag[i];
					}

					dst.add(values, f, l, fn);
				}
			}
		}
	}

	return newField;
}
