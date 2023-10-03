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
#include <MeshIO/FSFileImport.h>
#include <FEMLib/FSProject.h>

class IGESFileImport : public FSFileImport
{
protected:
	enum SECTION_CODE
	{
		IGS_FLAG,
		IGS_START,
		IGS_GLOBAL,
		IGS_DIRECTORY,
		IGS_PARAMETER,
		IGS_TERMINATE
	};

	struct RECORD
	{
		char	szdata[73];	// data field
		int		nsec;		// section identifier
		int		nnum;		// sequence number
	};

	struct DIRECTORY_ENTRY
	{
		int		ntype;		// entity type number
		int		pdata;		// pointer to parameter data
		int		pstruct;	// pointer to defining structure
		int		nfont;		// line font pattern
		int		nlevel;		// number of level
		int		pview;		// pointer to view
		int		ptrans;		// pointer to transformation matrix
		int		pdisplay;	// pointer to label display entitiy
		int		nstatus;	// status number
		int		nweight;	// line thickness
		int		ncolor;		// color number
		int		ncount;		// line count in parameter section
		int		nform;		// form number
		int		nlabel;		// label number		
	};

public:
	IGESFileImport(FSProject& prj);
	~IGESFileImport(void);

	bool Load(const char* szfile);

protected:
	bool read_record(RECORD& rec);

	bool ReadStartSection    (RECORD& rec);
	bool ReadGlobalSection   (RECORD& rec);
	bool ReadDirectorySection(RECORD& rec);
	bool ReadParameterSection(RECORD& rec);
	bool ReadTerminateSection(RECORD& rec);

protected:
	FSModel*	m_pfem;
};
