#include "stdafx.h"
#include "FEDataField.h"
#include "FEMeshData_T.h"
#include "constants.h"
#include "PostGL/GLModel.h"
using namespace Post;

FEDataField::FEDataField(const std::string& name, Data_Type ntype, Data_Format nfmt, Data_Class ncls, unsigned int flag)
{
	m_ntype = ntype;
	m_nfmt = nfmt;
	m_nclass = ncls;
	m_nref = 0;
	m_flag = flag;
	m_name = name;
	m_arraySize = 0;
}


void FEDataField::SetName(const std::string& newName)
{
	m_name = newName;
}

void FEDataField::SetArrayNames(vector<string>& n)
{
	m_arrayNames = n;
}

vector<string> FEDataField::GetArrayNames() const
{
	return m_arrayNames;
}

const char* FEDataField::TypeStr() const
{
	switch (m_ntype)
	{
	case DATA_FLOAT: return "float"; break;
	case DATA_VEC3F: return "vec3f"; break;
	case DATA_MAT3F: return "mat3f"; break;
	case DATA_MAT3D: return "Mat3d"; break;
	case DATA_MAT3FS: return "mat3fs"; break;
	case DATA_MAT3FD: return "mat3fd"; break;
	case DATA_TENS4FS: return "tens4fs"; break;
	case DATA_ARRAY: return "array"; break;
	case DATA_ARRAY_VEC3F: return "array<vec3>"; break;
	};
	return "unknown";
}

int FEDataField::components(Data_Tensor_Type ntype)
{
	if (ntype == DATA_SCALAR)
	{
		switch (m_ntype)
		{
		case DATA_FLOAT: return 1; break;
		case DATA_VEC3F: return 7; break;
		case DATA_MAT3F: return 9; break;
		case DATA_MAT3D: return 9; break;
		case DATA_MAT3FS: return 14; break;
		case DATA_MAT3FD: return 3; break;
		case DATA_TENS4FS: return 21; break;
		case DATA_ARRAY: return GetArraySize(); break;
		case DATA_ARRAY_VEC3F: return GetArraySize()*4; break;
		}
	}
	else if (ntype == DATA_VECTOR)
	{
		switch (m_ntype)
		{
		case DATA_FLOAT: return 0; break;
		case DATA_VEC3F: return 1; break;
		case DATA_MAT3F: return 3; break;
		case DATA_MAT3D: return 3; break;
		case DATA_MAT3FS: return 3; break;
		case DATA_MAT3FD: return 0; break;
		case DATA_TENS4FS: return 0; break;
		case DATA_ARRAY: return 0; break;
		case DATA_ARRAY_VEC3F: return GetArraySize(); break;
		}
	}
	else if (ntype == DATA_TENSOR2)
	{
		switch (m_ntype)
		{
		case DATA_FLOAT: return 0; break;
		case DATA_VEC3F: return 0; break;
		case DATA_MAT3F: return 1; break;
		case DATA_MAT3D: return 1; break;
		case DATA_MAT3FS: return 1; break;
		case DATA_MAT3FD: return 1; break;
		case DATA_TENS4FS: return 0; break;
		case DATA_ARRAY: return 0; break;
		case DATA_ARRAY_VEC3F: return 0; break;
		}
	}
	return 0;
}

std::string FEDataField::componentName(int ncomp, Data_Tensor_Type ntype)
{
	const std::string& name = GetName();
	const char* sz = name.c_str();

	char szline[256] = { 0 };

	if (ntype == DATA_SCALAR)
	{
		switch (m_ntype)
		{
		case DATA_FLOAT: return name; break;
		case DATA_VEC3F:
		{
			if (ncomp == 0) sprintf(szline, "X - %s", sz);
			else if (ncomp == 1) sprintf(szline, "Y - %s", sz);
			else if (ncomp == 2) sprintf(szline, "Z - %s", sz);
			else if (ncomp == 3) sprintf(szline, "XY - %s", sz);
			else if (ncomp == 4) sprintf(szline, "YZ - %s", sz);
			else if (ncomp == 5) sprintf(szline, "XZ - %s", sz);
			else if (ncomp == 6) sprintf(szline, "Total %s", sz);
			return szline;
		}
		break;
		case DATA_MAT3F:
		case DATA_MAT3D:
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
			return szline;
		}
		break;
		case DATA_MAT3FS:
		{
			if (ncomp == 0) sprintf(szline, "X - %s", sz);
			else if (ncomp == 1) sprintf(szline, "Y - %s", sz);
			else if (ncomp == 2) sprintf(szline, "Z - %s", sz);
			else if (ncomp == 3) sprintf(szline, "XY - %s", sz);
			else if (ncomp == 4) sprintf(szline, "YZ - %s", sz);
			else if (ncomp == 5) sprintf(szline, "XZ - %s", sz);
			else if (ncomp == 6) sprintf(szline, "Effective %s", sz);
			else if (ncomp == 7) sprintf(szline, "1 Principal %s", sz);
			else if (ncomp == 8) sprintf(szline, "2 Principal %s", sz);
			else if (ncomp == 9) sprintf(szline, "3 Principal %s", sz);
			else if (ncomp == 10) sprintf(szline, "1 Dev Principal %s", sz);
			else if (ncomp == 11) sprintf(szline, "2 Dev Principal %s", sz);
			else if (ncomp == 12) sprintf(szline, "3 Dev Principal %s", sz);
			else if (ncomp == 13) sprintf(szline, "Max Shear %s", sz);
			return szline;
		}
		break;
		case DATA_MAT3FD:
		{
			if (ncomp == 0) sprintf(szline, "1 - %s", sz);
			else if (ncomp == 1) sprintf(szline, "2 - %s", sz);
			else if (ncomp == 2) sprintf(szline, "3 - %s", sz);
			return szline;
		}
		break;
		case DATA_TENS4FS:
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
		case DATA_ARRAY_VEC3F:
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
	else if (ntype == DATA_VECTOR)
	{
		switch (m_ntype)
		{
		case DATA_VEC3F: return name; break;
		case DATA_MAT3FS:
			{
				if      (ncomp == 0) sprintf(szline, "1 Principal %s", sz);
				else if (ncomp == 1) sprintf(szline, "2 Principal %s", sz);
				else if (ncomp == 2) sprintf(szline, "3 Principal %s", sz);
				return szline;
			}
		break;
		case DATA_MAT3F:
		case DATA_MAT3D:
			{
				if      (ncomp == 0) sprintf(szline, "column 1 %s", sz);
				else if (ncomp == 1) sprintf(szline, "column 2 %s", sz);
				else if (ncomp == 2) sprintf(szline, "column 3 %s", sz);
				return szline;
			}
		break;
		case DATA_ARRAY_VEC3F:
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
	else if (ntype == DATA_TENSOR2)
	{
		switch (m_ntype)
		{
		case DATA_MAT3FS: return name; break;
		}
	}

	return "(invalid)";
}

//=================================================================================================
FEArrayDataField::FEArrayDataField(const std::string& name, Data_Class c, Data_Format f, unsigned int flag) : FEDataField(name, DATA_ARRAY, f, c, flag)
{
}

FEDataField* FEArrayDataField::Clone() const
{
	FEArrayDataField* newData = new FEArrayDataField(GetName(), DataClass(), Format(), m_flag);
	newData->SetArraySize(GetArraySize());
    vector<string> arrnames = GetArrayNames();
	newData->SetArrayNames(arrnames);
	return newData;
}

FEMeshData* FEArrayDataField::CreateData(FEState* pstate)
{
	switch (DataClass())
	{
	case CLASS_NODE: return new FENodeArrayData(pstate, GetArraySize()); break;
	case CLASS_ELEM: 
		switch (Format())
		{
		case DATA_ITEM: return new FEElemArrayDataItem(pstate, GetArraySize(), this); break;
		case DATA_NODE: return new FEElemArrayDataNode(pstate, GetArraySize(), this); break;
		}
		break;
	}
	assert(false);
	return 0;
}

//=================================================================================================
FEArrayVec3DataField::FEArrayVec3DataField(const std::string& name, Data_Class c, unsigned int flag) : FEDataField(name, DATA_ARRAY_VEC3F, DATA_ITEM, c, flag)
{
}

FEDataField* FEArrayVec3DataField::Clone() const
{
	FEArrayVec3DataField* newData = new FEArrayVec3DataField(GetName(), DataClass(), m_flag);
	newData->SetArraySize(GetArraySize());
    vector<string> arrnames = GetArrayNames();
    newData->SetArrayNames(arrnames);
	return newData;
}

FEMeshData* FEArrayVec3DataField::CreateData(FEState* pstate)
{
	switch (DataClass())
	{
	case CLASS_ELEM: return new FEElemArrayVec3Data(pstate, GetArraySize(), this); break;
	}
	assert(false);
	return 0;
}

//=================================================================================================

bool Post::ExportDataField(CGLModel& glm, const FEDataField& df, const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	bool bret = false;
	int nfield = df.GetFieldID();
	if (IS_NODE_FIELD(nfield))
	{
		bret = Post::ExportNodeDataField(glm, df, fp);
	}
	else if (IS_ELEM_FIELD(nfield))
	{
		bret = Post::ExportElementDataField(glm, df, fp);
	}
	else if (IS_FACE_FIELD(nfield))
	{
		bret = Post::ExportFaceDataField(glm, df, fp);
	}
	fclose(fp);

	return bret;
}

bool Post::ExportNodeDataField(CGLModel& glm, const FEDataField& df, FILE* fp)
{
	int nfield = df.GetFieldID();
	int ndata = FIELD_CODE(nfield);

	// get the mesh
	FEModel& fem = *glm.GetFEModel();
	FEMeshBase& mesh = *glm.GetActiveMesh();

	int nstates = fem.GetStates();

	// loop over all nodes
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FENode& node = mesh.Node(i);

		// write the node ID
		fprintf(fp, "%d,", i + 1);

		// loop over all states
		for (int n = 0; n<nstates; ++n)
		{
			FEState& s = *fem.GetState(n);

			FEMeshData& d = s.m_Data[ndata];
			Data_Format fmt = d.GetFormat();
			switch (d.GetType())
			{
			case DATA_FLOAT:
			{
				FENodeData_T<float>* pf = dynamic_cast<FENodeData_T<float>*>(&d);
				float f; pf->eval(i, &f);
				fprintf(fp, "%g", f);
			}
			break;
			case DATA_VEC3F:
			{
				FENodeData_T<vec3f>* pf = dynamic_cast<FENodeData_T<vec3f>*>(&d);
				vec3f f; pf->eval(i, &f);
				fprintf(fp, "%g,%g,%g", f.x, f.y, f.z);
			}
			break;
			case DATA_MAT3FS:
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

	return true;
}


bool Post::ExportFaceDataField(CGLModel& glm, const FEDataField& df, FILE* fp)
{
	int nfield = df.GetFieldID();
	int ndata = FIELD_CODE(nfield);

	// get the mesh
	FEModel& fem = *glm.GetFEModel();
	FEMeshBase& mesh = *glm.GetActiveMesh();

	int nstates = fem.GetStates();

	char buf[8192] = { 0 };

	// loop over all elements
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = mesh.Face(i);

		// write the element ID
		char* sz = buf;
		sprintf(sz, "%d,", i + 1); sz += strlen(sz);

		// loop over all states
		bool bwrite = true;
		for (int n = 0; n<nstates; ++n)
		{
			FEState& s = *fem.GetState(n);

			FEMeshData& d = s.m_Data[ndata];
			Data_Format fmt = d.GetFormat();
			if (fmt == DATA_ITEM)
			{
				switch (d.GetType())
				{
				case DATA_FLOAT:
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
				case DATA_VEC3F:
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
				case DATA_MAT3FS:
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
			else if (fmt == DATA_COMP)
			{
				int nn = face.Nodes();

				switch (d.GetType())
				{
				case DATA_FLOAT:
				{
					FEFaceData_T<float, DATA_COMP>* pf = dynamic_cast<FEFaceData_T<float, DATA_COMP>*>(&d);
					if (pf->active(i))
					{
						float v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
						float f = 0.0f;
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g", f); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_VEC3F:
				{
					FEFaceData_T<vec3f, DATA_COMP>* pf = dynamic_cast<FEFaceData_T<vec3f, DATA_COMP>*>(&d);
					if (pf->active(i))
					{
						vec3f v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
						vec3f f(0.f, 0.f, 0.f);
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g,%g,%g", f.x, f.y, f.z); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_MAT3FS:
				{
					FEFaceData_T<mat3fs, DATA_COMP>* pf = dynamic_cast<FEFaceData_T<mat3fs, DATA_COMP>*>(&d);
					if (pf->active(i))
					{
						mat3fs v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
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
				case DATA_FLOAT:
				{
					FEFaceData_T<float, DATA_NODE>* pf = dynamic_cast<FEFaceData_T<float, DATA_NODE>*>(&d);
					if (pf->active(i))
					{
						float v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
						float f = 0.0f;
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g", f); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_VEC3F:
				{
					FEFaceData_T<vec3f, DATA_NODE>* pf = dynamic_cast<FEFaceData_T<vec3f, DATA_NODE>*>(&d);
					if (pf->active(i))
					{
						vec3f v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
						vec3f f(0.f, 0.f, 0.f);
						for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
						sprintf(sz, "%g,%g,%g", f.x, f.y, f.z); sz += strlen(sz);
					}
					else bwrite = false;
				}
				break;
				case DATA_MAT3FS:
				{
					FEFaceData_T<mat3fs, DATA_NODE>* pf = dynamic_cast<FEFaceData_T<mat3fs, DATA_NODE>*>(&d);
					if (pf->active(i))
					{
						mat3fs v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
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

	return true;
}

bool Post::ExportElementDataField(CGLModel& glm, const FEDataField& df, FILE* fp)
{
	int nfield = df.GetFieldID();
	int ndata = FIELD_CODE(nfield);

	// get the mesh
	FEModel& fem = *glm.GetFEModel();
	FEMeshBase& mesh = *glm.GetActiveMesh();

	int nstates = fem.GetStates();

	// loop over all elements
	int NE = mesh.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);

		// write the element ID
		fprintf(fp, "%d,", i + 1);

		// loop over all states
		for (int n = 0; n<nstates; ++n)
		{
			FEState& s = *fem.GetState(n);

			FEMeshData& d = s.m_Data[ndata];
			Data_Format fmt = d.GetFormat();
			if (fmt == DATA_ITEM)
			{
				switch (d.GetType())
				{
				case DATA_FLOAT:
				{
					FEElemData_T<float, DATA_ITEM>* pf = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&d);
					float f; pf->eval(i, &f);
					fprintf(fp, "%g", f);
				}
				break;
				case DATA_VEC3F:
				{
					FEElemData_T<vec3f, DATA_ITEM>* pf = dynamic_cast<FEElemData_T<vec3f, DATA_ITEM>*>(&d);
					vec3f f; pf->eval(i, &f);
					fprintf(fp, "%g,%g,%g", f.x, f.y, f.z);
				}
				break;
				case DATA_MAT3FS:
				{
					FEElemData_T<mat3fs, DATA_ITEM>* pf = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>*>(&d);
					mat3fs f; pf->eval(i, &f);
					fprintf(fp, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz);
				}
				break;
				}
			}
			else if (fmt == DATA_COMP)
			{
				int nn = el.Nodes();

				switch (d.GetType())
				{
				case DATA_FLOAT:
				{
					FEElemData_T<float, DATA_COMP>* pf = dynamic_cast<FEElemData_T<float, DATA_COMP>*>(&d);
					float v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
					float f = 0.0f;
					for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
					fprintf(fp, "%g", f);
				}
				break;
				case DATA_VEC3F:
				{
					FEElemData_T<vec3f, DATA_COMP>* pf = dynamic_cast<FEElemData_T<vec3f, DATA_COMP>*>(&d);
					vec3f v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
					vec3f f(0.f, 0.f, 0.f);
					for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
					fprintf(fp, "%g,%g,%g", f.x, f.y, f.z);
				}
				break;
				case DATA_MAT3FS:
				{
					FEElemData_T<mat3fs, DATA_COMP>* pf = dynamic_cast<FEElemData_T<mat3fs, DATA_COMP>*>(&d);
					mat3fs v[FEGenericElement::MAX_NODES]; pf->eval(i, v);
					mat3fs f(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
					for (int i = 0; i<nn; ++i) f += v[i]; f /= (float)nn;
					fprintf(fp, "%g,%g,%g,%g,%g,%g", f.x, f.y, f.z, f.xy, f.yz, f.xz);
				}
				break;
				}
			}
			if (n != nstates - 1) fprintf(fp, ",");
		}
		fprintf(fp, "\n");
	}

	return true;
}

//------------------------------------------------------------------------------------------
// NOTE: the ndata relates to the index in DataPanel::on_AddStandard_triggered
// TODO: Find a better mechanism
bool Post::AddStandardDataField(CGLModel& glm, int ndata, bool bselection_only)
{
	FEDataField* pdf = 0;
	switch (ndata)
	{
	case  0: pdf = new FEDataField_T<FENodePosition         >("Position"); break;
	case  1: pdf = new FEDataField_T<FENodeInitPos          >("Initial position"); break;
	case  2: pdf = new FEDataField_T<FEDeformationGradient  >("Deformation gradient"); break;
	case  3: pdf = new FEStrainDataField("Infinitesimal strain", FEStrainDataField::INF_STRAIN); break;
	case  4: pdf = new FEStrainDataField("Lagrange strain", FEStrainDataField::LAGRANGE); break;
	case  5: pdf = new FEStrainDataField("Right Cauchy-Green", FEStrainDataField::RIGHT_CAUCHY_GREEN); break;
	case  6: pdf = new FEStrainDataField("Right stretch", FEStrainDataField::RIGHT_STRETCH); break;
	case  7: pdf = new FEStrainDataField("Biot strain", FEStrainDataField::BIOT); break;
	case  8: pdf = new FEStrainDataField("Right Hencky", FEStrainDataField::RIGHT_HENCKY); break;
	case  9: pdf = new FEStrainDataField("Left Cauchy-Green", FEStrainDataField::LEFT_CAUCHY_GREEN); break;
	case 10: pdf = new FEStrainDataField("Left stretch", FEStrainDataField::LEFT_STRETCH); break;
	case 11: pdf = new FEStrainDataField("Left Hencky", FEStrainDataField::LEFT_HENCKY); break;
	case 12: pdf = new FEStrainDataField("Almansi strain", FEStrainDataField::ALMANSI); break;
	case 13: pdf = new FEDataField_T<FEElementVolume        >("Volume"); break;
	case 14: pdf = new FEDataField_T<FEVolRatio             >("Volume ratio"); break;
	case 15: pdf = new FEDataField_T<FEVolStrain            >("Volume strain"); break;
	case 16: pdf = new FEDataField_T<FEAspectRatio          >("Aspect ratio"); break;
	case 17: pdf = new FECurvatureField("1-Princ curvature", FECurvatureField::PRINC1_CURVATURE); break;
	case 18: pdf = new FECurvatureField("2-Princ curvature", FECurvatureField::PRINC2_CURVATURE); break;
	case 19: pdf = new FECurvatureField("Gaussian curvature", FECurvatureField::GAUSS_CURVATURE); break;
	case 20: pdf = new FECurvatureField("Mean curvature", FECurvatureField::MEAN_CURVATURE); break;
	case 21: pdf = new FECurvatureField("RMS curvature", FECurvatureField::RMS_CURVATURE); break;
	case 22: pdf = new FECurvatureField("Princ curvature difference", FECurvatureField::DIFF_CURVATURE); break;
	case 23: pdf = new FEDataField_T<FECongruency           >("Congruency"); break;
	case 24: pdf = new FEDataField_T<FEPrincCurvatureVector1>("1-Princ curvature vector"); break;
	case 25: pdf = new FEDataField_T<FEPrincCurvatureVector2>("2-Princ curvature vector"); break;
	default:
		return false;
	}

	FEModel& fem = *glm.GetFEModel();

	// NOTE: This only works with curvatures
	if (bselection_only && (glm.GetSelectionMode() == SELECT_FACES))
	{
		vector<int> L;
		glm.GetSelectionList(L, glm.GetSelectionMode());
		if (L.empty() == false) fem.AddDataField(pdf, L);
		else fem.AddDataField(pdf);
	}
	else fem.AddDataField(pdf);

	return true;
}

//------------------------------------------------------------------------------------------
bool Post::AddNodeDataFromFile(CGLModel& glm, const char* szfile, const char* szname, int ntype)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// get the mesh
	FEModel* pm = glm.GetFEModel();
	FEMeshBase& m = *glm.GetActiveMesh();

	// create a new data field
	int ND = 0;
	switch (ntype)
	{
	case DATA_FLOAT: pm->AddDataField(new FEDataField_T<FENodeData<float  > >(szname, EXPORT_DATA)); ND = 1; break;
	case DATA_VEC3F: pm->AddDataField(new FEDataField_T<FENodeData<vec3f  > >(szname, EXPORT_DATA)); ND = 3; break;
	case DATA_MAT3D: pm->AddDataField(new FEDataField_T<FENodeData<Mat3d  > >(szname, EXPORT_DATA)); ND = 9; break;
	case DATA_MAT3F: pm->AddDataField(new FEDataField_T<FENodeData<mat3f  > >(szname, EXPORT_DATA)); ND = 9; break;
	case DATA_MAT3FS: pm->AddDataField(new FEDataField_T<FENodeData<mat3fs > >(szname, EXPORT_DATA)); ND = 6; break;
	case DATA_MAT3FD: pm->AddDataField(new FEDataField_T<FENodeData<mat3fd > >(szname, EXPORT_DATA)); ND = 3; break;
	case DATA_TENS4FS: pm->AddDataField(new FEDataField_T<FENodeData<tens4fs> >(szname, EXPORT_DATA)); ND = 21; break;
	default:
		assert(false);
		fclose(fp);
		return false;
	}

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
			while (ch && (nstate < pm->GetStates()))
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
				FEState* ps = pm->GetState(nstate);

				int ndf = ps->m_Data.size();

				// get the datafield
				switch (ntype)
				{
				case DATA_FLOAT:
				{
					FENodeData<float>& df = dynamic_cast<FENodeData<float>&>(ps->m_Data[ndf - 1]);
					df[node] = f[0];
				}
				break;
				case DATA_VEC3F:
				{
					FENodeData<vec3f>& df = dynamic_cast<FENodeData<vec3f>&>(ps->m_Data[ndf - 1]);
					df[node] = vec3f(f[0], f[1], f[2]);
				}
				break;
				case DATA_MAT3D:
				{
					FENodeData<Mat3d>& df = dynamic_cast<FENodeData<Mat3d>&>(ps->m_Data[ndf - 1]);
					df[node] = Mat3d(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]);
				}
				break;
				case DATA_MAT3F:
				{
					FENodeData<mat3f>& df = dynamic_cast<FENodeData<mat3f>&>(ps->m_Data[ndf - 1]);
					df[node] = mat3f(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]);
				}
				break;
				case DATA_MAT3FS:
				{
					FENodeData<mat3fs>& df = dynamic_cast<FENodeData<mat3fs>&>(ps->m_Data[ndf - 1]);
					df[node] = mat3fs(f[0], f[1], f[2], f[3], f[4], f[5]);
				}
				break;
				case DATA_MAT3FD:
				{
					FENodeData<mat3fd>& df = dynamic_cast<FENodeData<mat3fd>&>(ps->m_Data[ndf - 1]);
					df[node] = mat3fd(f[0], f[1], f[2]);
				}
				break;
				case DATA_TENS4FS:
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
bool Post::AddFaceDataFromFile(CGLModel& glm, const char* szfile, const char* szname, int ntype)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// get the mesh
	FEModel* pm = glm.GetFEModel();
	FEMeshBase& m = *glm.GetActiveMesh();

	// create a new data field
	int ND = 0;
	switch (ntype)
	{
	case DATA_FLOAT: pm->AddDataField(new FEDataField_T<FEFaceData<float, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 1; break;
	case DATA_VEC3F: pm->AddDataField(new FEDataField_T<FEFaceData<vec3f, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 3; break;
	case DATA_MAT3F: pm->AddDataField(new FEDataField_T<FEFaceData<mat3f, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 9; break;
	case DATA_MAT3D: pm->AddDataField(new FEDataField_T<FEFaceData<Mat3d, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 9; break;
	case DATA_MAT3FS: pm->AddDataField(new FEDataField_T<FEFaceData<mat3fs, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 6; break;
	case DATA_MAT3FD: pm->AddDataField(new FEDataField_T<FEFaceData<mat3fd, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 3; break;
	case DATA_TENS4FS: pm->AddDataField(new FEDataField_T<FEFaceData<tens4fs, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 21; break;
	default:
		assert(false);
		fclose(fp);
		return false;
	}

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
			while (ch && (nstate < pm->GetStates()))
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
				FEState* ps = pm->GetState(nstate);

				int ndf = ps->m_Data.size();

				// get the datafield
				switch (ntype)
				{
				case DATA_FLOAT:
				{
					FEFaceData<float, DATA_ITEM>& df = dynamic_cast<FEFaceData<float, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, f[0]);
				}
				break;
				case DATA_VEC3F:
				{
					FEFaceData<vec3f, DATA_ITEM>& df = dynamic_cast<FEFaceData<vec3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, vec3f(f[0], f[1], f[2]));
				}
				break;
				case DATA_MAT3F:
				{
					FEFaceData<mat3f, DATA_ITEM>& df = dynamic_cast<FEFaceData<mat3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, mat3f(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]));
				}
				break;
				case DATA_MAT3D:
				{
					FEFaceData<Mat3d, DATA_ITEM>& df = dynamic_cast<FEFaceData<Mat3d, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, Mat3d(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]));
				}
				break;
				case DATA_MAT3FS:
				{
					FEFaceData<mat3fs, DATA_ITEM>& df = dynamic_cast<FEFaceData<mat3fs, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, mat3fs(f[0], f[1], f[2], f[3], f[4], f[5]));
				}
				break;
				case DATA_MAT3FD:
				{
					FEFaceData<mat3fd, DATA_ITEM>& df = dynamic_cast<FEFaceData<mat3fd, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nface, mat3fd(f[0], f[1], f[2]));
				}
				break;
				case DATA_TENS4FS:
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
bool Post::AddElemDataFromFile(CGLModel& glm, const char* szfile, const char* szname, int ntype)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// get the mesh
	FEModel* pm = glm.GetFEModel();
	FEMeshBase& m = *glm.GetActiveMesh();

	// create a new data field
	int ND = 0;
	switch (ntype)
	{
	case DATA_FLOAT: pm->AddDataField(new FEDataField_T<FEElementData<float, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 1; break;
	case DATA_VEC3F: pm->AddDataField(new FEDataField_T<FEElementData<vec3f, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 3; break;
	case DATA_MAT3F: pm->AddDataField(new FEDataField_T<FEElementData<mat3f, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 9; break;
	case DATA_MAT3D: pm->AddDataField(new FEDataField_T<FEElementData<Mat3d, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 9; break;
	case DATA_MAT3FS: pm->AddDataField(new FEDataField_T<FEElementData<mat3fs, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 6; break;
	case DATA_MAT3FD: pm->AddDataField(new FEDataField_T<FEElementData<mat3fd, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 3; break;
	case DATA_TENS4FS: pm->AddDataField(new FEDataField_T<FEElementData<tens4fs, DATA_ITEM> >(szname, EXPORT_DATA)); ND = 21; break;
	default:
		assert(false);
		fclose(fp);
		return false;
	}

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
			while (ch && (nstate < pm->GetStates()))
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
				FEState* ps = pm->GetState(nstate);

				int ndf = ps->m_Data.size();

				// get the datafield
				switch (ntype)
				{
				case DATA_FLOAT:
				{
					FEElementData<float, DATA_ITEM>& df = dynamic_cast<FEElementData<float, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, f[0]);
				}
				break;
				case DATA_VEC3F:
				{
					FEElementData<vec3f, DATA_ITEM>& df = dynamic_cast<FEElementData<vec3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, vec3f(f[0], f[1], f[2]));
				}
				break;
				case DATA_MAT3F:
				{
					FEElementData<mat3f, DATA_ITEM>& df = dynamic_cast<FEElementData<mat3f, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, mat3f(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]));
				}
				break;
				case DATA_MAT3D:
				{
					FEElementData<Mat3d, DATA_ITEM>& df = dynamic_cast<FEElementData<Mat3d, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, Mat3d(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]));
				}
				break;
				case DATA_MAT3FS:
				{
					FEElementData<mat3fs, DATA_ITEM>& df = dynamic_cast<FEElementData<mat3fs, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, mat3fs(f[0], f[1], f[2], f[3], f[4], f[5]));
				}
				break;
				case DATA_MAT3FD:
				{
					FEElementData<mat3fd, DATA_ITEM>& df = dynamic_cast<FEElementData<mat3fd, DATA_ITEM>&>(ps->m_Data[ndf - 1]);
					df.add(nelem, mat3fd(f[0], f[1], f[2]));
				}
				break;
				case DATA_TENS4FS:
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
