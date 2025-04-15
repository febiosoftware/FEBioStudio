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
#include "FEBioImport.h"
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEBioLink/FEBioModule.h>
#include <GeomLib/GModel.h>
#include "FEBioFormatOld.h"
#include "FEBioFormat12.h"
#include "FEBioFormat2.h"
#include "FEBioFormat25.h"
#include "FEBioFormat3.h"
#include "FEBioFormat4.h"
#include <stdio.h>
#include <string.h>
#include <cstdarg>
#include <vector>
#include <sstream>

//-----------------------------------------------------------------------------
FEBioFileImport::FEBioFileImport(FSProject& prj) : FSFileImport(prj)
{
	m_szlog = 0;
	m_febio = 0;
	m_geomOnly = false;
    m_skipGeom = false;
}

//-----------------------------------------------------------------------------
FEBioFileImport::~FEBioFileImport()
{
	ClearLog();
	delete m_febio;
}

//-----------------------------------------------------------------------------
void FEBioFileImport::SetGeometryOnlyFlag(bool b)
{
	m_geomOnly = b;
}

void FEBioFileImport::SetSkipGeometryFlag(bool b)
{
    m_skipGeom = b;
}

//-----------------------------------------------------------------------------
void FEBioFileImport::ClearLog()
{
	delete [] m_szlog;
	m_szlog = 0;
}

//-----------------------------------------------------------------------------
void FEBioFileImport::AddLogEntry(const char* sz, ...)
{
	if ((sz == 0) || (*sz==0)) return;

	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char* szlog = NULL;

	va_start(args, sz);

	// count how many chars we need to allocate
    va_list argscopy;
	va_copy(argscopy, args);
	int l = vsnprintf(nullptr, 0, sz, argscopy) + 1;
    va_end(argscopy);
	if (l > 1)
	{
		szlog = new char[l]; assert(szlog);
		if (szlog)
		{
			vsnprintf(szlog, l, sz, args);
		}
	}
	va_end(args);
	if (szlog == NULL) return;

	l = (int) strlen(szlog);
	if (l == 0) { delete[] szlog;  return; }

	if (m_szlog == 0)
	{
		m_szlog = new char[l+1];
		strcpy(m_szlog, szlog);
	}
	else
	{
		int l0 = (int)strlen(m_szlog);
		char* sznew = new char[l0 + l + 2];
		sprintf(sznew, "%s\n%s", m_szlog, szlog);
		delete [] m_szlog;
		m_szlog = sznew;
	}

	delete[] szlog;
}

//-----------------------------------------------------------------------------
// helper class for blocking and unblocking FEBio create events.
// this ensure that the block is released upon exit. 
class FEBioEventBlocker
{
public:
	FEBioEventBlocker() { FEBio::BlockCreateEvents(true); }
	~FEBioEventBlocker() { FEBio::BlockCreateEvents(false); }
};

//-----------------------------------------------------------------------------
//  Imports an FEBio input file
//  The actual file is parsed using the XMLReader class.
//
bool FEBioFileImport::Load(const char* szfile)
{
	// set an event blocker
	FEBioEventBlocker blocker;

	// clear the log
	ClearLog();

	// extract the path
	strcpy(m_szpath, szfile);
	char* ch = strrchr(m_szpath, '/');
	if (ch==0)
	{
		ch = strrchr(m_szpath, '\\');
	}
	if (ch != 0) *(++ch) = 0; else m_szpath[0] = 0;

    SetFileName(szfile);

	// Open thefile with the XML reader
	XMLReader xml;
	if (xml.Open(szfile) == false) return errf("This is not a valid FEBio input file");

    // Set the file stream
    SetFileStream(xml.GetFileStream());

	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();

	// create a new FEBioInputModel
	InitLog(this);
	m_febio = new FEBioInputModel(fem);

	// loop over all child tags
	try
	{
		// Find the root element
		XMLTag tag;
		if (xml.FindTag("febio_spec", tag) == false) return errf("This is not a valid FEBio input file");

		mdl.SetInfo(xml.GetLastComment());

		// check the version number of the file (This also allocates the format)
		if (ParseVersion(tag) == false) return errf("Invalid version for febio_spec");

		// parse the file
		if (ReadFile(tag) == false) return false;
	}
	catch (std::runtime_error e)
	{
		SetFileStream(nullptr);
		return errf("FATAL ERROR: %s (line %d)\n", e.what(), xml.GetCurrentLine());
	}
	catch (...)
	{
		SetFileStream(nullptr);
		return errf("FATAL ERROR: unrecoverable error (line %d)\n", xml.GetCurrentLine());
	}

	// if we get here we are good to go!
	if (UpdateFEModel(fem) == false)
	{
		return errf("FATAL ERROR: Failed constructing model.");
	}

	// copy the log to the error string
	const char* szlog = GetLog();
	if (szlog != 0)
	{
		errf(szlog);
	}

	// TODO: This function (Load) can return without reaching this line! Fix this! 
	SetFileStream(nullptr);

	// we're done!
	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFileImport::ReadFile(XMLTag& tag)
{
	// loop over all file sections
	++tag;
	do
	{
		const char* szfrom = tag.AttributeValue("from", true);
		if (szfrom)
		{
			// make sure this tag is a leaf
			if (tag.isleaf() == false) return errf("Included sections may not have child sections.");

			// try to open the included file
			char szinc[1024] = { 0 };
			if (m_szpath[0])
			{
				sprintf(szinc, "%s%s", m_szpath, szfrom);
			}
			else strcpy(szinc, szfrom);

			XMLReader xml2;
			if (xml2.Open(szinc) == false) return errf("Failed opening included file %s", szinc);

			// first, find the febio_tag
			XMLTag tag2;
			if (xml2.FindTag("febio_spec", tag2) == false) return errf("febio_spec tag was not found in included file.");

			// find the section we are looking for
			string path = "febio_spec/" + string(tag.Name());
			if (xml2.FindTag(path.c_str(), tag2) == false) return errf("FATAL ERROR: Couldn't find %s section in file %s.\n\n", tag.Name(), szinc);

			// try to process the section
			if (m_fmt->ParseSection(tag2) == false) return errf("Cannot read included file");

			// okay, done
			++tag;
		}
		else if (tag == "Include")
		{
			// make sure this tag is a leaf
			if (tag.isleaf() == false) return errf("Included sections may not have child sections.");

			// try to open the included file
			char szinc[1024] = { 0 };
			if (m_szpath[0])
			{
				sprintf(szinc, "%s%s", m_szpath, tag.szvalue());
			}
			else strcpy(szinc, szfrom);

			XMLReader xml2;
			if (xml2.Open(szinc) == false) return errf("Failed opening included file %s", szinc);

			// first, find the febio_tag
			XMLTag tag2;
			if (xml2.FindTag("febio_spec", tag2) == false) return errf("febio_spec tag was not found in included file.");

			// Read the file
			try {
				ReadFile(tag2);
			}
			catch (...)
			{
				throw;
			}

			// all done
			++tag;
		}
		else
		{
			if (m_fmt->ParseSection(tag) == false) ParseUnknownTag(tag);
			++tag;
		}
	} 
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFileImport::ParseVersion(XMLTag& tag)
{
	const char* szv = tag.AttributeValue("version");
	int n1, n2;
	int nr = sscanf(szv, "%d.%d", &n1, &n2);
	if (nr != 2) return false;
	if ((n1 < 1) || (n1 > 0xFF)) return false;
	if ((n2 < 0) || (n2 > 0xFF)) return false;
	m_nversion = (n1 << 8) + n2;

	// we only support version 1.1, 1.2, 2.0 and 2.5
	switch (m_nversion)
	{
	case 0x0101: m_fmt = new FEBioFormatOld(this, *m_febio); break;
	case 0x0102: m_fmt = new FEBioFormat12 (this, *m_febio); break;
	case 0x0200: m_fmt = new FEBioFormat2  (this, *m_febio); break;
	case 0x0205: m_fmt = new FEBioFormat25 (this, *m_febio); break;
	case 0x0300: m_fmt = new FEBioFormat3  (this, *m_febio); break;
	case 0x0400: m_fmt = new FEBioFormat4  (this, *m_febio); break;
	default:
		return false;
	}

	m_fmt->SetGeometryOnlyFlag(m_geomOnly);
    m_fmt->SetSkipGeometryFlag(m_skipGeom);

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFileImport::ParseUnknownTag(XMLTag& tag)
{
	AddLogEntry("Skipping unknown tag \"%s\" (line %d)", tag.Name(), tag.m_nstart_line);
	tag.m_preader->SkipTag(tag);
}

//-----------------------------------------------------------------------------
void FEBioFileImport::ParseUnknownAttribute(XMLTag& tag, const char* szatt)
{
	AddLogEntry("Skipping tag \"%s\". Unknown value for attribute \"%s\". (line %d)", tag.Name(), szatt, tag.m_nstart_line);
	tag.m_preader->SkipTag(tag);
}

//-----------------------------------------------------------------------------
// TODO: Register parameters that require load curves in a list.
// Or store the load curves directly. 
// I think this would eliminate all these dynamic_casts
bool FEBioFileImport::UpdateFEModel(FSModel& fem)
{
	// set the parent ID's for rigid bodies
	int nmat = m_febio->Materials();
	for (int i = 0; i<nmat; ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (pm)
		{
			FSRigidMaterial* pr = dynamic_cast<FSRigidMaterial*>(pm->GetMaterialProperties());
			if (pr && (pr->m_pid != -1))
			{
				int pid = pr->m_pid - 1;
				if ((pid < 0) || (pid >= nmat)) return errf("Invalid material ID for rigid body");
				GMaterial* pp = fem.GetMaterial(pid);
				if ((pp == 0) || (dynamic_cast<FSRigidMaterial*>(pp->GetMaterialProperties()) == 0)) return errf("Invalid material for rigid body parent");
				pr->m_pid = pp->GetID();
			}
		}
	}

	// resolve all load curve references
	int NLC = m_febio->LoadCurves();
	if (NLC > 0)
	{
		assert(fem.LoadControllers() == 0);
		for (int i = 0; i < NLC; ++i)
		{
			fem.AddLoadCurve(m_febio->GetLoadCurve(i));
		}
	}

	int NPC = m_febio->ParamCurves();
	for (int i=0; i<NPC; ++i)
	{
		FEBioInputModel::PARAM_CURVE pc = m_febio->GetParamCurve(i);
		assert(pc.m_lc >= 0);
		assert(pc.m_p || pc.m_plc);
		if ((pc.m_lc >= 0) && (pc.m_lc < fem.LoadControllers()))
		{
			if (pc.m_p)
			{
				FSLoadController* plc = fem.GetLoadController(pc.m_lc);
				if (pc.m_p->GetParamType() == Param_Type::Param_STD_VECTOR_VEC2D)
				{
					// map the points directly to vector
					Param* src = plc->GetParam("points"); assert(src);
					if (src)
					{
						std::vector<vec2d> pt = src->GetVectorVec2dValue();
						pc.m_p->SetVectorVec2dValue(pt);
					}
				}
				else pc.m_p->SetLoadCurveID(plc->GetID());
			}
			if (pc.m_plc) 
			{
				// NOTE: This is only used for reading in must-point curves of older files.
				FEBioLoadController* plc = dynamic_cast<FEBioLoadController*>(fem.GetLoadController(pc.m_lc));
				if (plc)
				{
					LoadCurve* lc = plc->CreateLoadCurve();
					if (lc)
					{
						*pc.m_plc = *lc;
						pc.m_plc->SetID(pc.m_lc);
					}
				}
			}
		}
	}

	// set the GMeshObject's name
	// (only for older versions)
	if ((m_nversion < 0x0205) && (m_febio->Parts() == 1))
	{
		FEBioInputModel::PartInstance& part = *m_febio->GetInstance(0);
		GMeshObject* po = part.GetGObject();
		if (po)
		{
			char szname[256];
			FileTitle(szname);
			po->SetName(szname);
		}
	}

	// update plot variables
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	GModel& mdl = fem.GetModel();
	for (int i=0; i<m_febio->PlotVariables(); ++i)
	{
		FEBioInputModel::PlotVariable& v = m_febio->GetPlotVariable(i);
		std::string name = v.name();

		// try to find it
		CPlotVariable* pv = plt.FindVariable(name);
		if (pv == 0)
		{
			pv = plt.AddPlotVariable(name, true, true, v.domainType());
		}
		pv->setActive(true);
		pv->setShown(true);

		string domain = v.domain();
		if (domain.empty() == false)
		{
			//NOTE: This assumes the domain name is a surface
			FSItemListBuilder* surf = m_febio->FindNamedSurface(domain);
			if (surf)
			{
				pv->addDomain(surf);
			}
			else 
			{
				AddLogEntry("Could not find surface named %s", domain.c_str());
			}
		}
	}

	// add all the parts to the model
	for (int i = 0; i<m_febio->Instances(); ++i)
	{
		// get the next instance
		FEBioInputModel::PartInstance& part = *m_febio->GetInstance(i);

		// Take the GObject from the part (the part no longer keeps a pointer to this object)
		GMeshObject* po = part.TakeGObject();
		if (po == nullptr) return false;

		// pass ownership to the model
		fem.GetModel().AddObject(po);
	}

	// update material part lists
	fem.UpdateMaterialSelections();

	// update log variables
	GModel& model = fem.GetModel();
	GMeshObject* po = 0;
	if (model.Objects() == 1) po = dynamic_cast<GMeshObject*>(model.Object(0));
	CLogDataSettings& log = m_prj.GetLogDataSettings();
	log.ClearLogData();
	for (int i = 0; i<m_febio->LogVariables(); ++i)
	{
		FEBioInputModel::LogVariable& v = m_febio->GetLogVariable(i);

		FSLogData* ld = nullptr;
		switch (v.type())
		{
		case FSLogData::LD_NODE   : ld = new FSLogNodeData(mdl.FindNamedSelection(v.GroupID())); break;
		case FSLogData::LD_FACE   : ld = new FSLogFaceData(mdl.FindNamedSelection(v.GroupID())); break;
		case FSLogData::LD_SURFACE: ld = new FSLogSurfaceData(mdl.FindNamedSelection(v.GroupID())); break;
		case FSLogData::LD_DOMAIN : ld = new FSLogDomainData(mdl.FindNamedSelection(v.GroupID())); break;
		case FSLogData::LD_ELEM   : ld = new FSLogElemData(mdl.FindNamedSelection(v.GroupID())); break;
		case FSLogData::LD_RIGID  : ld = new FSLogRigidData(v.GroupID()); break;
		case FSLogData::LD_CNCTR  : ld = new FSLogConnectorData(v.GroupID()); break;
		}

		if (ld)
		{
			ld->SetDataString(v.data());
			ld->SetFileName(v.file());
			log.AddLogData(ld);
		}
	}

	if (m_nversion < 0x0400)
	{
		// older formats need to be converted
		AddLogEntry("Converting FE model:");
		AddLogEntry("===================");
		std::ostringstream log;
		m_prj.ConvertToNewFormat(log);
		string s = log.str();
		if (s.empty() == false) AddLogEntry(s.c_str());
		else AddLogEntry("No issues found!");
	}

	return true;
}

bool FEBioFileImport::ImportMaterials(const char* szfile)
{
	ClearLog();

	// extract the path
	strcpy(m_szpath, szfile);
	char* ch = strrchr(m_szpath, '/');
	if (ch == 0)
	{
		ch = strrchr(m_szpath, '\\');
	}
	if (ch != 0) *(++ch) = 0; else m_szpath[0] = 0;

	SetFileName(szfile);

	// Open thefile with the XML reader
	XMLReader xml;
	if (xml.Open(szfile) == false) return errf("This is not a valid FEBio input file");

	// Set the file stream
	SetFileStream(xml.GetFileStream());

	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();

	// we may need to convert the new materials, so let's keep track of how many materials there are now.
	int currentMatCount = fem.Materials();

	// create a new FEBioInputModel
	InitLog(this);
	m_febio = new FEBioInputModel(fem);

	// loop over all child tags
	bool bret = true;
	try
	{
		// Find the root element
		XMLTag tag;
		if (xml.FindTag("febio_spec", tag) == false) return errf("This is not a valid FEBio input file");

		mdl.SetInfo(xml.GetLastComment());

		// check the version number of the file (This also allocates the format)
		if (ParseVersion(tag) == false) return errf("Invalid version for febio_spec");

		// first section must be Module
		++tag;
		if (tag != "Module")
		{
			return errf("Module section not found.");
		}
		m_fmt->ParseSection(tag);

		// find the material tag
		if (xml.FindTag("febio_spec/Material", tag) == false)
		{
			return errf("File does not contain Material section.");
		}

		// loop over all file sections
		bret = m_fmt->ParseSection(tag);
	}
	catch (std::runtime_error e)
	{
		SetFileStream(nullptr);
		return errf("FATAL ERROR: %s (line %d)\n", e.what(), xml.GetCurrentLine());
	}
	catch (...)
	{
		SetFileStream(nullptr);
		return errf("FATAL ERROR: unrecoverable error (line %d)\n", xml.GetCurrentLine());
	}

	// copy the log to the error string
	const char* szlog = GetLog();
	if (szlog != 0)
	{
		errf(szlog);
	}

	SetFileStream(nullptr);

	if (m_nversion < 0x0400)
	{
		// older formats need to be converted
		AddLogEntry("Converting materials:");
		std::ostringstream log;
		for (int i = currentMatCount; i < fem.Materials(); ++i)
		{
			GMaterial* mat = fem.GetMaterial(i);
			m_prj.ConvertMaterial(mat, log);
		}
		string s = log.str();
		if (s.empty() == false) AddLogEntry(s.c_str());
		else AddLogEntry("No issues found!");
	}

	// we're done!
	return bret;
}
