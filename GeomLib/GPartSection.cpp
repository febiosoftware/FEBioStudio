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
	AddChoiceParam(0, "elem_type", "Element Formulation")->SetEnumNames("$(solid_domain)");
	m_form = nullptr;
}

GSolidSection::~GSolidSection()
{
	delete m_form;
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

GShellSection::GShellSection(GPart* pg) : GPartSection(pg)
{
	AddChoiceParam(0, "elem_type", "Shell Formulation")->SetEnumNames("$(shell_domain)");
	AddDoubleParam(0.0, "shell_thickness", "shell thickness");
	m_form = nullptr;
}

GShellSection::~GShellSection()
{
	delete m_form;
}

void GShellSection::SetElementFormulation(FEShellFormulation* form)
{
	delete m_form;
	m_form = form;
	if (form == nullptr) SetIntValue(0, 0);
	else
	{
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
		if (n <= 0) { delete m_form; m_form = nullptr; return true; }

		// we subtract by one, since n==0 is the null formulation (i.e. "default"). 
		n--;
		std::vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESHELLDOMAIN_ID);
		assert((n >= 0) && (n < l.size()));

		if ((m_form == nullptr) || (m_form->GetClassID() != l[n].classId))
		{
			delete m_form;
			m_form = FEBio::CreateShellFormulation(l[n].sztype, nullptr);
			assert(m_form);
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
