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
#include "GMaterial.h"
#include "FEMaterial.h"
#include <sstream>
#include <FSCore/Palette.h>

using std::stringstream;

int GMaterial::m_nref = 1;

GMaterial::GMaterial(FSMaterial* pm)
{
	m_pm = pm;
	if (m_pm) m_pm->SetOwner(this);
	m_ntag = 0;

	// set unique material ID
	m_nID = m_nref++;

	// give a default color
	m_glmat.type = GLMaterial::PLASTIC;
	m_glmat.diffuse = GLColor(200,200,200);
	m_glmat.ambient = GLColor(0,0,32);
	m_glmat.specular = GLColor(0,0,0);
	m_glmat.emission = GLColor(0,0,0);
	m_glmat.shininess = 1.f;

	// set default rendering style
	m_nrender = 0; // solid rendering

	// set a default name for this material
	stringstream ss;
	ss << "Material" << m_nID;
	SetName(ss.str());

	CPaletteManager& PM = CPaletteManager::GetInstance();
	const CPalette& pal = PM.CurrentPalette();
	int NCOL = pal.Colors();

	m_glmat.diffuse = pal.Color((m_nID-1) % NCOL);
}

GMaterial::~GMaterial(void)
{
	delete m_pm;
}

void GMaterial::SetMaterialProperties(FSMaterial* pm) 
{ 
	if (pm != m_pm)
	{
		delete m_pm;
		m_pm = pm;
		m_pm->SetOwner(this);
	}
}

FSMaterial* GMaterial::GetMaterialProperties()
{ 
	return m_pm; 
}

FSMaterial* GMaterial::TakeMaterialProperties()
{
	FSMaterial* pm = m_pm;
	m_pm = nullptr;
	if (pm) pm->SetOwner(nullptr);
	return pm;
}

GMaterial* GMaterial::Clone()
{
	FSMaterial* pmCopy = 0;
	if (m_pm) pmCopy = m_pm->Clone();
	return new GMaterial(pmCopy);
}

const char* GMaterial::GetFullName() const
{
	static char sz[256];
	sprintf(sz, "%s (%s)", GetName().c_str(), m_pm->GetTypeString());
	return sz;
}

const char* GMaterial::GetTypeString() const
{
	return (m_pm ? m_pm->GetTypeString() : nullptr);
}

void GMaterial::Save(OArchive &ar)
{
	ar.WriteChunk(CID_MAT_ID, m_nID);
	ar.WriteChunk(CID_MAT_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_MAT_DIFFUSE, m_glmat.diffuse);
	ar.WriteChunk(CID_MAT_AMBIENT, m_glmat.ambient);
	ar.WriteChunk(CID_MAT_SPECULAR, m_glmat.specular);
	ar.WriteChunk(CID_MAT_EMISSION, m_glmat.emission);
	ar.WriteChunk(CID_MAT_SHININESS, m_glmat.shininess);
	ar.WriteChunk(CID_MAT_RENDER, m_nrender);

	if (m_pm)
	{
		ar.BeginChunk(CID_MAT_PARAMS);
		{
			m_pm->Save(ar);
		}
		ar.EndChunk();
	}
}

void GMaterial::Load(IArchive &ar)
{
	TRACE("GMaterial::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MAT_ID: { int mid; ar.read(mid); SetID(mid); } break;
		case CID_MAT_NAME: { std::string name; ar.read(name); SetName(name); break; }
		case CID_FEOBJ_INFO: { std::string info; ar.read(info); SetInfo(info); } break;
		case CID_MAT_DIFFUSE: ar.read(m_glmat.diffuse); break;
		case CID_MAT_AMBIENT: ar.read(m_glmat.ambient); break;
		case CID_MAT_SPECULAR: ar.read(m_glmat.specular); break;
		case CID_MAT_EMISSION: ar.read(m_glmat.emission); break;
		case CID_MAT_SHININESS: ar.read(m_glmat.shininess); break;
		case CID_MAT_RENDER: ar.read(m_nrender); break;
		case CID_MAT_PARAMS: assert(m_pm); m_pm->Load(ar); break;
		}
		ar.CloseChunk();
	}
}
