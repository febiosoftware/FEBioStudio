#include "stdafx.h"
#include "FSObject.h"

FSObject::FSObject(FSObject* parent) : m_parent(parent)
{

}

FSObject::~FSObject(void)
{
	if (m_parent) m_parent->RemoveChild(this);
}

void FSObject::SetName(const std::string& name)
{
	m_name = name;
}

const std::string& FSObject::GetName() const
{
	return m_name;
}

void FSObject::SetInfo(const std::string& info)
{
	m_info = info;
}

const std::string& FSObject::GetInfo() const
{
	return m_info;
}

FSObject* FSObject::GetParent()
{
	return m_parent;
}

const FSObject* FSObject::GetParent() const
{
	return m_parent;
}

void FSObject::SetParent(FSObject* parent)
{
	m_parent = parent;
}

size_t FSObject::RemoveChild(FSObject* po)
{
	assert(false);
	return -1;
}

void FSObject::InsertChild(size_t pos, FSObject* po)
{
	assert(false);
}

void FSObject::Save(OArchive& ar)
{
	// save the name if there is one
	if (m_name.empty() == false)
	{
		ar.WriteChunk(CID_FEOBJ_NAME, m_name);
	}
	if (m_info.empty() == false)
	{
		ar.WriteChunk(CID_FEOBJ_INFO, m_info);
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
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: ar.read(m_name); break;
		case CID_FEOBJ_INFO: ar.read(m_info); break;
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
bool FSObject::UpdateData(bool bsave)
{
	return false;
}
