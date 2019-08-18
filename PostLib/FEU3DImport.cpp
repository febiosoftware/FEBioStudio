#include "stdafx.h"
#include "FEU3DImport.h"
#include "U3DFile.h"

using namespace Post;

FEU3DImport::FEU3DImport() : FEFileReader("U3D")
{
}

bool FEU3DImport::Load(FEModel& fem, const char* szfile)
{
	if (Open(szfile, "rb") == false) return errf("Failed opening file");

	U3DFile u3d;
	if (u3d.Load(m_fp) == false) return errf("Faileding loading u3d file");

	Close();

	return true;
}
