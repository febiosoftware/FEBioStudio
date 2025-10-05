/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <FEBioLink/FEBioModule.h>
#include <GeomLib/GObject.h>
#include <sstream>

using std::stringstream;

ModelFileReader::ModelFileReader(CModelDocument* doc)
{
	m_doc = doc;

	m_fileVersion = -1;
}

bool ModelFileReader::Open(const char* szfile)
{
	// try to open the archive
	if (!PRVArchive::Load(szfile)) return false;

	// get the version info
	IArchive& ar = GetArchive();
	IArchive::IOResult nret = IArchive::IO_OK;

	// the version chunk should be the first one!
	if (ar.OpenChunk() != IArchive::IO_OK) return false;
	if (ar.GetChunkID() != CID_VERSION) return false;
	nret = ar.read(m_fileVersion);
	ar.CloseChunk();

	// make sure to inform the archive of the version number
	ar.SetVersion(m_fileVersion);

	return true;
}

int ModelFileReader::GetFileVersion() const
{
	return m_fileVersion;
}

const std::vector<std::pair<int, std::string>>& ModelFileReader::GetMissingPlugins() const
{
    return m_missingPlugins;
}

bool ModelFileReader::Load(const char* szfile)
{
	FEBio::BlockCreateEvents(true);
	bool b = ReadFile(szfile);
	FEBio::BlockCreateEvents(false);
	return b;
}

bool ModelFileReader::ReadFile(const char* szfile)
{
	// get the archive
	IArchive& ar = GetArchive();

	// see if it's already open (which can be since we pull out some info before actually reading the file)
	if (ar.IsValid() == false)
	{
		// try to open it
		if (!PRVArchive::Load(szfile))
		{
			return errf("Failed opening file");
		}
	}

	// now try to read the rest of the file
	try
	{
		m_doc->SetDocFilePath(szfile);
		m_doc->Load(ar);

		std::string log = ar.GetLog();
		if (log.empty() == false)
		{
			errf(log.c_str());
		}

		Close();
		return true;
	}
	catch (InvalidVersion)
	{
		Close();
		return errf("This file has an invalid version number.");
	}
	catch (ReadError e)
	{
		std::string s = CCallStack::GetCallStackString();

		stringstream ss;
		if (e.m_szmsg) { ss << e.m_szmsg << "\n"; }
		else { ss << "(unknown)\n"; };
		ss << "\nCALL STACK:\n" << s;

		string errMsg = ss.str();

		Close();
		return errf(errMsg.c_str());
	}
    catch (MissingPluginError& e)
    {
        m_missingPlugins = e.m_plugins;

        std::string errMsg = "The following plugins are missing:\n";
        for (const auto& plugin : m_missingPlugins)
        {
            errMsg += "ID: " + std::to_string(plugin.first) + ", Name: " + plugin.second + "\n";
        }

        Close();
        return errf(errMsg.c_str());
    }
	catch (std::exception e)
	{
		Close();
		const char* szerr = (e.what() == nullptr ? "(unknown)" : e.what());
		return errf("An exception occurred: %s", szerr);
	}
	catch (...)
	{
		Close();
		return errf("Failed opening file %s", szfile);
	}

	Close();
	return false;
}
