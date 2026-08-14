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
#pragma once
#include <FSCore/Archive.h>
#include <MeshLib/IHasItemList.h>
#include <MeshLib/FSItemListBuilder.h>

class FSModel;

// Output for log file
class FSLogData
{
public:
	enum { LD_NODE, LD_ELEM, LD_RIGID, LD_CNCTR, LD_FACE, LD_SURFACE, LD_DOMAIN };

public:
	FSLogData(int ntype) { m_type = ntype; }
	virtual ~FSLogData() {}

	int Type() const { return m_type; }

	void SetDataString(const std::string& data) { m_sdata = data; }
	std::string GetDataString() const { return m_sdata; }

	void SetFileName(const std::string& fileName) { m_fileName = fileName; }
	std::string GetFileName() const { return m_fileName; }

private:
	int			m_type;			// type of data
	std::string	m_sdata;		// data string
	std::string	m_fileName;		// file name (optional)
};

class FSLogNodeData : public FSLogData, public FSHasOneItemList
{
public:
	FSLogNodeData() : FSLogData(LD_NODE) { SetMeshItemType(FE_NODE_FLAG); }
	FSLogNodeData(FSItemListBuilder* pl) : FSLogData(LD_NODE) {
		SetMeshItemType(FE_NODE_FLAG);
		SetItemList(pl);
	}
};

class FSLogElemData : public FSLogData, public FSHasOneItemList
{
public:
	FSLogElemData() : FSLogData(LD_ELEM) { SetMeshItemType(FE_ELEM_FLAG); }
	FSLogElemData(FSItemListBuilder* pl) : FSLogData(LD_ELEM) {
		SetMeshItemType(FE_ELEM_FLAG);
		SetItemList(pl);
	}
};

class FSLogFaceData : public FSLogData, public FSHasOneItemList
{
public:
	FSLogFaceData() : FSLogData(LD_FACE) { SetMeshItemType(FE_FACE_FLAG); }
	FSLogFaceData(FSItemListBuilder* pl) : FSLogData(LD_FACE) {
		SetMeshItemType(FE_FACE_FLAG);
		SetItemList(pl);
	}
};

class FSLogSurfaceData : public FSLogData, public FSHasOneItemList
{
public:
	FSLogSurfaceData() : FSLogData(LD_SURFACE) { SetMeshItemType(FE_FACE_FLAG); }
	FSLogSurfaceData(FSItemListBuilder* pl) : FSLogData(LD_SURFACE) {
		SetMeshItemType(FE_FACE_FLAG);
		SetItemList(pl);
	}
};

class FSLogDomainData : public FSLogData, public FSHasOneItemList
{
public:
	FSLogDomainData() : FSLogData(LD_DOMAIN) { SetMeshItemType(FE_PART_FLAG); }
	FSLogDomainData(FSItemListBuilder* pl) : FSLogData(LD_DOMAIN) {
		SetMeshItemType(FE_PART_FLAG);
		SetItemList(pl);
	}
};

class FSLogRigidData : public FSLogData
{
public:
	FSLogRigidData() : FSLogData(LD_RIGID) { m_matID = -1; }
	FSLogRigidData(int matID) : FSLogData(LD_RIGID) { m_matID = matID; }

	void SetMatID(int mid) { m_matID = mid; }
	int GetMatID() const { return m_matID; }

private:
	int	m_matID;
};

class FSLogConnectorData : public FSLogData
{
public:
	FSLogConnectorData() : FSLogData(LD_CNCTR) { m_rcID = -1; }
	FSLogConnectorData(int rcid) : FSLogData(LD_CNCTR) { m_rcID = rcid; }

	void SetConnectorID(int rcid) { m_rcID = rcid; }
	int GetConnectorID() const { return m_rcID; }

private:
	int	m_rcID;
};

// class that manages the log file settings
class CLogDataSettings
{
public:
	CLogDataSettings(FSModel& fem);
	~CLogDataSettings();

	//! save to file
	void Save(OArchive& ar);

	//! load from file
	void Load(IArchive& ar);

public:
	int LogDataSize() { return (int)m_log.size(); }
	FSLogData& LogData(int i);
	void AddLogData(FSLogData* d);
	void ClearLogData();
	void RemoveLogData(int item);

private:
	FSModel& m_fem;
	std::vector<FSLogData*>		m_log;		// log data
};
