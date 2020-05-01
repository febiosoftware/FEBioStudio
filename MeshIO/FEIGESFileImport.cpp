#include "FEIGESFileImport.h"

//-----------------------------------------------------------------------------

FEIGESFileImport::FEIGESFileImport(FEProject& prj) : FEFileImport(prj)
{
}

FEIGESFileImport::~FEIGESFileImport()
{
}

//-----------------------------------------------------------------------------

bool FEIGESFileImport::read_record(FEIGESFileImport::RECORD& rec)
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

bool FEIGESFileImport::Load(const char *szfile)
{
	FEModel &fem = m_prj.GetFEModel();
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

bool FEIGESFileImport::ReadStartSection(FEIGESFileImport::RECORD& rec)
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

bool FEIGESFileImport::ReadGlobalSection(FEIGESFileImport::RECORD &rec)
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

bool FEIGESFileImport::ReadDirectorySection(FEIGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_DIRECTORY) return false;

	return true;
}

//-----------------------------------------------------------------------------

bool FEIGESFileImport::ReadParameterSection(FEIGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_PARAMETER) return false;

	return true;
}

//-----------------------------------------------------------------------------

bool FEIGESFileImport::ReadTerminateSection(FEIGESFileImport::RECORD &rec)
{
	if (rec.nsec != IGS_TERMINATE) return false;

	return true;
}
