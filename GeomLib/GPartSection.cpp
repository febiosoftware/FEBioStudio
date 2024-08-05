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

#include "GPartSection.h"
#include <FEMLib/FEElementFormulation.h>
#include <FEBioLink/FEBioClass.h>
#include <FECore/fecore_enum.h>
#include "GItem.h"

//=============================================================================
GPartSection::GPartSection(GPart* part) : m_part(part)
{
	if (part) SetParent(part);
}

const GPart* GPartSection::GetPart() const { return m_part; }
GPart* GPartSection::GetPart() { return m_part; }

GSolidSection::GSolidSection(GPart* pg) : GPartSection(pg)
{
	AddChoiceParam(0, "type", "Solid formulation")->SetEnumNames("$(solid_domain)");
	m_form = nullptr;
}

GSolidSection::~GSolidSection()
{
	delete m_form;
}

void GSolidSection::Save(OArchive& ar)
{
	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	if (m_form)
	{
		ar.BeginChunk(CID_OBJ_SOLID_DOMAIN);
		{
			m_form->Save(ar);
		}
		ar.EndChunk();
	}
}

void GSolidSection::Load(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_OBJ_PARAMS: ParamContainer::Load(ar); break;
		case CID_OBJ_SOLID_DOMAIN:
		{
			FESolidFormulation* solid = new FESolidFormulation(nullptr);
			solid->Load(ar);
			SetElementFormulation(solid);
		}
		break;
		}
		ar.CloseChunk();
	}
}

bool GSolidSection::UpdateData(bool bsave)
{
	if (bsave)
	{
		int n = GetIntValue(0);
		if (n <= 0) { delete m_form; m_form = nullptr; return true; }

		// we subtract by one, since n==0 is the null formulation (i.e. "default"). 
		n--;
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESOLIDDOMAIN_ID);
		assert((n >= 0) && (n < l.size()));

		if ((m_form == nullptr) || (m_form->GetClassID() != l[n].classId))
		{
			delete m_form;
			m_form = FEBio::CreateSolidFormulation(l[n].sztype, nullptr);
			assert(m_form);
			return true;
		}
	}
	return false;
}

GSolidSection* GSolidSection::Copy()
{
	GSolidSection* s = new GSolidSection(nullptr);
	s->CopyParams(*this);
	return s;
}

void GSolidSection::SetElementFormulation(FESolidFormulation* form)
{
	delete m_form;
	m_form = form;
	if (form == nullptr) SetIntValue(0, 0);
	else
	{
		int n = form->GetClassID(); assert(n > 0);
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESOLIDDOMAIN_ID);
		for (int i = 0; i < l.size(); ++i)
		{
			if (l[i].classId == n)
			{
				// Note we add 1, since we need to offset for the "default" formulation.
				SetIntValue(0, i + 1);
				return;
			}
		}
	}
}

FESolidFormulation* GSolidSection::GetElementFormulation()
{
	return m_form;
}

//========================================================================
GShellSection::GShellSection(GPart* pg) : GPartSection(pg)
{
	AddChoiceParam(0, "type", "Shell formulation")->SetEnumNames("$(shell_domain)");
	AddDoubleParam(0.0, "shell_thickness", "shell thickness");
	m_form = nullptr;
}

GShellSection::~GShellSection()
{
	delete m_form;
}

void GShellSection::Save(OArchive& ar)
{
	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	if (m_form)
	{
		ar.BeginChunk(CID_OBJ_SHELL_DOMAIN);
		{
			m_form->Save(ar);
		}
		ar.EndChunk();
	}
}

void GShellSection::Load(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_OBJ_PARAMS: ParamContainer::Load(ar); break;
		case CID_OBJ_SHELL_DOMAIN:
		{
			FEShellFormulation* shell = new FEShellFormulation(nullptr);
			shell->Load(ar);
			SetElementFormulation(shell);
		}
		break;
		}
		ar.CloseChunk();
	}
}

void GShellSection::SetElementFormulation(FEShellFormulation* form)
{
	delete m_form;
	m_form = form;
	if (form == nullptr)
	{
		SetIntValue(0, 0);
		GetParam(1).SetVisible(true);
		GetParam(1).SetEditable(true);
	}
	else
	{
		GetParam(1).SetVisible(false);
		GetParam(1).SetEditable(false);
		Param* p = m_form->GetParam("shell_thickness");
		if (p) SetFloatValue(1, shellThickness());

		int n = form->GetClassID(); assert(n > 0);
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESHELLDOMAIN_ID);
		for (int i = 0; i < l.size(); ++i)
		{
			if (l[i].classId == n)
			{
				// Note we add 1, since we need to offset for the "default" formulation.
				SetIntValue(0, i + 1);
				return;
			}
		}
	}
}

FEShellFormulation* GShellSection::GetElementFormulation() { return m_form; }

void GShellSection::SetShellThickness(double h)
{
	SetFloatValue(1, h);
	if (m_form)
	{
		Param* p = m_form->GetParam("shell_thickness");
		if (p) p->SetFloatValue(h);
	}
}

double GShellSection::shellThickness() const
{
	return GetFloatValue(1);
}

bool GShellSection::UpdateData(bool bsave)
{
	if (bsave)
	{
		int n = GetIntValue(0);
		if (n <= 0) { 
			// we need to make sure that shell thickness stays in sync
			if (m_form)
			{
				Param* p = m_form->GetParam("shell_thickness");
				if (p) SetFloatValue(1, p->GetFloatValue());
			}

			SetElementFormulation(nullptr);
			return true; 
		}

		// we subtract by one, since n==0 is the null formulation (i.e. "default"). 
		n--;
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESHELLDOMAIN_ID);
		assert((n >= 0) && (n < l.size()));

		if ((m_form == nullptr) || (m_form->GetClassID() != l[n].classId))
		{
			FEShellFormulation* form = FEBio::CreateShellFormulation(l[n].sztype, nullptr);
			SetElementFormulation(form);
			return true;
		}
	}
	return false;
}

GShellSection* GShellSection::Copy()
{
	GShellSection* s = new GShellSection(nullptr);
	s->CopyParams(*this);
	return s;
}

//========================================================================
GBeamSection::GBeamSection(GPart* pg) : GPartSection(pg)
{
	AddChoiceParam(0, "type", "Beam formulation")->SetEnumNames("$(beam_domain)");
	m_form = nullptr;
}

GBeamSection::~GBeamSection()
{
	delete m_form;
}

void GBeamSection::Save(OArchive& ar)
{
	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	if (m_form)
	{
		ar.BeginChunk(CID_OBJ_BEAM_DOMAIN);
		{
			m_form->Save(ar);
		}
		ar.EndChunk();
	}
}

void GBeamSection::Load(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_OBJ_PARAMS: ParamContainer::Load(ar); break;
		case CID_OBJ_BEAM_DOMAIN:
		{
			FEBeamFormulation* beam = new FEBeamFormulation(nullptr);
			m_form = beam;// SetElementFormulation(beam);
			beam->Load(ar);
		}
		break;
		}
		ar.CloseChunk();
	}
}

void GBeamSection::SetElementFormulation(FEBeamFormulation* form)
{
	delete m_form;
	m_form = form;
	if (form == nullptr) SetIntValue(0, 0);
	else
	{
		int n = form->GetClassID(); assert(n > 0);
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FEBEAMDOMAIN_ID);
		for (int i = 0; i < l.size(); ++i)
		{
			if (l[i].classId == n)
			{
				// Note we add 1, since we need to offset for the "default" formulation.
				SetIntValue(0, i + 1);
				return;
			}
		}
	}
}

FEBeamFormulation* GBeamSection::GetElementFormulation() { return m_form; }

bool GBeamSection::UpdateData(bool bsave)
{
	if (bsave)
	{
		int n = GetIntValue(0);
		if (n <= 0) { delete m_form; m_form = nullptr; return true; }

		// we subtract by one, since n==0 is the null formulation (i.e. "default"). 
		n--;
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FEBEAMDOMAIN_ID);
		assert((n >= 0) && (n < l.size()));

		if ((m_form == nullptr) || (m_form->GetClassID() != l[n].classId))
		{
			delete m_form;
			m_form = FEBio::CreateBeamFormulation(l[n].sztype, nullptr);
			assert(m_form);
			return true;
		}
	}
	return false;
}

GBeamSection* GBeamSection::Copy()
{
	GBeamSection* s = new GBeamSection(nullptr);
	s->CopyParams(*this);
	return s;
}
