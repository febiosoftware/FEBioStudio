#include "FEObject.h"

FEObject::FEObject(void)
{
	
}

FEObject::~FEObject(void)
{
}

void FEObject::SetName(const std::string& name)
{
	m_name = name;
}

const std::string& FEObject::GetName() const 
{
	return m_name; 
}

void FEObject::Save(OArchive& ar)
{
	// save the name if there is one
	if (m_name.empty() == false)
	{
		const char* szname = m_name.c_str();
		ar.WriteChunk(CID_FEOBJ_NAME, szname);
	}
	ar.BeginChunk(CID_FEOBJ_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

}

void FEObject::Load(IArchive& ar)
{
	TRACE("FEObject::Load");

	if (ar.Version() < 0x0001000C) ParamContainer::Load(ar);
	else
	{
		while (IO_OK == ar.OpenChunk())
		{
			int nid = ar.GetChunkID();
			switch (nid)
			{
			case CID_FEOBJ_NAME: { char sz[256] = {0}; ar.read(sz); SetName(sz); } break;
			case CID_FEOBJ_PARAMS: ParamContainer::Load(ar); break;
			}
			ar.CloseChunk();
		}
	}
}
