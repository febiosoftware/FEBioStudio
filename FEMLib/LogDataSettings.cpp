/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "LogDataSettings.h"
#include "FSModel.h"
#include <GeomLib/GModel.h>

CLogDataSettings::CLogDataSettings(FSModel& fem) : m_fem(fem)
{}

CLogDataSettings::~CLogDataSettings()
{
	ClearLogData();
}

void CLogDataSettings::ClearLogData()
{
	for (auto p : m_log) delete p;
	m_log.clear();
}

FSLogData& CLogDataSettings::LogData(int i) { return *m_log[i]; }
void CLogDataSettings::AddLogData(FSLogData* d) { m_log.push_back(d); }

void CLogDataSettings::RemoveLogData(int item)
{
	delete m_log[item];
	m_log.erase(m_log.begin() + item);
}

void CLogDataSettings::Save(OArchive& ar)
{
	const int N = (int)m_log.size();
	for (int i = 0; i < N; ++i)
	{
		FSLogData& v = *m_log[i];
		ar.BeginChunk(CID_PRJ_LOGDATA_ITEM);
		{
			ar.WriteChunk(CID_PRJ_LOGDATA_TYPE, v.Type());
			ar.WriteChunk(CID_PRJ_LOGDATA_DATA, v.GetDataString());
			ar.WriteChunk(CID_PRJ_LOGDATA_FILE, v.GetFileName());

			switch (v.Type())
			{
			case FSLogData::LD_NODE:
			case FSLogData::LD_ELEM:
			case FSLogData::LD_FACE:
			case FSLogData::LD_SURFACE:
			case FSLogData::LD_DOMAIN:
			{
				FSHasOneItemList* pil = dynamic_cast<FSHasOneItemList*>(&v); assert(pil);
				if (pil)
				{
					FSItemListBuilder* pl = pil->GetItemList();
					if (pl) ar.WriteChunk(CID_PRJ_LOGDATA_GID, pl->GetID());
				}
			}
			break;
			case FSLogData::LD_RIGID:
			{
				FSLogRigidData* prd = dynamic_cast<FSLogRigidData*>(&v); assert(prd);
				if (prd) ar.WriteChunk(CID_PRJ_LOGDATA_MID, prd->GetMatID());
			}
			break;
			case FSLogData::LD_CNCTR:
			{
				FSLogConnectorData* prc = dynamic_cast<FSLogConnectorData*>(&v); assert(prc);
				if (prc) ar.WriteChunk(CID_PRJ_LOGDATA_CID, prc->GetConnectorID());
			}
			break;
			default:
				assert(false);
			}
		}
		ar.EndChunk();
	}
}

void CLogDataSettings::Load(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		if (ar.GetChunkID() == CID_PRJ_LOGDATA_ITEM)
		{
			std::string data, file;
			int ntype = -1;
			int mid = -1, gid = -1, cid = -1;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_PRJ_LOGDATA_TYPE: ar.read(ntype); break;
				case CID_PRJ_LOGDATA_DATA: ar.read(data); break;
				case CID_PRJ_LOGDATA_MID: ar.read(mid); break;
				case CID_PRJ_LOGDATA_GID: ar.read(gid); break;
				case CID_PRJ_LOGDATA_CID: ar.read(cid); break;
				case CID_PRJ_LOGDATA_FILE: ar.read(file); break;
				}
				ar.CloseChunk();
			}

			GModel& gm = m_fem.GetModel();

			FSLogData* ld = nullptr;
			switch (ntype)
			{
			case FSLogData::LD_NODE: ld = new FSLogNodeData(gm.FindNamedSelection(gid)); break;
			case FSLogData::LD_FACE: ld = new FSLogFaceData(gm.FindNamedSelection(gid)); break;
			case FSLogData::LD_SURFACE: ld = new FSLogSurfaceData(gm.FindNamedSelection(gid)); break;
			case FSLogData::LD_ELEM: ld = new FSLogElemData(gm.FindNamedSelection(gid)); break;
			case FSLogData::LD_DOMAIN: ld = new FSLogDomainData(gm.FindNamedSelection(gid)); break;
			case FSLogData::LD_RIGID: ld = new FSLogRigidData(mid); break;
			case FSLogData::LD_CNCTR: ld = new FSLogConnectorData(cid); break;
			default:
				assert(false);
			}

			if (ld)
			{
				ld->SetDataString(data);
				ld->SetFileName(file);
				AddLogData(ld);
			}
		}
		ar.CloseChunk();
	}
}
