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
#include "FEDataField.h"
#include "FEMeshData_T.h"
#include "constants.h"
#include "FEPostModel.h"
#include "FEDistanceMap.h"
#include "FEAreaCoverage.h"
using namespace Post;

ModelDataField::ModelDataField(FEPostModel* fem, DATA_TYPE ntype, DATA_FORMAT nfmt, DATA_CLASS ncls, unsigned int flag)
{
	m_fem = fem;
	m_ntype = ntype;
	m_nfmt = nfmt;
	m_nclass = ncls;
	m_flag = flag;
	m_arraySize = 0;
}

ModelDataField::~ModelDataField() {}

void ModelDataField::SetArrayNames(vector<string>& n)
{
	m_arrayNames = n;
}

vector<string> ModelDataField::GetArrayNames() const
{
	return m_arrayNames;
}

const char* ModelDataField::TypeStr() const
{
	switch (m_ntype)
	{
	case DATA_SCALAR     : return "float"; break;
	case DATA_VEC3      : return "vec3f"; break;
	case DATA_MAT3      : return "mat3f"; break;
	case DATA_MAT3S     : return "mat3fs"; break;
	case DATA_MAT3SD    : return "mat3fd"; break;
	case DATA_TENS4S    : return "tens4fs"; break;
	case DATA_ARRAY      : return "array<float>"; break;
	case DATA_ARRAY_VEC3: return "array<vec3>"; break;
	};
	return "unknown";
}

int ModelDataField::components(Data_Tensor_Type ntype)
{
	if (ntype == TENSOR_SCALAR)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR     : return 1; break;
		case DATA_VEC3      : return 7; break;
		case DATA_MAT3      : return 10; break;
		case DATA_MAT3S     : return 18; break;
		case DATA_MAT3SD    : return 3; break;
		case DATA_TENS4S    : return 21; break;
		case DATA_ARRAY      : return GetArraySize(); break;
		case DATA_ARRAY_VEC3: return GetArraySize()*4; break;
		}
	}
	else if (ntype == TENSOR_VECTOR)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR     : return 0; break;
		case DATA_VEC3      : return 1; break;
		case DATA_MAT3      : return 3; break;
		case DATA_MAT3S     : return 3; break;
		case DATA_MAT3SD    : return 0; break;
		case DATA_TENS4S    : return 0; break;
		case DATA_ARRAY      : return 0; break;
		case DATA_ARRAY_VEC3: return GetArraySize(); break;
		}
	}
	else if (ntype == TENSOR_TENSOR2)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR     : return 0; break;
		case DATA_VEC3      : return 0; break;
		case DATA_MAT3      : return 1; break;
		case DATA_MAT3S     : return 1; break;
		case DATA_MAT3SD    : return 1; break;
		case DATA_TENS4S    : return 0; break;
		case DATA_ARRAY      : return 0; break;
		case DATA_ARRAY_VEC3: return 0; break;
		}
	}
	return 0;
}

int ModelDataField::dataComponents(Data_Tensor_Type ntype)
{
	if (ntype == TENSOR_SCALAR)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR     : return 1; break;
		case DATA_VEC3      : return 3; break;
		case DATA_MAT3      : return 9; break;
		case DATA_MAT3S     : return 6; break;
		case DATA_MAT3SD    : return 3; break;
		case DATA_TENS4S    : return 21; break;
		case DATA_ARRAY      : return GetArraySize(); break;
		case DATA_ARRAY_VEC3: return GetArraySize() * 3; break;
		}
	}
	else if (ntype == TENSOR_VECTOR)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR     : return 0; break;
		case DATA_VEC3      : return 1; break;
		case DATA_MAT3      : return 3; break;
		case DATA_MAT3S     : return 3; break;
		case DATA_MAT3SD    : return 0; break;
		case DATA_TENS4S    : return 0; break;
		case DATA_ARRAY      : return 0; break;
		case DATA_ARRAY_VEC3: return GetArraySize(); break;
		}
	}
	else if (ntype == TENSOR_TENSOR2)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR     : return 0; break;
		case DATA_VEC3      : return 0; break;
		case DATA_MAT3      : return 1; break;
		case DATA_MAT3S     : return 1; break;
		case DATA_MAT3SD    : return 1; break;
		case DATA_TENS4S    : return 0; break;
		case DATA_ARRAY      : return 0; break;
		case DATA_ARRAY_VEC3: return 0; break;
		}
	}
	return 0;
}

std::string ModelDataField::componentName(int ncomp, Data_Tensor_Type ntype)
{
	const std::string& name = GetName();
	const char* sz = name.c_str();

	char szline[256] = { 0 };

	if (ntype == TENSOR_SCALAR)
	{
		switch (m_ntype)
		{
		case DATA_SCALAR: return name; break;
		case DATA_VEC3:
		{
			if (ncomp == 0) sprintf(szline, "X - %s", sz);
			else if (ncomp == 1) sprintf(szline, "Y - %s", sz);
			else if (ncomp == 2) sprintf(szline, "Z - %s", sz);
			else if (ncomp == 3) sprintf(szline, "XY - %s", sz);
			else if (ncomp == 4) sprintf(szline, "YZ - %s", sz);
			else if (ncomp == 5) sprintf(szline, "XZ - %s", sz);
			else if (ncomp == 6) sprintf(szline, "%s Magnitude", sz);
			return szline;
		}
		break;
		case DATA_MAT3:
		{
			if (ncomp == 0) sprintf(szline, "XX - %s", sz);
			else if (ncomp == 1) sprintf(szline, "XY - %s", sz);
			else if (ncomp == 2) sprintf(szline, "XZ - %s", sz);
			else if (ncomp == 3) sprintf(szline, "YX - %s", sz);
			else if (ncomp == 4) sprintf(szline, "YY - %s", sz);
			else if (ncomp == 5) sprintf(szline, "YZ - %s", sz);
			else if (ncomp == 6) sprintf(szline, "ZX - %s", sz);
			else if (ncomp == 7) sprintf(szline, "ZY - %s", sz);
			else if (ncomp == 8) sprintf(szline, "ZZ - %s", sz);
			else if (ncomp == 9) sprintf(szline, "%s Magnitude", sz);
			return szline;
		}
		break;
		case DATA_MAT3S:
		{
			if      (ncomp == Data_Mat3ds_Component::MAT3DS_XX) sprintf(szline, "X - %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_YY) sprintf(szline, "Y - %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_ZZ) sprintf(szline, "Z - %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_XY) sprintf(szline, "XY - %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_YZ) sprintf(szline, "YZ - %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_XZ) sprintf(szline, "XZ - %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_EFFECTIVE) sprintf(szline, "Effective %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_P1) sprintf(szline, "1 Principal %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_P2) sprintf(szline, "2 Principal %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_P3) sprintf(szline, "3 Principal %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_DEV_P1) sprintf(szline, "1 Dev Principal %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_DEV_P2) sprintf(szline, "2 Dev Principal %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_DEV_P3) sprintf(szline, "3 Dev Principal %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_MAX_SHEAR) sprintf(szline, "Max Shear %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_MAGNITUDE) sprintf(szline, "%s Magnitude", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_I1) sprintf(szline, "1 Invariant of %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_I2) sprintf(szline, "2 Invariant of %s", sz);
			else if (ncomp == Data_Mat3ds_Component::MAT3DS_I3) sprintf(szline, "3 Invariant of %s", sz);
			return szline;
		}
		break;
		case DATA_MAT3SD:
		{
			if (ncomp == 0) sprintf(szline, "1 - %s", sz);
			else if (ncomp == 1) sprintf(szline, "2 - %s", sz);
			else if (ncomp == 2) sprintf(szline, "3 - %s", sz);
			return szline;
		}
		break;
		case DATA_TENS4S:
		{
			if (ncomp == 0) sprintf(szline, "XXXX - %s", sz);
			else if (ncomp == 1) sprintf(szline, "XXYY - %s", sz);
			else if (ncomp == 2) sprintf(szline, "YYYY - %s", sz);
			else if (ncomp == 3) sprintf(szline, "XXZZ - %s", sz);
			else if (ncomp == 4) sprintf(szline, "YYZZ - %s", sz);
			else if (ncomp == 5) sprintf(szline, "ZZZZ - %s", sz);
			else if (ncomp == 6) sprintf(szline, "XXXY - %s", sz);
			else if (ncomp == 7) sprintf(szline, "YYXY - %s", sz);
			else if (ncomp == 8) sprintf(szline, "ZZXY - %s", sz);
			else if (ncomp == 9) sprintf(szline, "XYXY - %s", sz);
			else if (ncomp == 10) sprintf(szline, "XXYZ - %s", sz);
			else if (ncomp == 11) sprintf(szline, "YYYZ - %s", sz);
			else if (ncomp == 12) sprintf(szline, "ZZYZ - %s", sz);
			else if (ncomp == 13) sprintf(szline, "XYYZ - %s", sz);
			else if (ncomp == 14) sprintf(szline, "YZYZ - %s", sz);
			else if (ncomp == 15) sprintf(szline, "XXXZ - %s", sz);
			else if (ncomp == 16) sprintf(szline, "YYXZ - %s", sz);
			else if (ncomp == 17) sprintf(szline, "ZZXZ - %s", sz);
			else if (ncomp == 18) sprintf(szline, "XYXZ - %s", sz);
			else if (ncomp == 19) sprintf(szline, "YZXZ - %s", sz);
			else if (ncomp == 20) sprintf(szline, "XZXZ - %s", sz);
			return szline;
		}
		break;
		case DATA_ARRAY:
			{
				if (m_arrayNames.size() == m_arraySize)
				{
					sprintf(szline, "%s (%s)", sz, m_arrayNames[ncomp].c_str());
				}
				else
				{
					sprintf(szline, "%s[%d]", sz, ncomp);
				}
				return szline;
			}
			break;
		case DATA_ARRAY_VEC3:
			{
				int index = ncomp / 4;
				int m = ncomp % 4;

				if (m_arrayNames.size() == m_arraySize)
				{
					const string& arr = m_arrayNames[index];
					switch (m)
					{
					case 0: sprintf(szline, "X - %s (%s)", sz, arr.c_str()); break;
					case 1: sprintf(szline, "Y - %s (%s)", sz, arr.c_str()); break;
					case 2: sprintf(szline, "Z - %s (%s)", sz, arr.c_str()); break;
					case 3: sprintf(szline, "Total %s (%s)", sz, arr.c_str()); break;
					}
				}
				else
				{
					switch (m)
					{
					case 0: sprintf(szline, "X - %s [%d]", sz, index); break;
					case 1: sprintf(szline, "Y - %s [%d]", sz, index); break;
					case 2: sprintf(szline, "Z - %s [%d]", sz, index); break;
					case 3: sprintf(szline, "Total %s [%d]", sz, index); break;
					}
				}
				return szline;
			}
			break;
		}
	}
	else if (ntype == TENSOR_VECTOR)
	{
		switch (m_ntype)
		{
		case DATA_VEC3: return name; break;
		case DATA_MAT3S:
			{
				if      (ncomp == 0) sprintf(szline, "1 Principal %s", sz);
				else if (ncomp == 1) sprintf(szline, "2 Principal %s", sz);
				else if (ncomp == 2) sprintf(szline, "3 Principal %s", sz);
				return szline;
			}
		break;
		case DATA_MAT3:
			{
				if      (ncomp == 0) sprintf(szline, "column 1 %s", sz);
				else if (ncomp == 1) sprintf(szline, "column 2 %s", sz);
				else if (ncomp == 2) sprintf(szline, "column 3 %s", sz);
				return szline;
			}
		break;
		case DATA_ARRAY_VEC3:
			{
				const string& arr = m_arrayNames[ncomp];
				if (m_arrayNames.size() == m_arraySize)
				{
					sprintf(szline, "%s (%s)", sz, arr.c_str());
				}
				else
				{
					sprintf(szline, "%s [%d]", sz, ncomp);
				}
				return szline;
			}
		break;
		}
	}
	else if (ntype == TENSOR_TENSOR2)
	{
		switch (m_ntype)
		{
		case DATA_MAT3  : return name; break;
		case DATA_MAT3S : return name; break;
		case DATA_MAT3SD: return name; break;
		}
	}

	return "(invalid)";
}

void ModelDataField::SetUnits(const char* sz)
{
	if (sz) m_units = sz;
	else m_units.clear();
}

const char* ModelDataField::GetUnits() const
{
	if (m_units.empty()) return nullptr;
	else return m_units.c_str();
}

//=================================================================================================
FEArrayDataField::FEArrayDataField(FEPostModel* fem, DATA_CLASS c, DATA_FORMAT f, unsigned int flag) : ModelDataField(fem, DATA_ARRAY, f, c, flag)
{
}

ModelDataField* FEArrayDataField::Clone() const
{
	FEArrayDataField* newData = new FEArrayDataField(m_fem, DataClass(), Format(), m_flag);
	newData->SetName(GetName());
	newData->SetArraySize(GetArraySize());
    vector<string> arrnames = GetArrayNames();
	newData->SetArrayNames(arrnames);
	return newData;
}

Post::FEMeshData* FEArrayDataField::CreateData(FEState* pstate)
{
	switch (DataClass())
	{
	case NODE_DATA: return new FENodeArrayData(pstate, GetArraySize()); break;
	case ELEM_DATA: 
		switch (Format())
		{
		case DATA_ITEM: return new FEElemArrayDataItem(pstate, GetArraySize(), this); break;
		case DATA_NODE: return new FEElemArrayDataNode(pstate, GetArraySize(), this); break;
		}
		break;
	case FACE_DATA:
		switch (Format())
		{
		case DATA_ITEM: return new FEFaceArrayDataItem(pstate, GetArraySize(), this); break;
		}
		break;
	case OBJECT_DATA: return new FEGlobalArrayData(pstate, GetArraySize()); break;
	}
	assert(false);
	return 0;
}

//=================================================================================================
FEArrayVec3DataField::FEArrayVec3DataField(FEPostModel* fem, DATA_CLASS c, unsigned int flag) : ModelDataField(fem, DATA_ARRAY_VEC3, DATA_ITEM, c, flag)
{
}

ModelDataField* FEArrayVec3DataField::Clone() const
{
	FEArrayVec3DataField* newData = new FEArrayVec3DataField(m_fem, DataClass(), m_flag);
	newData->SetName(GetName());
	newData->SetArraySize(GetArraySize());
    vector<string> arrnames = GetArrayNames();
    newData->SetArrayNames(arrnames);
	return newData;
}

Post::FEMeshData* FEArrayVec3DataField::CreateData(FEState* pstate)
{
	switch (DataClass())
	{
	case ELEM_DATA: return new FEElemArrayVec3Data(pstate, GetArraySize(), this); break;
	}
	assert(false);
	return 0;
}

//=================================================================================================

bool Post::ExportDataField(Post::FEPostModel& fem, const ModelDataField& df, const char* szfile, bool selOnly, bool writeConn, const std::vector<int>& states)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	bool bret = false;
	int nfield = df.GetFieldID();
	if (IS_NODE_FIELD(nfield))
	{
		bret = Post::ExportNodeDataField(fem, df, fp, selOnly, states);
	}
	else if (IS_ELEM_FIELD(nfield))
	{
		bret = Post::ExportElementDataField(fem, df, fp, selOnly, writeConn, states);
	}
	else if (IS_FACE_FIELD(nfield))
	{
		bret = Post::ExportFaceDataField(fem, df, fp, selOnly, writeConn, states);
	}
	fclose(fp);

	return bret;
}

bool Post::ExportNodeDataField(FEPostModel& fem, const ModelDataField& df, FILE* fp, bool selOnly, const std::vector<int>& states)
{
	int nfield = df.GetFieldID();
	int ndata = FIELD_CODE(nfield);

	// get the mesh
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nstates = (int) states.size();

	// loop over all nodes
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = mesh.Node(i);

		if ((selOnly == false) || node.IsSelected())
		{
			// write the node ID
			fprintf(fp, "%d,", i + 1);

			// loop over all states
			for (int n = 0; n < nstates; ++n)
			{
				FEState& s = *fem.GetState( states[n] );

				FEMeshData& d = s.m_Data[ndata];
				DATA_FORMAT fmt = d.GetFormat();
				switch (d.GetType())
				{
				case DATA_SCALAR:
				{
					FENodeData_T<float>* pf = dynamic_cast<FENodeData_T<float>*>(&d);
					float f; pf->eval(i, &f);
					fprintf(fp, "%g", f);
				}
				break;
				case DATA_VEC3:
				{
					FENodeData_T<vec3f>* pf = dynamic_cast<FENodeData_T<vec3f>*>(&d);
					vec3f f; pf->eval(i, &f);
					fprintf(fp, "%g,%g,%g", f.x, f.y, f.z);
				}
				break;
				case DATA_MAT3S:
				{
					FENodeData_T<mat3fs>* pf = dynamic_cast<FENodeData_T<mat3fs>*>(&d);
					mat3fs f; pf->eval(i, &f);
					fprintf(fp, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz);
				}
				break;
				}
				if (n != nstates - 1) fprintf(fp, ",");
			}
			fprintf(fp, "\n");
		}
	}

	return true;
}


bool Post::ExportFaceDataField(FEPostModel& fem, const ModelDataField& df, FILE* fp, bool selOnly, bool writeConn, const std::vector<int>& states)
{
	int nfield = df.GetFieldID();
	int ndata = FIELD_CODE(nfield);

	// get the mesh
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nstates = (int)states.size();

	char buf[8192] = { 0 };

	// write connectivity 
	if (writeConn)
	{
		int NF = mesh.Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = mesh.Face(i);

			if ((selOnly == false) || face.IsSelected())
			{
				// write the element ID
				fprintf(fp, "%d,", i + 1);
				int nf = face.Nodes();
				for (int j = 0; j < nf; ++j)
				{
					fprintf(fp, " %d", face.n[j] + 1);
					if (j != nf - 1) fprintf(fp, ",");
				}
				fprintf(fp, "\n");
			}
		}
	}

	// loop over all elements
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = mesh.Face(i);

		if ((selOnly == false) || face.IsSelected())
		{
			// write the element ID
			char* sz = buf;
			sprintf(sz, "%d,", i + 1); sz += strlen(sz);

			// loop over all states
			bool bwrite = true;
			for (int n = 0; n < nstates; ++n)
			{
				FEState& s = *fem.GetState( states[n] );

				FEMeshData& d = s.m_Data[ndata];
				DATA_FORMAT fmt = d.GetFormat();
				if (fmt == DATA_ITEM)
				{
					switch (d.GetType())
					{
					case DATA_SCALAR:
					{
						FEFaceData_T<float, DATA_ITEM>* pf = dynamic_cast<FEFaceData_T<float, DATA_ITEM>*>(&d);
						if (pf->active(i))
						{
							float f; pf->eval(i, &f);
							sprintf(sz, "%g", f); sz += strlen(sz);
						}
						else bwrite = false;
					}
					break;
					case DATA_VEC3:
					{
						FEFaceData_T<vec3f, DATA_ITEM>* pf = dynamic_cast<FEFaceData_T<vec3f, DATA_ITEM>*>(&d);
						if (pf->active(i))
						{
							vec3f f; pf->eval(i, &f);
							sprintf(sz, "%g,%g,%g", f.x, f.y, f.z); sz += strlen(sz);
						}
						else bwrite = false;
					}
					break;
					case DATA_MAT3S:
					{
						FEFaceData_T<mat3fs, DATA_ITEM>* pf = dynamic_cast<FEFaceData_T<mat3fs, DATA_ITEM>*>(&d);
						if (pf->active(i))
						{
							mat3fs f; pf->eval(i, &f);
							sprintf(sz, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz); sz += strlen(sz);
						}
						else bwrite = false;
					}
					break;
					}
				}
				else if (fmt == DATA_MULT)
				{
					int nn = face.Nodes();

				switch (d.GetType())
				{
				case DATA_SCALAR:
				{
					FEFaceData_T<float, DATA_MULT>* pf = dynamic_cast<FEFaceData_T<float, DATA_MULT>*>(&d);
					if (pf->active(i))
					{
						float v[FSElement::MAX_NODES]; pf->eval(i, v);
						float f = 0.0f;
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g", f); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_VEC3:
				{
					FEFaceData_T<vec3f, DATA_MULT>* pf = dynamic_cast<FEFaceData_T<vec3f, DATA_MULT>*>(&d);
					if (pf->active(i))
					{
						vec3f v[FSElement::MAX_NODES]; pf->eval(i, v);
						vec3f f(0.f, 0.f, 0.f);
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g,%g,%g", f.x, f.y, f.z); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_MAT3S:
				{
					FEFaceData_T<mat3fs, DATA_MULT>* pf = dynamic_cast<FEFaceData_T<mat3fs, DATA_MULT>*>(&d);
					if (pf->active(i))
					{
						mat3fs v[FSElement::MAX_NODES]; pf->eval(i, v);
						mat3fs f(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				default:
					return false;
				}
			}
			else if (fmt == DATA_NODE)
			{
				int nn = face.Nodes();

				switch (d.GetType())
				{
				case DATA_SCALAR:
				{
					FEFaceData_T<float, DATA_NODE>* pf = dynamic_cast<FEFaceData_T<float, DATA_NODE>*>(&d);
					if (pf->active(i))
					{
						float v[FSElement::MAX_NODES]; pf->eval(i, v);
						float f = 0.0f;
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g", f); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_VEC3:
				{
					FEFaceData_T<vec3f, DATA_NODE>* pf = dynamic_cast<FEFaceData_T<vec3f, DATA_NODE>*>(&d);
					if (pf->active(i))
					{
						vec3f v[FSElement::MAX_NODES]; pf->eval(i, v);
						vec3f f(0.f, 0.f, 0.f);
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g,%g,%g", f.x, f.y, f.z); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_MAT3S:
				{
					FEFaceData_T<mat3fs, DATA_NODE>* pf = dynamic_cast<FEFaceData_T<mat3fs, DATA_NODE>*>(&d);
					if (pf->active(i))
					{
						mat3fs v[FSElement::MAX_NODES]; pf->eval(i, v);
						mat3fs f(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				default:
					return false;
				}
			}
			else return false;

				if (n != nstates - 1) { sprintf(sz, ",");  sz += strlen(sz); }
			}
			sprintf(sz, "\n");

			if (bwrite) fprintf(fp, "%s", buf);
		}
	}

	return true;
}

bool Post::ExportElementDataField(FEPostModel& fem, const ModelDataField& df, FILE* fp, bool selOnly, bool writeConn, const std::vector<int>& states)
{
	int nfield = df.GetFieldID();
	int ndata = FIELD_CODE(nfield);

	// get the mesh
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nstates = (int)states.size();

	// write connectivity 
	if (writeConn)
	{
		int NE = mesh.Elements();
		for (int i = 0; i < NE; ++i)
		{
			FSElement_& el = mesh.ElementRef(i);

			if ((selOnly == false) || el.IsSelected())
			{
				// write the element ID
				fprintf(fp, "%d,", i + 1);
				int ne = el.Nodes();
				for (int j = 0; j < ne; ++j)
				{
					fprintf(fp, " %d", el.m_node[j] + 1);
					if (j != ne - 1) fprintf(fp, ",");
				}
				fprintf(fp, "\n");
			}
		}
	}

	// loop over all elements
	int NE = mesh.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);

		if ((selOnly == false) || el.IsSelected())
		{
			// write the element ID
			fprintf(fp, "%d,", i + 1);

			// loop over all states
			for (int n = 0; n < nstates; ++n)
			{
				FEState& s = *fem.GetState(states[n]);

				FEMeshData& d = s.m_Data[ndata];
				DATA_FORMAT fmt = d.GetFormat();
				if (fmt == DATA_ITEM)
				{
					switch (d.GetType())
					{
					case DATA_SCALAR:
					{
						FEElemData_T<float, DATA_ITEM>* pf = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&d);
						float f; pf->eval(i, &f);
						fprintf(fp, "%g", f);
					}
					break;
					case DATA_VEC3:
					{
						FEElemData_T<vec3f, DATA_ITEM>* pf = dynamic_cast<FEElemData_T<vec3f, DATA_ITEM>*>(&d);
						vec3f f; pf->eval(i, &f);
						fprintf(fp, "%g,%g,%g", f.x, f.y, f.z);
					}
					break;
					case DATA_MAT3S:
					{
						FEElemData_T<mat3fs, DATA_ITEM>* pf = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&d);
						mat3fs f; pf->eval(i, &f);
						fprintf(fp, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz);
					}
					break;
					case DATA_ARRAY:
					{
						FEElemArrayDataItem& dm = dynamic_cast<FEElemArrayDataItem&>(d);
						int nsize = dm.arraySize();
						vector<double> data(nsize, 0.0);
						if (dm.active(i))
						{
							for (int j = 0; j < nsize; ++j) data[j] = dm.eval(i, j);
						}
						for (int j = 0; j < nsize; ++j)
						{
							fprintf(fp, "%g", data[j]);
							if (j != nsize - 1) fprintf(fp, ",");
						}
					}
					break;
					default:
						assert(false);
					}
				}
				else if (fmt == DATA_MULT)
				{
					int nn = el.Nodes();

					switch (d.GetType())
					{
					case DATA_SCALAR:
					{
						FEElemData_T<float, DATA_MULT>* pf = dynamic_cast<FEElemData_T<float, DATA_MULT>*>(&d);
						float v[FSElement::MAX_NODES]; pf->eval(i, v);
						float f = 0.0f;
						for (int i = 0; i < nn; ++i) f += v[i]; f /= (float)nn;
						fprintf(fp, "%g", f);
					}
					break;
					case DATA_VEC3:
					{
						FEElemData_T<vec3f, DATA_MULT>* pf = dynamic_cast<FEElemData_T<vec3f, DATA_MULT>*>(&d);
						vec3f v[FSElement::MAX_NODES]; pf->eval(i, v);
						vec3f f(0.f, 0.f, 0.f);
						for (int i = 0; i < nn; ++i) f += v[i]; f /= (float)nn;
						fprintf(fp, "%g,%g,%g", f.x, f.y, f.z);
					}
					break;
					case DATA_MAT3S:
					{
						FEElemData_T<mat3fs, DATA_MULT>* pf = dynamic_cast<FEElemData_T<mat3fs, DATA_MULT>*>(&d);
						mat3fs v[FSElement::MAX_NODES]; pf->eval(i, v);
						mat3fs f(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
						for (int i = 0; i < nn; ++i) f += v[i]; f /= (float)nn;
						fprintf(fp, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz);
					}
					break;
					}
				}
				else if (fmt == DATA_REGION)
				{
					switch (d.GetType())
					{
					case DATA_SCALAR:
					{
						FEElemData_T<float, DATA_REGION>* pf = dynamic_cast<FEElemData_T<float, DATA_REGION>*>(&d);
						float v; pf->eval(i, &v);
						fprintf(fp, "%g", v);
					}
					break;
					case DATA_VEC3:
					{
						FEElemData_T<vec3f, DATA_REGION>* pf = dynamic_cast<FEElemData_T<vec3f, DATA_REGION>*>(&d);
						vec3f v; pf->eval(i, &v);
						fprintf(fp, "%g,%g,%g", v.x, v.y, v.z);
					}
					break;
					}
				}
				if (n != nstates - 1) fprintf(fp, ",");
			}
			fprintf(fp, "\n");
		}
	}

	return true;
}

//=============================================================================

class StandardDataField
{
public:
	StandardDataField(const char* sz, int flag) : m_szname(sz), m_flag(flag) {}
	virtual ~StandardDataField() {}
public:
	virtual Post::ModelDataField* Create(Post::FEPostModel* fem) = 0;
	
public:
	const char*	m_szname;
	int			m_flag;
};

template <typename T> class StandardDataField_T : public StandardDataField
{
public:
	StandardDataField_T(const char* szname, int flag = 0) : StandardDataField(szname, flag) {}
	Post::ModelDataField* Create(Post::FEPostModel* fem) override 
	{ 
		Post::ModelDataField* dataField = new T(fem, m_flag);
		dataField->SetName(m_szname);
		return dataField;
	}
};

class StandardDataFieldManager
{
public:
	static void Init();

	static int StandardDataFields() { return (int)m_stdDataFields.size(); }

	static std::string GetStandarDataFieldName(int i)
	{ 
		assert((i >= 0) && (i < m_stdDataFields.size()));
		std::string name;
		if ((i >= 0) && (i < m_stdDataFields.size()))
		{
			name = m_stdDataFields[i]->m_szname;
		}
		return name;
	}

	static Post::ModelDataField* CreateDataField(Post::FEPostModel* fem, const std::string& name)
	{
		for (int i = 0; i < m_stdDataFields.size(); ++i)
		{
			std::string namei = m_stdDataFields[i]->m_szname;
			if (namei == name)
			{
				return m_stdDataFields[i]->Create(fem);
			}
		}
		return nullptr;
	}

	template <typename T>
	static void Add(const char* sz, int flag = 0)
	{
		m_stdDataFields.push_back(new StandardDataField_T<T>(sz, flag));
	}

private:
	StandardDataFieldManager() {}

	static std::vector<StandardDataField*>	m_stdDataFields;
};

std::vector<StandardDataField*>	StandardDataFieldManager::m_stdDataFields;

void StandardDataFieldManager::Init()
{
	if (m_stdDataFields.empty() == false) return;

	Add<FEDataField_T<NodePosition> >("Position");
	Add<FEDataField_T<NodeInitPos > >("Initial Position");
	Add<FEDataField_T<DeformationGradient> >("Deformation gradient");
	Add<StrainDataField >("Infinitesimal strain", StrainDataField::INF_STRAIN);
	Add<StrainDataField >("Lagrange strain"     , StrainDataField::LAGRANGE          );
	Add<StrainDataField >("Right Cauchy-Green"  , StrainDataField::RIGHT_CAUCHY_GREEN);
	Add<StrainDataField >("Right stretch"       , StrainDataField::RIGHT_STRETCH     );
	Add<StrainDataField >("Biot strain"         , StrainDataField::BIOT              );
	Add<StrainDataField >("Right Hencky"        , StrainDataField::RIGHT_HENCKY      );
	Add<StrainDataField >("Left Cauchy-Green"   , StrainDataField::LEFT_CAUCHY_GREEN );
	Add<StrainDataField >("Left stretch"        , StrainDataField::LEFT_STRETCH      );
	Add<StrainDataField >("Left Hencky"         , StrainDataField::LEFT_HENCKY       );
	Add<StrainDataField >("Almansi strain"      , StrainDataField::ALMANSI           );
	Add<FEDataField_T<LagrangeStrain2D     > >("Lagrange Strain 2D");
	Add<FEDataField_T<InfStrain2D          > >("Infinitesimal Strain 2D");
	Add<FEDataField_T<ElementVolume        > >("Volume"                    );
	Add<FEDataField_T<VolumeRatio          > >("Volume ratio"              );
	Add<FEDataField_T<VolumeStrain         > >("Volume strain"             );
	Add<FEDataField_T<AspectRatio          > >("Aspect ratio"              );
	Add<CurvatureField >("1-Princ curvature"         , CurvatureField::PRINC1_CURVATURE);
	Add<CurvatureField >("2-Princ curvature"         , CurvatureField::PRINC2_CURVATURE);
	Add<CurvatureField >("Gaussian curvature"        , CurvatureField::GAUSS_CURVATURE );
	Add<CurvatureField >("Mean curvature"            , CurvatureField::MEAN_CURVATURE  );
	Add<CurvatureField >("RMS curvature"             , CurvatureField::RMS_CURVATURE   );
	Add<CurvatureField >("Princ curvature difference", CurvatureField::DIFF_CURVATURE  );
	Add<FEDataField_T<SurfaceCongruency           > >("Congruency"              );
	Add<FEDataField_T<PrincCurvatureVector1> >("1-Princ curvature vector");
	Add<FEDataField_T<PrincCurvatureVector2> >("2-Princ curvature vector");
	Add<FEDistanceMap >("Distance map");
	Add<FEAreaCoverage>("Area coverage");
	Add<FEDataField_T<FEFacetArea> >("Facet area");
	Add<FEDataField_T<FEElementMaterial> >("Material ID");
	Add<FEDataField_T<FESurfaceNormal> >("Surface normal");
}

void Post::InitStandardDataFields()
{
	StandardDataFieldManager::Init();
}

int Post::StandardDataFields()
{
	return StandardDataFieldManager::StandardDataFields();
}

std::string Post::GetStandarDataFieldName(int i)
{
	return StandardDataFieldManager::GetStandarDataFieldName(i);
}

bool Post::AddStandardDataField(Post::FEPostModel& fem, const std::string& dataField)
{
	ModelDataField* pdf = StandardDataFieldManager::CreateDataField(&fem, dataField);
	if (pdf == nullptr) return false;
	fem.AddDataField(pdf);
	return true;
}


bool Post::AddStandardDataField(Post::FEPostModel& fem, const std::string& dataField, vector<int> selectionList)
{
	ModelDataField* pdf = StandardDataFieldManager::CreateDataField(&fem, dataField);
	if (pdf == nullptr) return false;

	// NOTE: This only works with curvatures
	if (selectionList.empty() == false)
	{
		fem.AddDataField(pdf, selectionList);
	}
	else fem.AddDataField(pdf);

	return true;
}

//------------------------------------------------------------------------------------------
bool Post::AddNodeDataFromFile(FEPostModel& fem, const char* szfile, const char* szname, int ntype)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// get the mesh
	FSMesh& m = *fem.GetFEMesh(0);

	// create a new data field
	int ND = 0;
	Post::ModelDataField* pdf = nullptr;
	switch (ntype)
	{
	case DATA_SCALAR : pdf = new FEDataField_T<FENodeData<float  > >(&fem, EXPORT_DATA); ND =  1; break;
	case DATA_VEC3  : pdf = new FEDataField_T<FENodeData<vec3f  > >(&fem, EXPORT_DATA); ND =  3; break;
	case DATA_MAT3  : pdf = new FEDataField_T<FENodeData<mat3f  > >(&fem, EXPORT_DATA); ND =  9; break;
	case DATA_MAT3S : pdf = new FEDataField_T<FENodeData<mat3fs > >(&fem, EXPORT_DATA); ND =  6; break;
	case DATA_MAT3SD: pdf = new FEDataField_T<FENodeData<mat3fd > >(&fem, EXPORT_DATA); ND =  3; break;
	case DATA_TENS4S: pdf = new FEDataField_T<FENodeData<tens4fs> >(&fem, EXPORT_DATA); ND = 21; break;
	}
	assert(pdf);
	if (pdf == nullptr)
	{
		fclose(fp);
		return false;
	}

	// Add the data field
	pdf->SetName(szname);
	fem.AddDataField(pdf);

	// the data should be organized in a comma seperated list. 
	// the first entry identifies the node for which the data is intended
	// the second and following contain the data, one entry for each state
	char szline[8192] = { 0 }, *ch, *sz;
	int N = 0;
	do
	{
		// read a line
		fgets(szline, 8191, fp);
		sz = szline;

		// read the first entry
		ch = strchr(szline, ',');
		if (ch) *ch = 0;
		int node = atoi(sz) - 1;
		if (ch) sz = ch + 1;

		if ((node >= 0) && (node < m.Nodes()))
		{
			int nstate = 0;
			while (ch && (nstate < fem.GetStates()))
			{
				float f[21] = { 0 };
				int nf = 0;
				do
				{
					f[nf++] = (float)atof(sz);
					ch = strchr(sz, ',');
					if (ch) { *ch = 0; sz = ch + 1; }
				} while (ch && (nf < ND));

				// get the state
				FEState* ps = fem.GetState(nstate);

				int ndf = ps->m_Data.size();

				// get the datafield
				switch (ntype)
				{
				case DATA_SCALAR:
				{
					FENodeData<float>& df = dynamic_cast<FENodeData<float>&>(ps->m_Data[ndf - 1]);
					df[node] = f[0];
				}
				break;
				case DATA_VEC3:
				{
					FENodeData<vec3f>& df = dynamic_cast<FENodeData<vec3f>&>(ps->m_Data[ndf - 1]);
					df[node] = vec3f(f[0], f[1], f[2]);
				}
				break;
				case DATA_MAT3:
				{
					FENodeData<mat3f>& df = dynamic_cast<FENodeData<mat3f>&>(ps->m_Data[ndf - 1]);
					df[node] = mat3f(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]);
				}
				break;
				case DATA_MAT3S:
				{
					FENodeData<mat3fs>& df = dynamic_cast<FENodeData<mat3fs>&>(ps->m_Data[ndf - 1]);
					df[node] = mat3fs(f[0], f[1], f[2], f[3], f[4], f[5]);
				}
				break;
				case DATA_MAT3SD:
				{
					FENodeData<mat3fd>& df = dynamic_cast<FENodeData<mat3fd>&>(ps->m_Data[ndf - 1]);
					df[node] = mat3fd(f[0], f[1], f[2]);
				}
				break;
				case DATA_TENS4S:
				{
					FENodeData<tens4fs>& df = dynamic_cast<FENodeData<tens4fs>&>(ps->m_Data[ndf - 1]);
					df[node] = tens4fs(f);
				}
				break;
				};

				++nstate;
			}
		}
		++N;
	} while (!feof(fp) && !ferror(fp) && (N < m.Nodes()));

	fclose(fp);
	return true;
}

//------------------------------------------------------------------------------------------
bool Post::AddFaceDataFromFile(Post::FEPostModel& fem, const char* szfile, const char* szname, int ntype)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// get the mesh
	FSMesh& m = *fem.GetFEMesh(0);

	// create a new data field
	int ND = 0;
	Post::ModelDataField* pdf = nullptr;
	switch (ntype)
	{
	case DATA_SCALAR : pdf = new FEDataField_T<FEFaceData<float  , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 1; break;
	case DATA_VEC3  : pdf = new FEDataField_T<FEFaceData<vec3f  , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 3; break;
	case DATA_MAT3  : pdf = new FEDataField_T<FEFaceData<mat3f  , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 9; break;
	case DATA_MAT3S : pdf = new FEDataField_T<FEFaceData<mat3fs , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 6; break;
	case DATA_MAT3SD: pdf = new FEDataField_T<FEFaceData<mat3fd , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 3; break;
	case DATA_TENS4S: pdf = new FEDataField_T<FEFaceData<tens4fs, DATA_ITEM> >(&fem, EXPORT_DATA); ND = 21; break;
	}
	assert(pdf);
	if (pdf == nullptr)
	{
		fclose(fp);
		return false;
	}

	// add the data field
	pdf->SetName(szname);
	fem.AddDataField(pdf);

	// the data should be organized in a comma seperated list. 
	// the first entry identifies the element for which the data is intended
	// the second and following contain the data, one entry for each state
	char szline[8192] = { 0 }, *ch, *sz;
	int N = 0;
	do
	{
		// read a line
		fgets(szline, 8191, fp);
		sz = szline;

		// read the first entry
		ch = strchr(szline, ',');
		if (ch) *ch = 0;
		int nface = atoi(sz) - 1;
		if (ch) sz = ch + 1;

		if ((nface >= 0) || (nface < m.Faces()))
		{
			int nstate = 0;
			while (ch && (nstate < fem.GetStates()))
			{
				float f[21] = { 0 };
				int nf = 0;
				do
				{
					f[nf++] = (float)atof(sz);
					ch = strchr(sz, ',');
					if (ch) { *ch = 0; sz = ch + 1; }
				} while (ch && (nf < ND));

				// get the state
				FEState* ps = fem.GetState(nstate);

				int ndf = ps->m_Data.size();

				// get the datafield
				switch (ntype)
				{
				case DATA_SCALAR:
				{
					FEFaceData<float, DATA_ITEM>& df = dynamic_cast<FEFaceData<float, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, f[0]);
				}
				break;
				case DATA_VEC3:
				{
					FEFaceData<vec3f, DATA_ITEM>& df = dynamic_cast<FEFaceData<vec3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, vec3f(f[0], f[1], f[2]));
				}
				break;
				case DATA_MAT3:
				{
					FEFaceData<mat3f, DATA_ITEM>& df = dynamic_cast<FEFaceData<mat3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, mat3f(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]));
				}
				break;
				case DATA_MAT3S:
				{
					FEFaceData<mat3fs, DATA_ITEM>& df = dynamic_cast<FEFaceData<mat3fs, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, mat3fs(f[0], f[1], f[2], f[3], f[4], f[5]));
				}
				break;
				case DATA_MAT3SD:
				{
					FEFaceData<mat3fd, DATA_ITEM>& df = dynamic_cast<FEFaceData<mat3fd, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, mat3fd(f[0], f[1], f[2]));
				}
				break;
				case DATA_TENS4S:
				{
					FEFaceData<tens4fs, DATA_ITEM>& df = dynamic_cast<FEFaceData<tens4fs, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, tens4fs(f));
				}
				break;
				}

				++nstate;
			}
		}
		++N;
	} while (!feof(fp) && !ferror(fp) && (N < m.Faces()));

	fclose(fp);
	return true;
}

//------------------------------------------------------------------------------------------
bool Post::AddElemDataFromFile(Post::FEPostModel& fem, const char* szfile, const char* szname, int ntype)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// get the mesh
	FSMesh& m = *fem.GetFEMesh(0);

	// create a new data field
	int ND = 0;
	Post::ModelDataField* pdf = nullptr;
	switch (ntype)
	{
	case DATA_SCALAR : pdf = new FEDataField_T<FEElementData<float  , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 1; break;
	case DATA_VEC3  : pdf = new FEDataField_T<FEElementData<vec3f  , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 3; break;
	case DATA_MAT3  : pdf = new FEDataField_T<FEElementData<mat3f  , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 9; break;
	case DATA_MAT3S : pdf = new FEDataField_T<FEElementData<mat3fs , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 6; break;
	case DATA_MAT3SD: pdf = new FEDataField_T<FEElementData<mat3fd , DATA_ITEM> >(&fem, EXPORT_DATA); ND = 3; break;
	case DATA_TENS4S: pdf = new FEDataField_T<FEElementData<tens4fs, DATA_ITEM> >(&fem, EXPORT_DATA); ND = 21; break;
	}
	assert(pdf);
	if (pdf == nullptr)
	{
		fclose(fp);
		return false;
	}

	// add the data field
	pdf->SetName(szname);
	fem.AddDataField(pdf);

	// the data should be organized in a comma seperated list. 
	// the first entry identifies the element for which the data is intended
	// the second and following contain the data, one entry for each state
	char szline[8192] = { 0 }, *ch, *sz;
	int N = 0;
	do
	{
		// read a line
		fgets(szline, 8191, fp);
		sz = szline;

		// read the first entry
		ch = strchr(szline, ',');
		if (ch) *ch = 0;
		int nelem = atoi(sz) - 1;
		if (ch) sz = ch + 1;

		if ((nelem >= 0) || (nelem < m.Elements()))
		{
			int nstate = 0;
			while (ch && (nstate < fem.GetStates()))
			{
				float f[21] = { 0 };
				int nf = 0;
				do
				{
					f[nf++] = (float)atof(sz);
					ch = strchr(sz, ',');
					if (ch) { *ch = 0; sz = ch + 1; }
				} while (ch && (nf < ND));

				// get the state
				FEState* ps = fem.GetState(nstate);

				int ndf = ps->m_Data.size();

				// get the datafield
				switch (ntype)
				{
				case DATA_SCALAR:
				{
					FEElementData<float, DATA_ITEM>& df = dynamic_cast<FEElementData<float, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, f[0]);
				}
				break;
				case DATA_VEC3:
				{
					FEElementData<vec3f, DATA_ITEM>& df = dynamic_cast<FEElementData<vec3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, vec3f(f[0], f[1], f[2]));
				}
				break;
				case DATA_MAT3:
				{
					FEElementData<mat3f, DATA_ITEM>& df = dynamic_cast<FEElementData<mat3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, mat3f(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]));
				}
				break;
				case DATA_MAT3S:
				{
					FEElementData<mat3fs, DATA_ITEM>& df = dynamic_cast<FEElementData<mat3fs, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, mat3fs(f[0], f[1], f[2], f[3], f[4], f[5]));
				}
				break;
				case DATA_MAT3SD:
				{
					FEElementData<mat3fd, DATA_ITEM>& df = dynamic_cast<FEElementData<mat3fd, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, mat3fd(f[0], f[1], f[2]));
				}
				break;
				case DATA_TENS4S:
				{
					FEElementData<tens4fs, DATA_ITEM>& df = dynamic_cast<FEElementData<tens4fs, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, tens4fs(f));
				}
				break;
				}

				++nstate;
			}
		}
		++N;
	} while (!feof(fp) && !ferror(fp) && (N < m.Elements()));

	fclose(fp);
	return true;
}
