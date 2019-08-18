#pragma once
#include "FEProject.h"
#include <string>

class FEFileExport  
{
public:
	FEFileExport();
	virtual ~FEFileExport();

	void ClearLog();

	virtual bool Export(FEProject& prj, const char* szfile) = 0;

	// return the error message
	std::string GetErrorMessage() { return m_err; }

protected:
	bool errf(const char* szerr, ...);

protected:
	std::string	m_err;	// error message
};
