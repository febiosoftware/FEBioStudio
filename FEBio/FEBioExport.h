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
#include <MeshIO/FSFileExport.h>
#include <XML/XMLWriter.h>
#include "FEBioException.h"
#include <FEMLib/FSProject.h>
#include <sstream>
#include <map>

// export sections
enum FEBioExportSections
{
	FEBIO_MODULE,
	FEBIO_CONTROL,
	FEBIO_GLOBAL,
	FEBIO_MATERIAL,
	FEBIO_GEOMETRY,
	FEBIO_MESHDATA,			// new in FEBio 2.5
	FEBIO_MESHADAPTOR,		// new in FEBio 4.0
	FEBIO_INITIAL,
	FEBIO_BOUNDARY,
	FEBIO_LOADS,
	FEBIO_CONSTRAINTS,
	FEBIO_CONTACT,			// new in FEBio 2.0
	FEBIO_DISCRETE,			// new in FEBio 2.0
	FEBIO_STEPS,
	FEBIO_LOADDATA,
	FEBIO_OUTPUT,
	FEBIO_MAX_SECTIONS		// = max nr of sections
};

class FEBioExport : public FSFileExport
{
public:
	FEBioExport(FSProject& prj);

	void SetPlotfileCompressionFlag(bool b);
	void SetExportSelectionsFlag(bool b);

protected:
	void WriteParam(Param& p);
	void WriteParamList(ParamContainer& c);

	virtual void Clear();

	virtual bool PrepareExport(FSProject& prj);

	void WriteNote(FSObject* po);

	const char* GetEnumKey(Param& p);

protected:
	int	GetLC(const Param* p);

public: // helper functions for writing to the xml file directly
	XMLWriter& GetXMLWriter();

protected:
	XMLWriter		m_xml;

	bool	m_section[FEBIO_MAX_SECTIONS];	//!< write section flags

	bool	m_compress;				//!< compress plot file
	bool	m_exportSelections;		//!< export named selections as well
	bool	m_exportEnumStrings;	//!< export enums as strings (otherwise output numbers)

	bool	m_exportNonPersistentParams;

	std::map<int, int>	m_LCT;	// load-controller map: key = load curve ID, value = lc
};

template <> std::string type_to_string<vec2i>(const vec2i& v);
template <> std::string type_to_string<vec2f>(const vec2f& v);
template <> std::string type_to_string<vec2d>(const vec2d& v);
template <> std::string type_to_string<vec3f>(const vec3f& v);
template <> std::string type_to_string<vec3d>(const vec3d& v);
template <> std::string type_to_string<quatd>(const quatd& v);
template <> std::string type_to_string<mat3ds>(const mat3ds& v);
template <> std::string type_to_string<mat3d>(const mat3d& v);
