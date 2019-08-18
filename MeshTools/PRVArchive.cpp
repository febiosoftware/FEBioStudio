#include "stdafx.h"
#include "PRVArchive.h"

PRVArchive::PRVArchive()
{ 
	m_nc = 0; 
}

bool PRVArchive::Load(const char* szfile)
{
	// try to open the file
	if (Open(szfile, "rb") == false) return false;

	// open the archive        P R V
	return m_ar.Open(m_fp, 0x00505256);
}

void PRVArchive::Close()
{
	m_ar.Close();
	FileReader::Close();
}
