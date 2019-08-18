#include "stdafx.h"
#include "DataFilter.h"
#include "FEModel.h"
#include "constants.h"
#include "FEMeshData_T.h"
#include "evaluate.h"
using namespace Post;

bool Post::DataScale(FEModel& fem, int nfield, double scale)
{
	FEMeshBase& mesh = *fem.GetFEMesh(0);
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
				FENodeData<Mat3d>* pv = dynamic_cast< FENodeData<Mat3d>* >(&d);
				for (int n = 0; n<NN; ++n) { Mat3d& v = (*pv)[n]; v *= fscale; }
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
					FEElementData<Mat3d, DATA_NODE>* pf = dynamic_cast<FEElementData<Mat3d, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEElementData<Mat3d, DATA_ITEM>* pf = dynamic_cast<FEElementData<Mat3d, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_COMP)
				{
					FEElementData<Mat3d, DATA_COMP>* pf = dynamic_cast<FEElementData<Mat3d, DATA_COMP>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= fscale;
				}
				else if (fmt == DATA_REGION)
				{
					FEElementData<Mat3d, DATA_REGION>* pf = dynamic_cast<FEElementData<Mat3d, DATA_REGION>*>(&d);
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
					FEFaceData<Mat3d, DATA_NODE>* pf = dynamic_cast<FEFaceData<Mat3d, DATA_NODE>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData<Mat3d, DATA_ITEM>* pf = dynamic_cast<FEFaceData<Mat3d, DATA_ITEM>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData<Mat3d, DATA_COMP>* pf = dynamic_cast<FEFaceData<Mat3d, DATA_COMP>*>(&d);
					int N = pf->size();
					for (int n = 0; n<N; ++n) (*pf)[n] *= scale;
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData<Mat3d, DATA_REGION>* pf = dynamic_cast<FEFaceData<Mat3d, DATA_REGION>*>(&d);
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
// Apply a smoothing step operation on data
bool DataSmoothStep(FEModel& fem, int nfield, double theta)
{
	// loop over all states
	int ndata = FIELD_CODE(nfield);
	for (int n = 0; n<fem.GetStates(); ++n)
	{
		FEState& s = *fem.GetState(n);
		FEMeshBase& mesh = *s.GetFEMesh();
		if (IS_NODE_FIELD(nfield))
		{
			int NN = mesh.Nodes();
			FEMeshData& d = s.m_Data[ndata];
			
			switch (d.GetType())
			{
			case DATA_FLOAT:
			{
				vector<float> D; D.assign(NN, 0.f);
				vector<int> tag; tag.assign(NN, 0);
				FENodeData<float>& data = dynamic_cast< FENodeData<float>& >(d);

				// evaluate the average value of the neighbors
				int NE = mesh.Elements();
				for (int i=0; i<NE; ++i)
				{
					FEElement& el = mesh.Element(i);
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
				FENodeData<vec3f>& data = dynamic_cast< FENodeData<vec3f>& >(d);

				// evaluate the average value of the neighbors
				int NE = mesh.Elements();
				for (int i = 0; i<NE; ++i)
				{
					FEElement& el = mesh.Element(i);
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
			FEMeshData& d = s.m_Data[ndata];
			if ((d.GetFormat() == DATA_ITEM)&&(d.GetType() == DATA_FLOAT))
			{
				int NE = mesh.Elements();

				vector<float> D; D.assign(NE, 0.f);
				vector<int> tag; tag.assign(NE, 0);
				FEElementData<float, DATA_ITEM>& data = dynamic_cast< FEElementData<float, DATA_ITEM>& >(d);

				for (int i=0; i<NE; ++i) mesh.Element(i).m_ntag = i;

				// evaluate the average value of the neighbors
				for (int i=0; i<NE; ++i)
				{
					FEElement& el = mesh.Element(i);
					int nf = el.Faces();
					for (int j=0; j<nf; ++j)
					{
						FEElement* pj = el.m_pElem[j];
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
bool Post::DataSmooth(FEModel& fem, int nfield, double theta, int niters)
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
bool Post::DataArithmetic(FEModel& fem, int nfield, int nop, int noperand)
{
	int ndst = FIELD_CODE(nfield);
	int nsrc = FIELD_CODE(noperand);

	FEMeshBase& mesh = *fem.GetFEMesh(0);

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
bool Post::DataGradient(FEModel& fem, int vecField, int sclField)
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
		FEMeshBase* mesh = state.GetFEMesh();

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

					float ed[FEGenericElement::MAX_NODES] = {0.f};
					for (int i=0; i<mesh->Elements(); ++i)
					{
						FEElement& el = mesh->Element(i);
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
						FEElement& el = mesh->Element(i);
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

					float ed[FEGenericElement::MAX_NODES] = { 0.f };
					for (int i = 0; i<mesh->Elements(); ++i)
					{
						FEElement& el = mesh->Element(i);
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
		vec3f eg[FEGenericElement::MAX_NODES];
		float ed[FEGenericElement::MAX_NODES];
		vector<int> tag(NN, 0);
		for (int i=0; i<mesh->Elements(); ++i)
		{
			FEElement& el = mesh->Element(i);

			for (int j = 0; j<el.Nodes(); ++j) ed[j] = d[el.m_node[j]];

			for (int j=0; j<el.Nodes(); ++j)
			{
				// get the iso-coords at the nodes
				double q[3] = {0,0,0};
				el.iso_coord(j, q);

				// evaluate the gradient at the node
				shape_grad(fem, i, q, n, eg);

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
template <typename T> void extractNodeDataComponent_T(FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	FENodeData_T<T>& vec = dynamic_cast<FENodeData_T<T>&>(src);
	FENodeData<float>& scl = dynamic_cast<FENodeData<float>&>(dst);

	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		T v; vec.eval(i, &v);
		scl[i] = component(v, ncomp);
	}
}

void extractNodeDataComponent(Data_Type ntype, FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	switch (ntype)
	{
	case DATA_VEC3F  : extractNodeDataComponent_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractNodeDataComponent_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractNodeDataComponent_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractNodeDataComponent_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractNodeDataComponent_T<Mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractNodeDataComponent_T<mat3f  >(dst, src, ncomp, mesh); break;
	}
}

template <typename T> void extractElemDataComponentITEM_T(FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	FEElemData_T<T, DATA_ITEM>& vec = dynamic_cast<FEElemData_T<T, DATA_ITEM>&>(src);
	FEElementData<float, DATA_ITEM>& scl = dynamic_cast<FEElementData<float, DATA_ITEM>&>(dst);

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

void extractElemDataComponentITEM_ARRAY_VEC3F(FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	FEElemArrayVec3Data& vec = dynamic_cast<FEElemArrayVec3Data&>(src);
	FEElementData<float, DATA_ITEM>& scl = dynamic_cast<FEElementData<float, DATA_ITEM>&>(dst);

	int index = ncomp / 4;
	int veccomp = ncomp % 4;

	int NE = mesh.Elements();
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		int ne = el.Nodes();
		if (vec.active(i))
		{
			vec3f v = vec.eval(i, index);
			float f = component2(v, veccomp);
			scl.add(i, f);
		}
	}
}

void extractElemDataComponentITEM(Data_Type ntype, FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3F  : extractElemDataComponentITEM_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractElemDataComponentITEM_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractElemDataComponentITEM_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractElemDataComponentITEM_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractElemDataComponentITEM_T<Mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractElemDataComponentITEM_T<mat3f  >(dst, src, ncomp, mesh); break;
	case DATA_ARRAY_VEC3F: extractElemDataComponentITEM_ARRAY_VEC3F(dst, src, ncomp, mesh); break;
	}
}

template <typename T> void extractElemDataComponentNODE_T(FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	FEElemData_T<T, DATA_NODE>& vec = dynamic_cast<FEElemData_T<T, DATA_NODE>&>(src);
	FEElementData<float, DATA_NODE>& scl = dynamic_cast<FEElementData<float, DATA_NODE>&>(dst);

	int NE = mesh.Elements();
	T val[FEGenericElement::MAX_NODES];
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
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

void extractElemDataComponentNODE_ARRAY(FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	FEElemArrayDataNode& vec = dynamic_cast<FEElemArrayDataNode&>(src);
	FEElementData<float, DATA_NODE>& scl = dynamic_cast<FEElementData<float, DATA_NODE>&>(dst);

	int NE = mesh.Elements();
	float val[FEGenericElement::MAX_NODES];
	vector<float> data;
	vector<int> elem(1);
	vector<int> l;
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
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

void extractElemDataComponentNODE(Data_Type ntype, FEMeshData& dst, FEMeshData& src, int ncomp, FEMeshBase& mesh)
{
	switch(ntype)
	{
	case DATA_VEC3F  : extractElemDataComponentNODE_T<vec3f  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FS : extractElemDataComponentNODE_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_MAT3FD : extractElemDataComponentNODE_T<mat3fs >(dst, src, ncomp, mesh); break;
	case DATA_TENS4FS: extractElemDataComponentNODE_T<tens4fs>(dst, src, ncomp, mesh); break;
	case DATA_MAT3D  : extractElemDataComponentNODE_T<Mat3d  >(dst, src, ncomp, mesh); break;
	case DATA_MAT3F  : extractElemDataComponentNODE_T<mat3f  >(dst, src, ncomp, mesh); break;
	case DATA_ARRAY  : extractElemDataComponentNODE_ARRAY(dst, src, ncomp, mesh); break;
	}
}

FEDataField* Post::DataComponent(FEModel& fem, FEDataField* pdf, int ncomp, const std::string& sname)
{
	if (pdf == 0) return 0;

	int nclass = pdf->DataClass();
	Data_Type ntype = pdf->Type();
	int nfmt = pdf->Format();

	FEMeshBase& mesh = *fem.GetFEMesh(0);

	FEDataField* newField = 0;
	if (nclass == CLASS_NODE)
	{
		newField = new FEDataField_T<FENodeData<float> >(sname);
		fem.AddDataField(newField);

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
			newField = new FEDataField_T<FEElementData<float, DATA_ITEM> >(sname);
			fem.AddDataField(newField);

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
			newField = new FEDataField_T<FEElementData<float, DATA_NODE> >(sname);
			fem.AddDataField(newField);

			int nvec = pdf->GetFieldID(); nvec = FIELD_CODE(nvec);
			int nscl = newField->GetFieldID(); nscl = FIELD_CODE(nscl);

			for (int n = 0; n<fem.GetStates(); ++n)
			{
				FEState* state = fem.GetState(n);
				extractElemDataComponentNODE(ntype, state->m_Data[nscl], state->m_Data[nvec], ncomp, mesh);
			}
		}
	}

	return newField;
}
