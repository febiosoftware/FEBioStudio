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
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	float fscale = (float) scale;
	// loop over all states
	int NN = mesh.Nodes();
	int ndata = FIELD_CODE(nfield);
	for (int i = 0; i<fem.GetStates(); ++i)
	{
		FEState& s = *fem.GetState(i);
		FEMeshData& d = s.m_Data[ndata];
		Data_Type type = d.GetType();
		Data_Format fmt = d.GetFormat();
		if (IS_NODE_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_FLOAT:
			{
				FENodeData<float>* pf = dynamic_cast< FENodeData<float>* >(&d);
				for (int n = 0; n<NN; ++n) { float& v = (*pf)[n]; v *= fscale; }
			}
			break;
			case DATA_VEC3F:
			{
				FENodeData<vec3f>* pv = dynamic_cast< FENodeData<vec3f>* >(&d);
				for (int n = 0; n<NN; ++n) { vec3f& v = (*pv)[n]; v *= fscale; }
			}
			break;
			case DATA_MAT3FS:
			{
				FENodeData<mat3fs>* pv = dynamic_cast< FENodeData<mat3fs>* >(&d);
				for (int n = 0; n<NN; ++n) { mat3fs& v = (*pv)[n]; v *= fscale; }
			}
			break;
			case DATA_MAT3D:
			{
				FENodeData<mat3d>* pv = dynamic_cast< FENodeData<mat3d>* >(&d);
				for (int n = 0; n<NN; ++n) { mat3d& v = (*pv)[n]; v *= fscale; }
			}
			break;
			case DATA_MAT3F:
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
			case DATA_FLOAT:
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
				else if (fmt == DATA_COMP)
				{
					FEElementData<float, DATA_COMP>* pf = dynamic_cast<FEElementData<float, DATA_COMP>*>(&d);
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
			case DATA_VEC3F:
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
				else if (fmt == DATA_COMP)
				{
					FEElementData<vec3f, DATA_COMP>* pf = dynamic_cast<FEElementData<vec3f, DATA_COMP>*>(&d);
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
			case DATA_MAT3FS:
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
				else if (fmt == DATA_COMP)
				{
					FEElementData<mat3fs, DATA_COMP>* pf = dynamic_cast<FEElementData<mat3fs, DATA_COMP>*>(&d);
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
			case DATA_MAT3D:
			{
				if (fmt == DATA_NODE)
				{
					FEElementData<mat3d, DATA_NODE>* pf = dynamic_cast<FEElementData<mat3d, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<mat3d, DATA_ITEM>* pf = dynamic_cast<FEElementData<mat3d, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_COMP)
				{
					FEElementData<mat3d, DATA_COMP>* pf = dynamic_cast<FEElementData<mat3d, DATA_COMP>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<mat3d, DATA_REGION>* pf = dynamic_cast<FEElementData<mat3d, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
			}
			break;
			case DATA_MAT3F:
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
				else if (fmt == DATA_COMP)
				{
					FEElementData<mat3f, DATA_COMP>* pf = dynamic_cast<FEElementData<mat3f, DATA_COMP>*>(&d);
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
			case DATA_FLOAT:
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
				else if (fmt == DATA_COMP)
				{
					FEFaceData<float, DATA_COMP>* pf = dynamic_cast<FEFaceData<float, DATA_COMP>*>(&d);
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
			case DATA_VEC3F:
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
				else if (fmt == DATA_COMP)
				{
					FEFaceData<vec3f, DATA_COMP>* pf = dynamic_cast<FEFaceData<vec3f, DATA_COMP>*>(&d);
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
			case DATA_MAT3FS:
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
				else if (fmt == DATA_COMP)
				{
					FEFaceData<mat3fs, DATA_COMP>* pf = dynamic_cast<FEFaceData<mat3fs, DATA_COMP>*>(&d);
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
			case DATA_MAT3D:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData<mat3d, DATA_NODE>* pf = dynamic_cast<FEFaceData<mat3d, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<mat3d, DATA_ITEM>* pf = dynamic_cast<FEFaceData<mat3d, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData<mat3d, DATA_COMP>* pf = dynamic_cast<FEFaceData<mat3d, DATA_COMP>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<mat3d, DATA_REGION>* pf = dynamic_cast<FEFaceData<mat3d, DATA_REGION>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
			}
			break;
			case DATA_MAT3F:
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
				else if (fmt == DATA_COMP)
				{
					FEFaceData<mat3f, DATA_COMP>* pf = dynamic_cast<FEFaceData<mat3f, DATA_COMP>*>(&d);
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
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	vec3f fscale = to_vec3f(scale);

	// loop over all states
	int NN = mesh.Nodes();
	int ndata = FIELD_CODE(nfield);
	for (int i = 0; i < fem.GetStates(); ++i)
	{
		FEState& s = *fem.GetState(i);
		FEMeshData& d = s.m_Data[ndata];
		Data_Type type = d.GetType();
		Data_Format fmt = d.GetFormat();
		if (IS_NODE_FIELD(nfield))
		{
			switch (type)
			{
			case DATA_VEC3F:
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
			case DATA_VEC3F:
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
				else if (fmt == DATA_COMP)
				{
					FEElementData<vec3f, DATA_COMP>* pf = dynamic_cast<FEElementData<vec3f, DATA_COMP>*>(&d);
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
			case DATA_VEC3F:
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
				else if (fmt == DATA_COMP)
				{
					FEFaceData<vec3f, DATA_COMP>* pf = dynamic_cast<FEFaceData<vec3f, DATA_COMP>*>(&d);
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
		Post::FEPostMesh& mesh = *s.GetFEMesh();
		if (IS_NODE_FIELD(nfield))
		{
			int NN = mesh.Nodes();
			Post::FEMeshData& d = s.m_Data[ndata];
			
			switch (d.GetType())
			{
			case DATA_FLOAT:
			{
				vector<float> D; D.assign(NN, 0.f);
				vector<int> tag; tag.assign(NN, 0);
				Post::FENodeData<float>& data = dynamic_cast< Post::FENodeData<float>& >(d);

				// evaluate the average value of the neighbors
				int NE = mesh.Elements();
				for (int i=0; i<NE; ++i)
				{
					FEElement_& el = mesh.ElementRef(i);
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
				for (int i = 0; i<NN; ++i) { data[i] = (1.0 - theta)*data[i] + theta*D[i];  }
			}
			break;
			case DATA_VEC3F:
			{
				vector<vec3f> D; D.assign(NN, vec3f(0.f, 0.f, 0.f));
				vector<int> tag; tag.assign(NN, 0);
				Post::FENodeData<vec3f>& data = dynamic_cast< Post::FENodeData<vec3f>& >(d);

				// evaluate the average value of the neighbors
				int NE = mesh.Elements();
				for (int i = 0; i<NE; ++i)
				{
					FEElement_& el = mesh.ElementRef(i);
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
			if ((d.GetFormat() == DATA_ITEM)&&(d.GetType() == DATA_FLOAT))
			{
				int NE = mesh.Elements();

				vector<float> D; D.assign(NE, 0.f);
				vector<int> tag; tag.assign(NE, 0);
				Post::FEElementData<float, DATA_ITEM>& data = dynamic_cast< Post::FEElementData<float, DATA_ITEM>& >(d);

				for (int i=0; i<NE; ++i) mesh.ElementRef(i).m_ntag = i;

				// evaluate the average value of the neighbors
				for (int i=0; i<NE; ++i)
				{
					FEElement_& el = mesh.ElementRef(i);
					int nf = el.Faces();
					for (int j=0; j<nf; ++j)
					{
						FEElement_* pj = mesh.ElementPtr(el.m_nbr[j]);
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
						D[i] = (1.0 - theta)*f + theta*D[i];
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

	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	// loop over all states
	for (int n = 0; n<fem.GetStates(); ++n)
	{
		FEState& state = *fem.GetState(n);
		FEMeshData& d = state.m_Data[ndst];
		FEMeshData& s = state.m_Data[nsrc];

		Data_Format fmt = d.GetFormat();
		if (d.GetFormat() != s.GetFormat()) return false;
		if ((d.GetType() != s.GetType()) && (s.GetType() != DATA_FLOAT)) return false;

		if (IS_NODE_FIELD(nfield) && IS_NODE_FIELD(noperand))
		{
			if ((d.GetType() == DATA_FLOAT) && (s.GetType() == DATA_FLOAT))
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
			else if (d.GetType() == DATA_VEC3F)
			{
				if (s.GetType() == DATA_VEC3F)
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
				else if (s.GetType() == DATA_FLOAT)
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
			if ((d.GetType() == DATA_FLOAT) && (s.GetType() == DATA_FLOAT))
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
				else
				{
					return false;
				}
			}
			else if (d.GetType() == DATA_MAT3FS)
			{
				if (s.GetType() == DATA_MAT3FS)
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
				else if (s.GetType() == DATA_FLOAT)
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
		if (IS_NODE_FIELD(vecField) && (v.GetType() == DATA_VEC3F))
		{
			FENodeData<vec3f>* pv = dynamic_cast<FENodeData<vec3f>*>(&v);
			int N = pv->size();
			for (int i = 0; i<N; ++i) (*pv)[i] = vec3f(0,0,0);
		}
		else return false;

		// get the mesh
		Post::FEPostMesh* mesh = state.GetFEMesh();

		// evaluate the field over all the nodes
		const int NN = mesh->Nodes();
		vector<double> d(NN, 0.f);

		if (s.GetType() == DATA_FLOAT)
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
						FEElement_& el = mesh->ElementRef(i);
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
						FEElement_& el = mesh->ElementRef(i);
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
				else if (s.GetFormat() == DATA_COMP)
				{
					vector<int> tag(NN, 0);
					FEElemData_T<float, DATA_COMP>* ps = dynamic_cast<FEElemData_T<float, DATA_COMP>*>(&s);

					float ed[FSElement::MAX_NODES] = { 0.f };
					for (int i = 0; i<mesh->Elements(); ++i)
					{
						FEElement_& el = mesh->ElementRef(i);
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
			FEElement_& el = mesh->ElementRef(i);

			for (int j = 0; j<el.Nodes(); ++j) ed[j] = d[el.m_node[j]];

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
template <typename T> void extractNodeDataComponent_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
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

void extractNodeDataComponent(Data_Type ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	switch (ntype)
	{
	case DATA_VEC3F  : extractNodeDataComponent_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractNodeDataComponent_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractNodeDataComponent_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractNodeDataComponent_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractNodeDataComponent_T<mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractNodeDataComponent_T<mat3f  >(dst, src, ncomp, mesh); break;
	}
}

template <typename T> void extractElemDataComponentITEM_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
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

void extractElemDataComponentITEM_ARRAY(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	FEElemArrayDataItem& vec = dynamic_cast<FEElemArrayDataItem&>(src);
	Post::FEElementData<float, DATA_ITEM>& scl = dynamic_cast<Post::FEElementData<float, DATA_ITEM>&>(dst);

	int NE = mesh.Elements();
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			float f = vec.eval(i, ncomp);
			scl.add(i, f);
		}
	}
}

void extractElemDataComponentITEM_ARRAY_VEC3F(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
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
		FEElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			vec3f v = vec.eval(i, index);
			float f = component2(v, veccomp);
			scl.add(i, f);
		}
	}
}

void extractElemDataComponentITEM(Data_Type ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3F  : extractElemDataComponentITEM_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractElemDataComponentITEM_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractElemDataComponentITEM_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractElemDataComponentITEM_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractElemDataComponentITEM_T<mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractElemDataComponentITEM_T<mat3f  >(dst, src, ncomp, mesh); break;
	case DATA_ARRAY      : extractElemDataComponentITEM_ARRAY(dst, src, ncomp, mesh); break;
	case DATA_ARRAY_VEC3F: extractElemDataComponentITEM_ARRAY_VEC3F(dst, src, ncomp, mesh); break;
	}
}

template <typename T> void extractElemDataComponentNODE_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	FEElemData_T<T, DATA_NODE>& vec = dynamic_cast<FEElemData_T<T, DATA_NODE>&>(src);
	Post::FEElementData<float, DATA_NODE>& scl = dynamic_cast<Post::FEElementData<float, DATA_NODE>&>(dst);

	int NE = mesh.Elements();
	T val[FSElement::MAX_NODES];
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			vec.eval(i, val);

			data.resize(ne);
			for (int j=0; j<ne; ++j) data[j] = component(val[j], ncomp);

			l.resize(ne);
			for (int j=0; j<ne; ++j) l[j] = el.m_node[j];

			elem[0] = i;
			
			scl.add(data, elem, l, ne);
		}
	}
}

void extractElemDataComponentNODE_ARRAY(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
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
		FEElement_& el = mesh.ElementRef(i);
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

void extractElemDataComponentNODE(Data_Type ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3F  : extractElemDataComponentNODE_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractElemDataComponentNODE_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractElemDataComponentNODE_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractElemDataComponentNODE_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractElemDataComponentNODE_T<mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractElemDataComponentNODE_T<mat3f  >(dst, src, ncomp, mesh); break;
	case DATA_ARRAY  : extractElemDataComponentNODE_ARRAY(dst, src, ncomp, mesh); break;
	}
}


template <typename T> void extractElemDataComponentCOMP_T(Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	FEElemData_T<T, DATA_COMP>& vec = dynamic_cast<FEElemData_T<T, DATA_COMP>&>(src);
	Post::FEElementData<float, DATA_COMP>& scl = dynamic_cast<Post::FEElementData<float, DATA_COMP>&>(dst);

	int NE = mesh.Elements();
	T val[FSElement::MAX_NODES];
	vector<float> data;
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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

void extractElemDataComponentCOMP(Data_Type ntype, Post::FEMeshData& dst, Post::FEMeshData& src, int ncomp, Post::FEPostMesh& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3F  : extractElemDataComponentNODE_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractElemDataComponentCOMP_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractElemDataComponentNODE_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractElemDataComponentNODE_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractElemDataComponentNODE_T<mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractElemDataComponentNODE_T<mat3f  >(dst, src, ncomp, mesh); break;
//	case DATA_ARRAY  : extractElemDataComponentNODE_ARRAY(dst, src, ncomp, mesh); break;
	}
}

ModelDataField* Post::DataComponent(FEPostModel& fem, ModelDataField* pdf, int ncomp, const std::string& sname)
{
	if (pdf == 0) return 0;

	int nclass = pdf->DataClass();
	Data_Type ntype = pdf->Type();
	int nfmt = pdf->Format();

	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	ModelDataField* newField = 0;
	if (nclass == CLASS_NODE)
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
	else if (nclass == CLASS_FACE)
	{
		
	}
	else if (nclass == CLASS_ELEM)
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
		else if (nfmt == DATA_COMP)
		{
			newField = new FEDataField_T<FEElementData<float, DATA_COMP> >(&fem, EXPORT_DATA);
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
		if (IS_ELEM_FIELD(scalarField) && (s.GetType() == DATA_FLOAT))
		{
			ps = dynamic_cast<Post::FEElementData<float, DATA_ITEM>*>(&s);
			int N = ps->size();
			for (int i = 0; i < N; ++i) (*ps)[i] = 0.f;
		}
		else return false;

		// get the mesh
		Post::FEPostMesh* mesh = state.GetFEMesh();

		// evaluate the field
		if (v.GetType() == DATA_MAT3FS)
		{
			if (IS_ELEM_FIELD(tensorField) && (v.GetFormat() == DATA_ITEM))
			{
				FEElemData_T<mat3fs, DATA_ITEM>* pv = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&v);

				mat3fs ev;
				for (int i = 0; i<mesh->Elements(); ++i)
				{
					FEElement_& el = mesh->ElementRef(i);
					if (pv->active(i))
					{
						pv->eval(i, &ev);

						ps->add(i, fractional_anisotropy(ev));
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
	Data_Type ntype = dataField->Type();
	int nfmt = dataField->Format();

	if (newFormat == nfmt) return nullptr;

	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	ModelDataField* newField = nullptr;
	if (ntype == DATA_FLOAT)
	{
		if ((nclass == CLASS_ELEM) && (newClass == CLASS_ELEM))
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
			else if ((nfmt == DATA_COMP) && (newFormat == DATA_ITEM))
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

					FEElemData_T<float, DATA_COMP>* pold = dynamic_cast<FEElemData_T<float, DATA_COMP>*>(&state->m_Data[nold]);
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
			else if ((nfmt == DATA_COMP) && (newFormat == DATA_NODE))
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

					FEElemData_T<float, DATA_COMP>* pold = dynamic_cast<FEElemData_T<float, DATA_COMP>*>(&state->m_Data[nold]);
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
		}
		else if ((nclass == CLASS_ELEM) && (newClass == CLASS_NODE))
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
			else if (nfmt == DATA_COMP)
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

					FEElemData_T<float, DATA_COMP>* pold = dynamic_cast<FEElemData_T<float, DATA_COMP>*>(&state->m_Data[nold]);
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
	}
	else if (ntype == DATA_VEC3F)
	{
		if ((nclass == CLASS_ELEM) && (newClass == CLASS_NODE))
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
	}
	else if (ntype == DATA_MAT3FS)
	{
		if ((nclass == CLASS_ELEM) && (newClass == CLASS_NODE))
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
	}
	else if (ntype == DATA_MAT3F)
	{
		if ((nclass == CLASS_ELEM) && (newClass == CLASS_NODE))
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

	if (dataType != DATA_MAT3FS) return nullptr;
	if (nclass != CLASS_ELEM) return nullptr;
	if (nfmt != DATA_ITEM) return nullptr;

	ModelDataField* newField = new FEDataField_T<FEElementData<mat3f, DATA_ITEM> >(&fem, EXPORT_DATA);
	fem.AddDataField(newField, name);

	int nold = dataField->GetFieldID(); nold = FIELD_CODE(nold);
	int nnew = newField->GetFieldID(); nnew = FIELD_CODE(nnew);

	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
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
	Data_Type ntype = dataField->Type();
	int nfmt = dataField->Format();

	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	ModelDataField* newField = 0;
	if (nclass == CLASS_NODE)
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
		else if (ntype == DATA_VEC3F)
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

	return newField;
}
