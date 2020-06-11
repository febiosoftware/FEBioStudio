/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
			return errf("An error occurred processing model:\n%s", e.ErrorMsg());
		}
		catch (...)
		{
			return errf("Failed opening file %s", szfile);
		}
	}

	return false;
}
