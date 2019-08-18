#pragma once
#include <MeshIO/FileReader.h>
#include "Archive.h"

//-----------------------------------------------------------------------------
class FEModel;

//-----------------------------------------------------------------------------
// The PreView archive
class PRVArchive : public FileReader
{
public:
	PRVArchive();

	// This doesn't actually load the model, but initializes the IArchive
	// Call the GetArchive next, and use that to do further processing
	bool Load(const char* szfile);

	IArchive& GetArchive() { return m_ar; }

	void Close();

private:
	FEModel*	m_pfem;
	int			m_nc;	// counter
	IArchive	m_ar;	// the archive
};
