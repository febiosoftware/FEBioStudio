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

// XmlImport.cpp: implementation of the FEBioImport class.
//
//////////////////////////////////////////////////////////////////////

#include "FEBioImport.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <MeshTools/GModel.h>
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
//using namespace std;

extern GLColor col[];

//-----------------------------------------------------------------------------
FEBioImport::FEBioImport(FEProject& prj) : FEFileImport(prj)
{
	m_szlog = 0;
	m_febio = 0;
	m_geomOnly = false;
}

//-----------------------------------------------------------------------------
FEBioImport::~FEBioImport()
{
	ClearLog();
	delete m_febio;
}

//-----------------------------------------------------------------------------
void FEBioImport::SetGeometryOnlyFlag(bool b)
{
	m_geomOnly = b;
}

//-----------------------------------------------------------------------------
void FEBioImport::ClearLog()
{
	delete [] m_szlog;
	m_szlog = 0;
}

//-----------------------------------------------------------------------------
void FEBioImport::AddLogEntry(const char* sz, ...)
{
	if (sz == 0) return;

	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char szlog[256] = {0};
	va_start(args, sz);
	vsprintf(szlog, sz, args);
	va_end(args);

	int l = (int)strlen(szlog);
	if (l == 0) return;

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
}

//-----------------------------------------------------------------------------
//  Imports an FEBio input file
//  The actual file is parsed using the XMLReader class.
//
bool FEBioImport::Load(const char* szfile)
{
	ClearLog();

	// extract the path
	strcpy(m_szpath, szfile);
	char* ch = strrchr(m_szpath, '/');
	if (ch==0)
	{
		ch = strrchr(m_szpath, '\\');
	}
	if (ch != 0) *(++ch) = 0; else m_szpath[0] = 0;

	// open the file
	if (Open(szfile, "rb") == false) return errf("Failed opening file: %s", szfile);

	// Attach the XML reader to the stream
	XMLReader xml;
	if (xml.Attach(m_fp) == false) return errf("This is not a valid FEBio input file");

	FEModel& fem = m_prj.GetFEModel();
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
	catch (XMLReader::EndOfFile e)
	{
		// this is fine. Moving on ...
	}
	catch (std::runtime_error e)
	{
		return errf("FATAL ERROR: %s (line %d)\n", e.what(), xml.GetCurrentLine());
	}
	catch (...)
	{
		return errf("FATAL ERROR: unrecoverable error (line %d)\n", xml.GetCurrentLine());
	}

	// close the XML file
	Close();

	// if we get here we are good to go!
	UpdateFEModel(fem);

	// copy the log to the error string
	const char* szlog = GetLog();
	if (szlog != 0)
	{
		errf(szlog);
	}

	// we're done!
	return true;
}

//-----------------------------------------------------------------------------
bool FEBioImport::ReadFile(XMLTag& tag)
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
			if (xml2.FindTag(tag.Name(), tag2) == false) return errf("FATAL ERROR: Couldn't find %s section in file %s.\n\n", tag.Name(), szinc);

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
			catch (XMLReader::EndOfFile)
			{
				// we catch this, since this will always be thrown. 
				// TODO: I need to fix this. 
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
bool FEBioImport::ParseVersion(XMLTag& tag)
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

	return true;
}

//-----------------------------------------------------------------------------
void FEBioImport::ParseUnknownTag(XMLTag& tag)
{
	AddLogEntry("Skipping unknown tag \"%s\" (line %d)", tag.Name(), tag.m_nstart_line);
	tag.m_preader->SkipTag(tag);
}

//-----------------------------------------------------------------------------
void FEBioImport::ParseUnknownAttribute(XMLTag& tag, const char* szatt)
{
	AddLogEntry("Skipping tag \"%s\". Unknown value for attribute \"%s\". (line %d)", tag.Name(), szatt, tag.m_nstart_line);
	tag.m_preader->SkipTag(tag);
}

//-----------------------------------------------------------------------------
// TODO: Register parameters that require load curves in a list.
// Or store the load curves directly. 
// I think this would eliminate all these dynamic_casts
bool FEBioImport::UpdateFEModel(FEModel& fem)
{
	// set the parent ID's for rigid bodies
	int nmat = m_febio->Materials();
	for (int i = 0; i<nmat; ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (pm)
		{
			FERigidMaterial* pr = dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties());
			if (pr && (pr->m_pid != -1))
			{
				int pid = pr->m_pid - 1;
				if ((pid < 0) || (pid >= nmat)) return errf("Invalid material ID for rigid body");
				GMaterial* pp = fem.GetMaterial(pid);
				if ((pp == 0) || (dynamic_cast<FERigidMaterial*>(pp->GetMaterialProperties()) == 0)) return errf("Invalid material for rigid body parent");
				pr->m_pid = pp->GetID();
			}
		}
	}

	// resolve all load curve references
	int NLC = m_febio->LoadCurves();
	int NPC = m_febio->ParamCurves();
	for (int i=0; i<NPC; ++i)
	{
		FEBioInputModel::PARAM_CURVE pc = m_febio->GetParamCurve(i);
		assert(pc.m_lc >= 0);
		assert(pc.m_p || pc.m_plc);
		if ((pc.m_lc >= 0) && (pc.m_lc < m_febio->LoadCurves()))
		{
			if (pc.m_p)
			{
				pc.m_p->SetLoadCurve(m_febio->GetLoadCurve(pc.m_lc));
			}
			if (pc.m_plc) 
			{
				*pc.m_plc = m_febio->GetLoadCurve(pc.m_lc);
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
		FEPlotVariable* pv = plt.FindVariable(name);
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
			FEItemListBuilder* surf = mdl.FindNamedSelection(domain);
			if (surf)
			{
				pv->addDomain(surf);
			}
			else 
			{
				FEBioInputModel::Surface* surf = m_febio->FindSurface(domain.c_str());
				if (surf)
				{
					FESurface* ps = m_febio->BuildFESurface(domain.c_str());
					GObject* po = ps->GetGObject(); assert(po);
					if (po)
					{
						po->AddFESurface(ps);
						pv->addDomain(ps);
					}
				}
				else AddLogEntry("Could not find surface named %s", domain.c_str());
			}
		}
	}

	// make unused surfaces into named selections.
	// This can happen when surfaces are used in features that 
	// are not supported. The features will be skipped, but we may 
	// want to retain the surfaces.
	for (int i = 0; i < m_febio->Instances(); ++i)
	{
		// get the next instance
		FEBioInputModel::PartInstance& partInstance = *m_febio->GetInstance(i);
		FEBioInputModel::Part* part = partInstance.GetPart();
		GMeshObject* po = partInstance.GetGObject();
		for (int j = 0; j < part->Surfaces(); ++j)
		{
			FEBioInputModel::Surface& surf = part->GetSurface(j);
			if (surf.m_refs == 0)
			{
				FESurface* psurf = partInstance.BuildFESurface(surf.name().c_str());
				if (psurf)
				{
					psurf->SetName(surf.name());
					po->AddFESurface(psurf);
				}
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

		// pass ownership to the model
		fem.GetModel().AddObject(po);
	}

	// update log variables
	GModel& model = fem.GetModel();
	GMeshObject* po = 0;
	if (model.Objects() == 1) po = dynamic_cast<GMeshObject*>(model.Object(0));
	CLogDataSettings& log = m_prj.GetLogDataSettings();
	log.ClearLogData();
	for (int i = 0; i<m_febio->LogVariables(); ++i)
	{
		FEBioInputModel::LogVariable& v = m_febio->GetLogVariable(i);

		FELogData ld;
		ld.type = v.type();
		ld.sdata = v.data();
		ld.groupID = v.GroupID();
		ld.fileName = v.file();
		log.AddLogData(ld);
	}

	return true;
}
