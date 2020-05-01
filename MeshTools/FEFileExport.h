#pragma once
#include <MeshIO/FileWriter.h>
#include <string>

class FEProject;

class FEFileExport : public FileWriter
{
public:
	FEFileExport(FEProject& prj);
	virtual ~FEFileExport();

	void ClearLog();

	// return the error message
	std::string GetErrorMessage() { return m_err; }

protected:
	bool errf(const char* szerr, ...);

protected:
	std::string	m_err;	// error message
	FEProject&	m_prj;
};
