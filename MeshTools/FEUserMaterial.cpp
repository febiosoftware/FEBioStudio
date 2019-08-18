#include "FEUserMaterial.h"
#include <cstdlib>

void FEUserMaterial::AddParameter(const char* szname, double v)
{
	// the parameter does not copy the name so we have to make
	// sure we make a copy first
	char* sz = strdup(szname);
	m_pname.push_back(sz);
	AddDoubleParam(v, sz, sz);
}

FEUserMaterial::~FEUserMaterial()
{
	for (int i=0; i<(int) m_pname.size(); ++i) free(m_pname[i]);
	m_pname.clear();
}

void FEUserMaterial::SetTypeStr(const char* sz)
{
	strcpy(m_sztype, sz);
}

//-----------------------------------------------------------------------------
void FEUserMaterial::copy(FEMaterial* pmat)
{
	FEUserMaterial* pm = dynamic_cast<FEUserMaterial*>(pmat);
	assert(pm);
	if (pm) SetTypeStr(pm->GetTypeStr());
	FEMaterial::copy(pmat);
}

//-----------------------------------------------------------------------------
void FEUserMaterial::Save(OArchive &ar)
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
void FEUserMaterial::Load(IArchive &ar)
{
	TRACE("FEUserMaterial::Load");

	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case SZTYPE: ar.read(m_sztype); break;
		case PARAMDATA:
			{
				while (ar.OpenChunk() == IO_OK)
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
