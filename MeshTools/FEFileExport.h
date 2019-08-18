// FEFileExport.h: interface for the FEFileExport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEFILEEXPORT_H__B0E95434_4F7B_42A9_B6B4_29C1B4E11430__INCLUDED_)
#define AFX_FEFILEEXPORT_H__B0E95434_4F7B_42A9_B6B4_29C1B4E11430__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#endif // !defined(AFX_FEFILEEXPORT_H__B0E95434_4F7B_42A9_B6B4_29C1B4E11430__INCLUDED_)
