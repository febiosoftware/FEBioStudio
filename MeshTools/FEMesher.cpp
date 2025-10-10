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
#include "FEMesher.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GMeshObject.h>
#include "FETetGenMesher.h"
#include <stack>
//using namespace std;

//-----------------------------------------------------------------------------
// constructor
FEMesher::FEMesher(GObject& o) : m_o(o)
{
	m_ntype = 0;
}

//-----------------------------------------------------------------------------
// destructor
FEMesher::~FEMesher()
{

}

//-----------------------------------------------------------------------------
int FEMesher::Type() const
{
	return m_ntype;
}

//-----------------------------------------------------------------------------
void FEMesher::SetType(int ntype)
{
	m_ntype = ntype;
}

//-----------------------------------------------------------------------------
void FEMesher::Save(OArchive &ar)
{
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

//-----------------------------------------------------------------------------

void FEMesher::Load(IArchive& ar)
{
	TRACE("FEMesher::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == PARAMS) ParamContainer::Load(ar);
		ar.CloseChunk();
	}
}

FEMesher* FEMesher::Create(GObject* po, int classType)
{
	assert(po);
	if (po == nullptr) return nullptr;

	FEMesher* mesher = nullptr;
	switch (classType)
	{
	case 0: mesher = po->CreateDefaultMesher(); break;
	case 1: mesher = new FETetGenMesher(*po); break;
	default:
		assert(false);
	}
	assert(mesher);

	return mesher;
}

void FEMesher::SetErrorMessage(const std::string& err)
{
	m_error = err;
}

std::string FEMesher::GetErrorMessage() const
{
	return m_error;
}
