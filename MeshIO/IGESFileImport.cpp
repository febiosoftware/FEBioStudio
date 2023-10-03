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

#include "IGESFileImport.h"

//-----------------------------------------------------------------------------

IGESFileImport::IGESFileImport(FSProject& prj) : FSFileImport(prj)
{
}

IGESFileImport::~IGESFileImport()
{
}

//-----------------------------------------------------------------------------

bool IGESFileImport::read_record(IGESFileImport::RECORD& rec)
{
	// make sure the file is open
	// we check with an assert since the file should
	// always be open when calling this function
	assert(m_fp);

	// allocate a buffer to store a line
	static char szline[82] = {0};

	// read the line from the file
	if (fgets(szline, 81, m_fp) == 0) return false;

	// parse the line
	strncpy(rec.szdata, szline, 72);

	switch (szline[72])
	{
	case 'F': rec.nsec = IGS_FLAG; break;
	case 'S': rec.nsec = IGS_START; break;
	case 'G': rec.nsec = IGS_GLOBAL; break;
	case 'D': rec.nsec = IGS_DIRECTORY; break;
	case 'P': rec.nsec = IGS_PARAMETER; break;
	case 'T': rec.nsec = IGS_TERMINATE; break;
	default:
		assert(false);
		return false;
	}

	rec.nnum = atoi(szline+73);

	return true;
}

//-----------------------------------------------------------------------------

bool IGESFileImport::Load(const char *szfile)
{
	FSModel&fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// open the file
	if (!Open(szfile, "rt")) return false;

	// read the first line
	RECORD rec;
	read_record(rec);

	// make sure it is not the flag section
	if (rec.nsec != IGS_START) return false;

	// read the start section
	if (!ReadStartSection(rec)) return false;

	// read the global section
	if (!ReadGlobalSection(rec)) return false;

	// read the directory section
	if (!ReadDirectorySection(rec)) return false;

	// read parameter data section
	if (!ReadParameterSection(rec)) return false;

	// read terminate section
	if (!ReadTerminateSection(rec)) return false;


	// close the file
	Close();

	return true;
}

//-----------------------------------------------------------------------------

bool IGESFileImport::ReadStartSection(IGESFileImport::RECORD& rec)
{
	if (rec.nsec != IGS_START) return false;

	// we ignore this section for now
	do
	{
		// read the next record
		if (read_record(rec) == false) return false;
	}
	while (rec.nsec == IGS_START);

	return true;
}

//-----------------------------------------------------------------------------

bool IGESFileImport::ReadGlobalSection(IGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_GLOBAL) return false;

	// we ignore this section for now
	do
	{
		// read the next record
		if (read_record(rec) == false) return false;
	}
	while (rec.nsec == IGS_GLOBAL);

	return true;
}

//-----------------------------------------------------------------------------

bool IGESFileImport::ReadDirectorySection(IGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_DIRECTORY) return false;

	return true;
}

//-----------------------------------------------------------------------------

bool IGESFileImport::ReadParameterSection(IGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_PARAMETER) return false;

	return true;
}

//-----------------------------------------------------------------------------

bool IGESFileImport::ReadTerminateSection(IGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_TERMINATE) return false;

	return true;
}
