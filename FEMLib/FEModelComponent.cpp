/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "FEModelComponent.h"
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include <exception>
#include <sstream>

FSModelComponent::FSModelComponent()
{
	m_superClassID = -1;
	m_sztype = "(unknown)";
}

int FSModelComponent::GetSuperClassID() const
{ 
	return m_superClassID; 
}

void FSModelComponent::SetSuperClassID(int superClassID)
{
	m_superClassID = superClassID;
}

void FSModelComponent::SetTypeString(const char* sztype)
{ 
	m_sztype = sztype; 
}

const char* FSModelComponent::GetTypeString() 
{ 
	return m_sztype; 
}

//==============================================================================
void SaveClassMetaData(FSModelComponent* pc, OArchive& ar)
{
	string typeStr(pc->GetTypeString());
	ar.WriteChunk(CID_FEBIO_TYPE_STRING, typeStr);
}

void LoadClassMetaData(FSModelComponent* pc, IArchive& ar)
{
	TRACE("LoadClassMetaData");
	int superClassId = pc->GetSuperClassID(); assert(superClassId > 0);
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_TYPE_STRING:
		{
			string typeStr;
			ar.read(typeStr);
			assert(superClassId != -1);
			if (FEBio::CreateModelComponent(superClassId, typeStr, pc) == false)
			{
				const char* szsuperclass = FEBio::GetSuperClassString(superClassId);
				if (szsuperclass == nullptr) szsuperclass = "(unknown)";
				std::stringstream ss;
				ss << "Failed to allocate model component:\n";
				ss << "super class : " << szsuperclass << std::endl;
				ss << "type string : \"" << typeStr << "\"";
				ss << "\n\nIt is possible that this file requires an FEBio plugin before it can be loaded.";
				string s = ss.str();
                throw std::exception();
			}
		}
		break;
		}
		ar.CloseChunk();
	}
}
