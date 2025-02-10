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
#include "FEBioExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>
#include <sstream>
#include <iomanip>

//=============================================================================

template <> std::string type_to_string<vec2i>(const vec2i& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y;
	return ss.str();
}

template <> std::string type_to_string<vec2f>(const vec2f& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y;
	return ss.str();
}

template <> std::string type_to_string<vec2d>(const vec2d& v)
{
	std::stringstream ss;
	ss << v.x() << "," << v.y();
	return ss.str();
}

template <> std::string type_to_string<vec3f>(const vec3f& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y << "," << v.z;
	return ss.str();
}

template <> std::string type_to_string<vec3d>(const vec3d& v)
{
	std::stringstream ss;
	ss << std::setprecision(9) << v.x << "," << v.y << "," << v.z;
	return ss.str();
}

template <> std::string type_to_string<quatd>(const quatd& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y << "," << v.z << "," << v.w;
	return ss.str();
}

template <> std::string type_to_string<mat3ds>(const mat3ds& v)
{
	std::stringstream ss;
	ss << v.xx() << "," << v.yy() << "," << v.zz() << "," << v.xy() << "," << v.yz() << "," << v.xz();
	return ss.str();
}

template <> std::string type_to_string<mat3d>(const mat3d& v)
{
	std::stringstream ss;
	ss << v[0][0] << "," << v[0][1] << "," << v[0][2] << ",";
	ss << v[1][0] << "," << v[1][1] << "," << v[1][2] << ",";
	ss << v[2][0] << "," << v[2][1] << "," << v[2][2];
	return ss.str();
}

//=============================================================================
FEBioExport::FEBioExport(FSProject& prj) : FSFileExport(prj)
{
	m_compress = false;
	m_exportSelections = false;
	m_exportEnumStrings = false;
	m_exportNonPersistentParams = true;

	// initialize section flags
	m_section = 0xFFFF;
}

void FEBioExport::SetPlotfileCompressionFlag(bool b)
{
	m_compress = b;
}

void FEBioExport::SetExportSelectionsFlag(bool b)
{
	m_exportSelections = b;
}

void FEBioExport::SetSectionFlags(unsigned int flags)
{
	m_section = flags;
}

void FEBioExport::SetSectionFlag(unsigned int n, bool bwrite)
{
	assert(n < FEBIO_MAX_SECTIONS);
	unsigned int flag = 1 << n;
	if (bwrite) m_section |= flag;
	else m_section &= ~flag;
}

bool FEBioExport::WriteSection(unsigned int n)
{
	assert(n < FEBIO_MAX_SECTIONS);
	unsigned int flag = 1 << n;
	return (m_section & flag);
}

//-----------------------------------------------------------------------------
XMLWriter& FEBioExport::GetXMLWriter()
{
	return m_xml;
}

//-----------------------------------------------------------------------------
const char* FEBioExport::GetEnumKey(Param& p)
{
	assert((p.GetParamType() == Param_CHOICE) || (p.GetParamType() == Param_INT));
	FSModel& fem = m_prj.GetFSModel();
	return fem.GetEnumKey(p, false);
}

//-----------------------------------------------------------------------------
void FEBioExport::WriteParam(Param &p)
{
	// don't export hidden parameters
	if (p.IsReadWrite() == false) return;

	// see if we are writing non-persistent parameters
	if ((m_exportNonPersistentParams==false) && (p.IsPersistent() == false)) return;

	// if parameter is checkable, we only write it when it's checked
	if (p.IsCheckable() && !p.IsChecked()) return;

	// get the (short) name
	const char* szname = p.GetShortName();

	// get the index data (if any)
	const char* szindex = p.GetIndexName();
	int nindex = p.GetIndexValue();

	FSModel& fem = m_prj.GetFSModel();

	// setup the xml-element
	XMLElement e;
	e.name(szname);
	if (szindex) e.add_attribute(szindex, nindex);
	int lc = GetLC(&p);
	if (lc > 0) e.add_attribute("lc", lc);
	switch (p.GetParamType())
	{
	case Param_INT:
	case Param_CHOICE: 
		{
			if (p.GetEnumNames())
			{
				if (m_exportEnumStrings)
				{
					const char* sz = GetEnumKey(p);
					if (sz == nullptr) {
						errf("Invalid key for parameter %s", p.GetShortName());
						sz = "(invalid)";
					}
					e.value(sz);
				}
				else
				{
					int n = fem.GetEnumValue(p);
					e.value(n);
				}
			}
			else
			{
				int n = p.GetIntValue() + p.GetOffset();
				e.value(n); 
			}
		}
		break;
//	case Param_INT   : e.value(p.GetIntValue  ()); break;
	case Param_FLOAT : e.value(p.GetFloatValue()); break;
	case Param_BOOL  : e.value(p.GetBoolValue ()); break;
	case Param_VEC3D : e.value(p.GetVec3dValue()); break;
	case Param_VEC2I : e.value(p.GetVec2iValue()); break;
	case Param_VEC2D : e.value(p.GetVec2dValue()); break;
	case Param_MAT3D : e.value(p.GetMat3dValue()); break;
	case Param_MAT3DS : e.value(p.GetMat3dsValue()); break;
	case Param_STD_VECTOR_INT:
	{
		if (p.GetEnumNames())
		{
			std::vector<int> l = p.GetVectorIntValue();
			char buf[256] = { 0 };
			if (fem.GetEnumValues(buf, l, p.GetEnumNames()))
			{
				e.value(buf);
			}
			else
			{
				std::stringstream ss;
				for (int i = 0; i < l.size(); ++i)
				{
					if (i != 0) ss << ",";
					const char* sz = p.GetEnumName(l[i]); assert(sz);
					if (sz) ss << sz;
				}
				string s = ss.str();
				e.value(s.c_str());
			}
		}
		else e.value(p.GetVectorIntValue());
	}
	break;
	case Param_STD_VECTOR_DOUBLE:
	{
		e.value(p.GetVectorDoubleValue());
	}
	break;
	case Param_ARRAY_INT:
	{
		e.value(p.GetArrayIntValue());
	}
	break;
	case Param_ARRAY_DOUBLE:
	{
		e.value(p.GetArrayDoubleValue());
	}
	break;
	case Param_MATH  :
	{
		e.add_attribute("type", "math");
		std::string smath = p.GetMathString();
		e.value(smath.c_str());
	}
	break;
	case Param_STRING:
	{
		if (p.IsVariable())
		{
			e.add_attribute("type", "map");
		}
		std::string s = p.GetStringValue();
		e.value(s.c_str());
	}
	break;
	case Param_STD_VECTOR_VEC2D:
	{
		std::vector<vec2d> v = p.GetVectorVec2dValue();
		m_xml.add_branch(p.GetShortName());
		for (int i = 0; i < v.size(); ++i)
		{
			vec2d& p = v[i];
			double d[2] = { p.x(), p.y() };
			m_xml.add_leaf("pt", d, 2);
		}
		m_xml.close_branch();
		return;
	}
	break;
    default:
		assert(false);
	}

	// write the xml-element
	m_xml.add_leaf(e);
}

//-----------------------------------------------------------------------------
void FEBioExport::WriteParamList(ParamContainer& c)
{
	int NP = c.Parameters();
	for (int i = 0; i < NP; ++i)
	{
		Param& p = c.GetParam(i);
		// don't write attribute params
		if ((p.GetFlags() & FS_PARAM_ATTRIBUTE) == 0)
		{
			// don't write watch variables
			if ((p.GetFlags() & FS_PARAM_WATCH) == 0)
			{
				// Watched variables are only written if the corresponding
				// watch variable is true. 
				if ((p.IsWatched() == false) || (p.GetWatchFlag()))
					WriteParam(p);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport::WriteNote(FSObject* po)
{
	if (po)
	{
		m_xml.add_comment(po->GetInfo());
	}
}

//-----------------------------------------------------------------------------
void FEBioExport::Clear()
{
	m_LCT.clear();
}

//-----------------------------------------------------------------------------
bool FEBioExport::PrepareExport(FSProject& prj)
{
	Clear();

	// get the model
	FSModel& fem = prj.GetFSModel();
	GModel& model = fem.GetModel();

	// make sure all objects are meshed
	for (int i = 0; i < model.Objects(); ++i)
	{
		FSCoreMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == nullptr) return errf("Not all objects are meshed.");
	}

	// verify nodal IDs
	int nextID = 1;
	for (int i = 0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		int N = pm->Nodes();
		for (int j = 0; j < N; ++j)
		{
			FSNode& node = pm->Node(j);
			if ((node.m_nid == -1) || (node.m_nid < nextID)) node.m_nid = nextID++;
			else if (node.m_nid > nextID) nextID = node.m_nid + 1;
			else nextID = node.m_nid + 1;
		}
		pm->BuildNLT();
	}

	// verify element IDs
	nextID = 1;
	for (int i = 0; i < model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		int NE = pm->Elements();
		for (int j = 0; j < NE; ++j)
		{
			FSElement& el = pm->Element(j);
			if ((el.m_nid == -1) || (el.m_nid < nextID)) el.m_nid = nextID++;
			else if (el.m_nid > nextID) nextID = el.m_nid + 1;
			else nextID = el.m_nid + 1;
		}
		pm->BuildELT();
	}

	// set material tags
	for (int i = 0; i<fem.Materials(); ++i) fem.GetMaterial(i)->m_ntag = i + 1;

	// build load controller table
	m_LCT.clear();
	for (int i = 0; i < fem.LoadControllers(); ++i)
	{
		FSLoadController* plc = fem.GetLoadController(i);
		m_LCT[plc->GetID()] = i + 1;
	}

	return true;
}

//-----------------------------------------------------------------------------
int	FEBioExport::GetLC(const Param* p)
{
	assert(p);
	if (p == nullptr) return -1;
	int lcid = p->GetLoadCurveID();
	if (lcid < 0) return -1;
	assert(m_LCT.find(lcid) != m_LCT.end());
	return m_LCT[lcid];
}
