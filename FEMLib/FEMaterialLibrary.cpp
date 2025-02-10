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
#include "FEMaterialLibrary.h"
#include <FSCore/Archive.h>

FEMaterialLibrary::FEMaterialLibrary()
{

}

FEMaterialLibrary::~FEMaterialLibrary()
{
	Clear();
}

void FEMaterialLibrary::Clear()
{
	int N = Materials();
	for (int i=0; i<N; ++i) delete m_mat[i].m_pmat;
	m_mat.clear();
}

bool FEMaterialLibrary::Load(IArchive& ar)
{
	TRACE("FEMaterialLibrary::Load");

	// read the version
	ar.OpenChunk();
	int nid = ar.GetChunkID();
	if (nid != CID_MATLIB_VERSION) return false;
	int nversion;
	ar.read(nversion);
	if (nversion != ML_VERSION) return false;
	ar.CloseChunk();

	// read the material section
	ar.OpenChunk();
	nid = ar.GetChunkID();
	if (nid == CID_MATERIAL_SECTION)
	{
		while (IArchive::IO_OK == ar.OpenChunk())
		{
			int ntype = ar.GetChunkID();

			// allocate the material
			FSMaterial* pmat = FEMaterialFactory::Create(nullptr, ntype);
			if (pmat == 0) throw ReadError("error parsing CID_MATERIAL_SECTION in FEMaterialLibrary::Load");

			char szname[256] = {0};

			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case CID_MAT_NAME: ar.read(szname); break;
				case CID_MAT_PARAMS:
					{
						// load material data
						pmat->Load(ar);
					}
					break;
				}
				ar.CloseChunk();
			}

			// add the material to the library
			Add(szname, pmat);

			ar.CloseChunk();
		}
	}
	else return false;
	ar.CloseChunk();
	
	return true;
}

bool FEMaterialLibrary::Save(OArchive& ar)
{
	// save the version number
	int nversion = ML_VERSION;
	ar.WriteChunk(CID_MATLIB_VERSION, nversion);

	// save all the materials
	int nmats = Materials();
	if (nmats > 0)
	{
		ar.BeginChunk(CID_MATERIAL_SECTION);
		{
			for (int i=0; i<nmats; ++i)
			{
				FSMaterial* pm = Material(i);
				int ntype = pm->Type();
				ar.BeginChunk(ntype);
				{
					ar.WriteChunk(CID_MAT_NAME, m_mat[i].m_szname);
					ar.BeginChunk(CID_MAT_PARAMS);
					{
						pm->Save(ar);
					}
					ar.EndChunk();
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
	return true;
}

void FEMaterialLibrary::Add(const char* szname, FSMaterial* pmat)
{
	int ntype = pmat->Type();
	FEMaterialFactory* pMF = FEMaterialFactory::GetInstance();
	FSMaterial* pm = pMF->Create(nullptr, ntype);
	pm->copy(pmat);
	MATENTRY m;
	m.m_pmat = pm;
	strcpy(m.m_szname, szname);
	m_mat.push_back(m);
}
