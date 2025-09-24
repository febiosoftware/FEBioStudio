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
#include "FSObject.h"

//=============================================================================
FSBase::FSBase() {}

FSBase::~FSBase() {}

// update parameters
bool FSBase::UpdateData(bool bsave)
{
	return false;
}

//=============================================================================
FSObject::FSObject(FSObject* parent) : m_parent(parent), m_typeStr("")
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

std::string FSObject::GetNameAndType() const
{
	std::string s = GetName();
	s += " [";
	s += (GetTypeString() ? GetTypeString() : "<unknown>");
	s += "]";
	return s;
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

bool FSObject::IsType(const char* sztype) const
{
	const char* sz = GetTypeString();
	return (strcmp(sz, sztype) == 0);
}
