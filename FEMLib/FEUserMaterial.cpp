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

#include "FEUserMaterial.h"
#include <cstdlib>

void FSUserMaterial::AddParameter(const char* szname, double v)
{
	// the parameter does not copy the name so we have to make
	// sure we make a copy first
	char* sz = strdup(szname);
	m_pname.push_back(sz);
	AddDoubleParam(v, sz, sz);
}

FSUserMaterial::FSUserMaterial(int ntype, FSModel* fem) : FSMaterial(ntype, fem) 
{
	m_sztype[0] = 0;
}

FSUserMaterial::~FSUserMaterial()
{
	for (int i=0; i<(int) m_pname.size(); ++i) free(m_pname[i]);
	m_pname.clear();
}

void FSUserMaterial::SetTypeString(const std::string& sz)
{
	strcpy(m_sztype, sz.c_str());
}

//-----------------------------------------------------------------------------
void FSUserMaterial::copy(FSMaterial* pmat)
{
	FSUserMaterial* pm = dynamic_cast<FSUserMaterial*>(pmat);
	assert(pm);
	if (pm) SetTypeString(pm->GetTypeString());
	FSMaterial::copy(pmat);
}

//-----------------------------------------------------------------------------
void FSUserMaterial::Save(OArchive &ar)
{
	ParamBlock& PB = GetParamBlock();
	int NP = PB.Size();

	ar.WriteChunk(SZTYPE, m_sztype);
	if (NP > 0)
	{
		for (int i=0; i<NP; ++i)
		{
			ar.BeginChunk(PARAMDATA);
			{
				Param& p = PB[i];
				int ntype = p.GetParamType();
				ar.WriteChunk(PARAMNAME, (char*) p.GetLongName());
				ar.WriteChunk(PARAMTYPE, ntype);
				ar.BeginChunk(PARAMVALUE);
				{
					SaveParam(p, ar);
				}
				ar.EndChunk();
			}
			ar.EndChunk();
		}
	}
}

//-----------------------------------------------------------------------------
void FSUserMaterial::Load(IArchive &ar)
{
	TRACE("FSUserMaterial::Load");

	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case SZTYPE: ar.read(m_sztype); break;
		case PARAMDATA:
			{
				while (ar.OpenChunk() == IArchive::IO_OK)
				{
					int nid = ar.GetChunkID();
					char szname[256];
					switch (nid)
					{
					case PARAMNAME: ar.read(szname); break;
					case PARAMTYPE:
						{
							int ntype;
							ar.read(ntype);
							switch (ntype)
							{
							case Param_FLOAT: AddParameter(szname ,0); break;
							default:
								assert(false);
							}
						};
						break;
					case PARAMVALUE:
						{
							LoadParam(ar);
						}
						break;
					}
					ar.CloseChunk();
				}
			}
			break;
		}
		ar.CloseChunk();
	}
}
