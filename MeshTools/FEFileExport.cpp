// FEFileExport.cpp: implementation of the FEFileExport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEFileExport.h"
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEFileExport::FEFileExport(FEProject& prj) : m_prj(prj)
{

}

FEFileExport::~FEFileExport()
{

}

void FEFileExport::ClearLog()
{
	m_err.clear();
}

bool FEFileExport::errf(const char* szerr, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char szmsg[256] = {0};
	va_start(args, szerr);
	vsprintf(szmsg, szerr, args);
	va_end(args);

	m_err += szmsg;
	m_err += "\n";

	return false;
}
