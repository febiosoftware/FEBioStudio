#include "stdafx.h"
#include "FSObject.h"

FSObject::FSObject(void)
{

}

FSObject::~FSObject(void)
{
}

void FSObject::SetName(const std::string& name)
{
	m_name = name;
}

const std::string& FSObject::GetName() const
{
	return m_name;
}

void FSObject::Save(OArchive& ar)
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

void FSObject::Load(IArchive& ar)
{
	TRACE("FSObject::Load");
	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { char sz[256] = { 0 }; ar.read(sz); SetName(sz); } break;
		case CID_FEOBJ_PARAMS: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

// update the object's data
bool FSObject::Update(bool b)
{
	return true;
}

// update parameters
void FSObject::UpdateData(bool bsave)
{

}
