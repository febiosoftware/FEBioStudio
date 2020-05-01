#include "stdafx.h"
#include "ModelFileReader.h"
#include "ModelDocument.h"
#include <GeomLib/GObject.h>
#include <sstream>

ModelFileReader::ModelFileReader(CModelDocument* doc)
{
	m_doc = doc;
}

bool ModelFileReader::Load(const char* szfile)
{
	if (!PRVArchive::Load(szfile))
	{
		return errf("Failed opening file");
	}
	else
	{
		try
		{
			m_doc->SetDocFilePath(szfile);
			m_doc->Load(GetArchive());
			return true;
		}
		catch (InvalidVersion)
		{
			return errf("This file has an invalid version number.");
		}
		catch (ReadError e)
		{
			char* sz = 0;
			int L = CCallStack::GetCallStackString(0);
			sz = new char[L + 1];
			CCallStack::GetCallStackString(sz);

			stringstream ss;
			ss << "An error occurred while reading the file:\n" << szfile << "\nCall stack:" << sz;
			delete[] sz;

			string errMsg = ss.str();

			return errf(errMsg.c_str());
		}
		catch (GObjectException e)
		{
			return errf("An error occurred processing model:\n\s", e.ErrorMsg());
		}
		catch (...)
		{
			return errf("Failed opening file \s1.", szfile);
		}
	}

	return false;
}
